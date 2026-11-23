#!/usr/bin/env bash
set -Eeuo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence plots
exec > >(tee evidence/run.log) 2>&1
trap 'code=$?; echo "run failed at line $LINENO (exit $code)" | tee -a evidence/verification.txt; exit "$code"' ERR

echo '=== Environment ==='
{
  . /etc/os-release
  printf 'Linux: %s\n' "$PRETTY_NAME"
  uname -a
  gcc --version | head -n1
  python3 --version
  docker --version
  docker compose version
  if [[ -e /dev/kvm ]]; then echo 'Virtualization: KVM'; else echo 'Virtualization: QEMU TCG fallback'; fi
} | tee evidence/environment.txt

sudo apt-get update -qq
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -qq sysstat nginx ufw cron apache2-utils qemu-system-x86 qemu-utils cloud-image-utils > evidence/apt-install.log 2>&1

echo '=== B1.1 Linux observations ===' | tee evidence/b1-observations.txt
{
  echo '$ ps -eo pid,ppid,user,%cpu,%mem,rss,comm --sort=-%cpu | head -n 15'
  ps -eo pid,ppid,user,%cpu,%mem,rss,comm --sort=-%cpu | head -n 15
  echo '$ top -b -n1 | head -n 15'; top -b -n1 | head -n 15
  echo '$ free -h'; free -h
  echo '$ vmstat 1 3'; vmstat 1 3
  echo '$ iostat -xz 1 2'; iostat -xz 1 2
  echo '$ df -h'; df -h
  echo '$ ss -tulpen'; ss -tulpen
  echo '$ uname -r'; uname -r
  echo '$ head /proc/meminfo'; head /proc/meminfo
  echo '$ cat /sys/devices/system/cpu/online'; cat /sys/devices/system/cpu/online
} | tee -a evidence/b1-observations.txt

echo '=== B1.2 log analysis, disk alert and cron ==='
bash TaskB1/log_summary.sh | tee evidence/b1-log-summary.txt
bash TaskB1/log_summary_corrected.sh | tee evidence/b1-log-summary-independent.txt
grep -Fq '  200 4' evidence/b1-log-summary.txt
grep -Fq '  10.0.0.5 4' evidence/b1-log-summary.txt
grep -Fq 'Total bytes sent: 43200' evidence/b1-log-summary.txt
ALERT_THRESHOLD=0 bash TaskB1/disk-alert.sh | tee evidence/b1-disk-alert-boundary.txt
grep -F '0 * * * *' TaskB1/cron.txt | tee evidence/b1-cron.txt

echo '=== B1.3 systemd, Nginx and firewall ==='
if [[ ! -d /run/systemd/system ]] || ! command -v systemctl >/dev/null; then
  echo 'systemd is unavailable on this runner; service execution cannot be measured.' | tee evidence/b1-services.txt
  exit 1
fi
sudo bash TaskB1/setup-service.sh
sudo install -o result-portal -g result-portal -m 0640 /dev/null /var/log/disk-alert.log
sudo -u result-portal crontab -l 2>/dev/null > evidence/b1-cron-current.txt || true
sudo -u result-portal crontab TaskB1/cron.txt
sudo systemctl enable --now cron
sudo -u result-portal crontab -l | tee -a evidence/b1-cron.txt
sudo ufw --force reset >/dev/null
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow 22/tcp
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
sudo ufw --force enable
sudo ufw status verbose | tee evidence/b1-firewall.txt
sudo systemctl status --no-pager result-portal nginx | tee evidence/b1-services.txt
curl --retry 12 --retry-connrefused --retry-delay 2 -fsS http://127.0.0.1/health | tee evidence/b1-service-health.json
curl -fsS http://127.0.0.1/results/10001 | tee evidence/b1-service-result.json
sudo journalctl -u result-portal -n 15 --no-pager | tee evidence/b1-journal.txt

