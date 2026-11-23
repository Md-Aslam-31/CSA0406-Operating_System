#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence
if ! command -v qemu-system-x86_64 >/dev/null || ! command -v cloud-localds >/dev/null; then
  echo 'VM result: SKIPPED; QEMU or cloud-localds is not installed.' | tee evidence/vm-benchmark.txt
  exit 0
fi
work=$(mktemp -d)
pidfile="$work/vm.pid"
cleanup() {
  if [[ -f "$pidfile" ]]; then kill "$(cat "$pidfile")" 2>/dev/null || true; fi
  rm -rf "$work"
}
trap cleanup EXIT
mkdir -p "$work/wheels"
python3 -m pip download --only-binary=:all: --dest "$work/wheels" Flask==3.1.2 gunicorn==23.0.0
tar -czf "$work/wheels.tar.gz" -C "$work/wheels" .
app_b64=$(base64 -w0 webapp/app.py)
wheels_b64=$(base64 -w0 "$work/wheels.tar.gz")
cat > "$work/server.py" <<'PY'
import sys
sys.path.insert(0, "/opt/site-packages")
sys.argv = ["gunicorn", "--workers", "2", "--bind", "0.0.0.0:8000", "app:app"]
from gunicorn.app.wsgiapp import run
run()
PY
server_b64=$(base64 -w0 "$work/server.py")
cat > "$work/install-wheels.py" <<'PY'
import os
import tarfile

os.makedirs("/opt/site-packages", exist_ok=True)
with tarfile.open("/opt/wheels.tar.gz", "r:gz") as archive:
    archive.extractall("/opt/site-packages", filter="data")
PY
installer_b64=$(base64 -w0 "$work/install-wheels.py")
image="$work/noble-cloud.img"
overlay="$work/portal-vm.qcow2"
seed="$work/seed.iso"
curl -fsSL --retry 3 https://cloud-images.ubuntu.com/noble/current/noble-server-cloudimg-amd64.img -o "$image"
qemu-img create -q -f qcow2 -F qcow2 -b "$image" "$overlay" 12G
cat > "$work/user-data" <<EOF
#cloud-config
users:
  - name: portalvm
    system: true
    shell: /usr/sbin/nologin
write_files:
  - path: /opt/app.py
    encoding: b64
    permissions: '0644'
    content: $app_b64
  - path: /opt/server.py
    encoding: b64
    permissions: '0755'
    content: $server_b64
  - path: /opt/install-wheels.py
    encoding: b64
    permissions: '0644'
    content: $installer_b64
  - path: /opt/wheels.tar.gz
    encoding: b64
    permissions: '0644'
    content: $wheels_b64
  - path: /etc/systemd/system/vm-portal.service
    permissions: '0644'
    content: |
      [Unit]
      Description=Result portal VM benchmark
      After=network.target
      [Service]
      Type=simple
      User=portalvm
      WorkingDirectory=/opt
      Environment="PYTHONPATH=/opt/site-packages"
      Environment="APP_NAME=University Result Portal"
      ExecStart=/usr/bin/python3 /opt/server.py
      Restart=on-failure
      [Install]
      WantedBy=multi-user.target
runcmd:
  - [python3, /opt/install-wheels.py]
  - [systemctl, enable, --now, vm-portal.service]
EOF
cat > "$work/meta-data" <<'EOF'
instance-id: assignment5-vm-01
local-hostname: result-portal-vm
EOF
cloud-localds "$seed" "$work/user-data" "$work/meta-data"
if [[ -e /dev/kvm && -r /dev/kvm && -w /dev/kvm ]]; then
  accel=kvm
  accel_args=(-accel kvm)
else
  accel=tcg
  accel_args=(-accel tcg,thread=multi)
fi
start=$(date +%s%3N)
qemu-system-x86_64 "${accel_args[@]}" -cpu max -m 1536 -smp 2 \
  -display none -serial "file:$PWD/evidence/vm-console.log" -daemonize -pidfile "$pidfile" \
  -drive "file=$overlay,if=virtio,format=qcow2" \
  -drive "file=$seed,if=virtio,media=cdrom,readonly=on" \
  -netdev user,id=net0,hostfwd=tcp:127.0.0.1:18080-:8000 -device virtio-net-pci,netdev=net0
ready=0
for _ in $(seq 1 180); do
  if curl -fsS http://127.0.0.1:18080/health >/dev/null; then ready=1; break; fi
  sleep 2
done
end=$(date +%s%3N)
if [[ $ready -ne 1 ]]; then
  echo "VM result: UNAVAILABLE; QEMU mode=$accel; guest did not become healthy within 360s." | tee evidence/vm-benchmark.txt
  grep -E 'Started vm-portal|Failed to start|cloud-init.*finished|login:' evidence/vm-console.log | tail -n 12 >> evidence/vm-benchmark.txt || true
  exit 0
fi
startup=$(awk -v s="$start" -v e="$end" 'BEGIN {printf "%.3f",(e-s)/1000}')
printf 'mode=%s\nstartup_seconds=%s\nassigned_ram_mib=1536\n' "$accel" "$startup" | tee evidence/vm-benchmark.txt
ab -n 2000 -c 25 -k http://127.0.0.1:18080/health > evidence/benchmark-vm.txt
qemu_pid=$(cat "$pidfile")
vm_rss_kib=$(ps -o rss= -p "$qemu_pid" | tr -d ' ')
vm_rss_mib=$(awk -v x="${vm_rss_kib:-0}" 'BEGIN {printf "%.1f",x/1024}')
vm_size_mib=$(awk -v a="$(stat -c%s "$image")" -v b="$(stat -c%s "$overlay")" 'BEGIN {printf "%.1f",(a+b)/1048576}')
python3 - evidence/benchmark-vm.txt "$startup" "$vm_rss_mib" "$vm_size_mib" "$accel" >> evidence/benchmark.csv <<'PY'
import re,sys
text=open(sys.argv[1]).read()
rps=re.search(r"Requests per second:\s*([0-9.]+)",text)
lat=re.search(r"Time per request:\s*([0-9.]+)\s+\[ms\] \(mean\)",text)
print(f"vm,{sys.argv[2]},{sys.argv[3]},{sys.argv[4]},{rps.group(1) if rps else ''},{lat.group(1) if lat else ''},QEMU-{sys.argv[5]} guest 1536 MiB")
PY
