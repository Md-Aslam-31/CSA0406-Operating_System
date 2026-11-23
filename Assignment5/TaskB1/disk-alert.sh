#!/usr/bin/env bash
# Report a warning for any mounted filesystem above ALERT_THRESHOLD (default 80%).
set -euo pipefail
threshold=${ALERT_THRESHOLD:-80}
df -P -x tmpfs -x devtmpfs | awk -v limit="$threshold" 'NR > 1 {used=$5; gsub(/%/, "", used); if (used+0 >= limit) printf "ALERT filesystem=%s used=%s%% mount=%s\n", $1, used, $6; else printf "OK filesystem=%s used=%s%% mount=%s\n", $1, used, $6}'