echo '=== B3.1 bare-metal deployment ==='
python3 -m venv .venv
.venv/bin/pip install --disable-pip-version-check -q -r webapp/requirements.txt
printf 'deployment,startup_seconds,rss_mib,size_mib,requests_per_second,mean_latency_ms,notes\n' > evidence/benchmark.csv
host_start=$(date +%s%3N)
APP_NAME='University Result Portal' .venv/bin/gunicorn --workers 2 --bind 127.0.0.1:8100 --access-logfile evidence/host-access.log --error-logfile evidence/host-error.log app:app >/dev/null 2>&1 &
host_pid=$!
for _ in $(seq 1 40); do
  if curl -fsS http://127.0.0.1:8100/health >/dev/null; then break; fi
  sleep .25
done
host_end=$(date +%s%3N)
host_start_s=$(awk -v s="$host_start" -v e="$host_end" 'BEGIN {printf "%.3f",(e-s)/1000}')
curl -fsS http://127.0.0.1:8100/health | tee evidence/benchmark-host-health.json
ab -n 2000 -c 25 -k http://127.0.0.1:8100/simulate/login > evidence/benchmark-host.txt
host_rss_kib=$(ps -o rss= -p "$host_pid" --ppid "$host_pid" | awk '{s+=$1} END {print s+0}')
host_rss_mib=$(awk -v x="$host_rss_kib" 'BEGIN {printf "%.1f",x/1024}')
app_size_mib=$(du -sm webapp | awk '{print $1}')
python3 - evidence/benchmark-host.txt "$host_start_s" "$host_rss_mib" "$app_size_mib" >> evidence/benchmark.csv <<'PY'
import re,sys
s=open(sys.argv[1]).read(); r=re.search(r"Requests per second:\s*([0-9.]+)",s); l=re.search(r"Time per request:\s*([0-9.]+)\s+\[ms\] \(mean\)",s)
print(f"bare-metal,{sys.argv[2]},{sys.argv[3]},{sys.argv[4]},{r.group(1) if r else ''},{l.group(1) if l else ''},Linux host")
PY
echo '=== B3.1 VM deployment ==='
bash scripts/run_vm_benchmark.sh

echo '=== B2 image comparison and Compose deployment ==='
full_s=$(date +%s%3N)
docker build -f TaskB2/Dockerfile.full -t result-portal:full . > evidence/docker-build-full.txt
full_e=$(date +%s%3N)
slim_s=$(date +%s%3N)
docker build -f TaskB2/Dockerfile -t result-portal:compose . > evidence/docker-build-slim.txt
slim_e=$(date +%s%3N)
multi_s=$(date +%s%3N)
docker build -f TaskB2/Dockerfile.multistage -t result-portal:multistage . > evidence/docker-build-multistage.txt
multi_e=$(date +%s%3N)
ubuntu_s=$(date +%s%3N)
docker build -f TaskB2/Dockerfile.ubuntu -t result-portal:ubuntu . > evidence/docker-build-ubuntu.txt
ubuntu_e=$(date +%s%3N)
{
  docker image ls result-portal
  printf 'build_ms full_python=%s slim_python=%s multistage=%s ubuntu_base=%s\n' "$((full_e-full_s))" "$((slim_e-slim_s))" "$((multi_e-multi_s))" "$((ubuntu_e-ubuntu_s))"
  for tag in full compose multistage ubuntu; do
    echo "docker history result-portal:$tag"
    docker history --no-trunc result-portal:"$tag" | head -n 12
    docker image inspect result-portal:"$tag" --format 'image={{.RepoTags}} bytes={{.Size}}'
    docker run --rm --entrypoint sh result-portal:"$tag" -c 'printf "installed_deb_packages="; dpkg-query -W -f="\${binary:Package}\n" | wc -l' 2>/dev/null || true
  done
} | tee evidence/b2-images.txt
docker compose -f TaskB2/compose.yaml up -d --build | tee evidence/b2-compose-up.txt
ready=0
for _ in $(seq 1 60); do
  if curl -fsS http://127.0.0.1:8000/health >/dev/null && curl -fsS http://127.0.0.1:8001/health >/dev/null; then ready=1; break; fi
  sleep 2
