#!/usr/bin/env bash
set -euo pipefail
{
  echo 'Block devices and rotation flags:'
  lsblk -dn -o NAME,ROTA,TYPE,SIZE,MODEL || true
  echo 'Available Linux block schedulers:'
  found=0
  for sched in /sys/block/*/queue/scheduler; do
    [[ -r "$sched" ]] || continue
    found=1
    printf '%s: ' "$sched"
    cat "$sched"
  done
  if [[ $found -eq 0 ]]; then echo 'No host block scheduler sysfs paths exposed to this environment.'; fi
  echo 'The CI runner is virtualized; these values describe its guest-visible device, not the faculty/student laptop.'
} | tee evidence/task04-device.txt
