#!/usr/bin/env bash
set -euo pipefail
repo_root=$(cd "$(dirname "$0")/.." && pwd)
if [[ $(id -u) -ne 0 ]]; then echo 'Run with sudo.' >&2; exit 1; fi
id result-portal >/dev/null 2>&1 || useradd --system --home-dir /opt/result-portal --shell /usr/sbin/nologin result-portal
install -d -o result-portal -g result-portal -m 0750 /opt/result-portal
install -o result-portal -g result-portal -m 0640 "$repo_root/webapp/app.py" /opt/result-portal/app.py
install -o result-portal -g result-portal -m 0640 "$repo_root/webapp/requirements.txt" /opt/result-portal/requirements.txt
python3 -m venv /opt/result-portal/.venv
/opt/result-portal/.venv/bin/pip install --disable-pip-version-check -r /opt/result-portal/requirements.txt
install -m 0644 "$repo_root/TaskB1/result-portal.service" /etc/systemd/system/result-portal.service
install -m 0644 "$repo_root/TaskB1/nginx-result-portal.conf" /etc/nginx/sites-available/result-portal
ln -sfn /etc/nginx/sites-available/result-portal /etc/nginx/sites-enabled/result-portal
rm -f /etc/nginx/sites-enabled/default
install -m 0755 "$repo_root/TaskB1/disk-alert.sh" /usr/local/sbin/disk-alert.sh
install -o result-portal -g result-portal -m 0640 /dev/null /var/log/disk-alert.log
systemctl daemon-reload
systemctl enable --now result-portal
nginx -t
systemctl enable --now nginx