done
[[ $ready -eq 1 ]]
curl -fsS http://127.0.0.1:8000/health | tee evidence/b2-portal-health.json
curl -fsS http://127.0.0.1:8000/results/10001 | tee evidence/b2-db-result.json
unknown_code=$(curl -sS -o evidence/b2-unknown-result.json -w '%{http_code}' http://127.0.0.1:8000/results/99999)
echo "unknown_student_http_status=$unknown_code" | tee evidence/b2-boundary.txt
[[ "$unknown_code" == 404 ]]
docker compose -f TaskB2/compose.yaml ps | tee evidence/b2-compose-ps.txt
docker stats --no-stream | tee evidence/b2-stats.txt
docker exec "$(docker compose -f TaskB2/compose.yaml ps -q portal)" whoami | tee evidence/b2-container-user.txt
docker compose -f TaskB2/compose.yaml down | tee evidence/b2-compose-down.txt
docker compose -f TaskB2/compose.yaml up -d | tee evidence/b2-compose-restart.txt
for _ in $(seq 1 30); do
  if curl -fsS http://127.0.0.1:8000/results/10001 >/dev/null; then break; fi
  sleep 2
done
curl -fsS http://127.0.0.1:8000/results/10001 | tee evidence/b2-volume-persistence.json
docker compose -f TaskB2/compose.yaml down

echo '=== B3.1 container deployment and benchmark ==='
container_start=$(date +%s%3N)
docker run -d --rm --name portal-bench --cpus=0.5 --memory=512m -p 127.0.0.1:8000:8000 result-portal:compose | tee evidence/benchmark-container-id.txt
for _ in $(seq 1 40); do
  if curl -fsS http://127.0.0.1:8000/health >/dev/null; then break; fi
  sleep .25
done
container_end=$(date +%s%3N)
container_start_s=$(awk -v s="$container_start" -v e="$container_end" 'BEGIN {printf "%.3f",(e-s)/1000}')
ab -n 2000 -c 25 -k http://127.0.0.1:8000/simulate/login > evidence/benchmark-container.txt
docker stats --no-stream portal-bench | tee evidence/benchmark-container-stats.txt
container_size_bytes=$(docker image inspect result-portal:compose --format '{{.Size}}')
container_size_mib=$(awk -v x="$container_size_bytes" 'BEGIN {printf "%.1f",x/1048576}')
container_rss_mib=$(docker stats --no-stream --format '{{.MemUsage}}' portal-bench | python3 -c 'import re,sys; s=sys.stdin.read().split(" /")[0]; m=re.fullmatch(r"([0-9.]+)(B|KiB|MiB|GiB)",s); print(round(float(m[1])*{"B":1/1048576,"KiB":1/1024,"MiB":1,"GiB":1024}[m[2]],1) if m else 0)')
python3 - evidence/benchmark-container.txt "$container_start_s" "$container_rss_mib" "$container_size_mib" >> evidence/benchmark.csv <<'PY'
import re,sys
s=open(sys.argv[1]).read(); r=re.search(r"Requests per second:\s*([0-9.]+)",s); l=re.search(r"Time per request:\s*([0-9.]+)\s+\[ms\] \(mean\)",s)
print(f"container,{sys.argv[2]},{sys.argv[3]},{sys.argv[4]},{r.group(1) if r else ''},{l.group(1) if l else ''},Docker image; 0.5 CPU / 512 MiB cap")
PY
docker rm -f portal-bench >/dev/null

echo '=== B3.3 kind three-pod cluster, deletion recovery, load and scale ==='
curl -fsSLo /usr/local/bin/kind https://kind.sigs.k8s.io/dl/v0.33.0/kind-linux-amd64
chmod +x /usr/local/bin/kind
if ! command -v kubectl >/dev/null; then
  curl -fsSLo /usr/local/bin/kubectl https://dl.k8s.io/release/v1.37.0/bin/linux/amd64/kubectl
  chmod +x /usr/local/bin/kubectl
fi
kind create cluster --name assignment5 --config TaskB3/kind-config.yaml --wait 180s | tee evidence/b3-kind-create.txt
kind load docker-image result-portal:compose --name assignment5 | tee evidence/b3-kind-load.txt
kubectl apply -f TaskB3/deployment.yaml | tee evidence/b3-apply.txt
kubectl rollout status deployment/result-portal --timeout=180s | tee evidence/b3-initial-rollout.txt
kubectl get pods -l app=result-portal -o wide | tee evidence/b3-pods-three.txt
old_pod=$(kubectl get pod -l app=result-portal -o jsonpath='{.items[0].metadata.name}')
kubectl describe pod "$old_pod" | tee evidence/b3-pod-resources.txt
echo "Deleting pod $old_pod" | tee evidence/b3-pod-delete.txt
kubectl delete pod "$old_pod" | tee -a evidence/b3-pod-delete.txt
kubectl rollout status deployment/result-portal --timeout=180s | tee evidence/b3-recovery.txt
kubectl get pods -l app=result-portal -o wide | tee evidence/b3-pods-recovered.txt

echo '=== B4.1 second workload with independent resource limits ==='
kubectl apply -f TaskB4/additional-service.yaml | tee evidence/b4-apply.txt
kubectl rollout status deployment/lms --timeout=180s | tee evidence/b4-lms-ready.txt
kubectl get deployment lms -o yaml | tee evidence/b4-lms-resources.yaml
kubectl port-forward service/result-portal 18000:80 > evidence/b3-port-forward.txt 2>&1 &
pf_pid=$!
for _ in $(seq 1 30); do
  if curl -fsS http://127.0.0.1:18000/health >/dev/null; then break; fi
  sleep 1
done
python3 TaskB3/load_burst.py http://127.0.0.1:18000/simulate/login 20 30 > evidence/b3-load-under-scale.txt &
load_pid=$!
sleep 2
kubectl scale deployment/result-portal --replicas=6 | tee evidence/b3-scale-six.txt
kubectl rollout status deployment/result-portal --timeout=180s | tee evidence/b3-six-ready.txt
wait "$load_pid"
kill "$pf_pid" 2>/dev/null || true
kubectl get deployments,pods -o wide | tee evidence/b3-final-pods.txt
kind delete cluster --name assignment5 | tee evidence/b3-kind-delete.txt
kill "$host_pid" 2>/dev/null || true

echo '=== B4 capacity, annual compute-only sensitivity and plots ==='
python3 TaskB4/capacity_estimate.py evidence/benchmark.csv | tee evidence/b4-capacity-cost.txt
printf 'deployment,startup_seconds,rss_mib,size_mib,requests_per_second,mean_latency_ms,notes\ncontainer,0,0,0,1,0,test\n' > evidence/capacity-low.csv
python3 TaskB4/capacity_estimate.py evidence/capacity-low.csv > evidence/capacity-low.txt
printf 'deployment,startup_seconds,rss_mib,size_mib,requests_per_second,mean_latency_ms,notes\ncontainer,0,0,0,10000,0,test\n' > evidence/capacity-high.csv
python3 TaskB4/capacity_estimate.py evidence/capacity-high.csv > evidence/capacity-high.txt
grep -Fq 'conservative_10_request_replica_count=ceil(peak/(measured_rps*0.70))=1489' evidence/capacity-low.txt
grep -Fq 'three-replica_safety_floor=3' evidence/capacity-high.txt
python3 scripts/plot_results.py
printf 'OK: B1 TODO output, Nginx/systemd/ufw health, Docker build/user/limits/DB volume, three deployment rates, pod replacement, six replicas during load, LMS limits, capacity model and charts.\n' | tee evidence/verification.txt
