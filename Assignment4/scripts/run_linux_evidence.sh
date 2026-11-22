#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence plots
{
  . /etc/os-release
  printf 'Linux: %s\nCompiler: ' "$PRETTY_NAME"
  gcc --version | head -n1
  echo 'Compile flags: gcc -std=c11 -O2 -Wall -Wextra -pthread'
  echo 'Filesystem mount:'
  findmnt -T . -o TARGET,SOURCE,FSTYPE
} | tee evidence/build-environment.txt

for spec in \
  'Task02/allocation_todo.c:Task02/allocation_todo:' \
  'Task02/fragmentation_demo.c:Task02/fragmentation_demo:' \
  'Task02/allocation_lab.c:Task02/allocation_lab:' \
  'Task04/disk_todo.c:Task04/disk_todo:' \
  'Task04/scheduler_sim.c:Task04/scheduler_sim:' \
  'Task06/streaming_sim.c:Task06/streaming_sim:-lm'; do
  src=$(echo "$spec" | cut -d: -f1)
  bin=$(echo "$spec" | cut -d: -f2)
  libs=$(echo "$spec" | cut -d: -f3)
  command="gcc -std=c11 -O2 -Wall -Wextra -pthread $src -o $bin $libs"
  echo "$command" | tee -a evidence/build-environment.txt
  eval "$command"
done

timeout 30s Task02/allocation_todo | tee evidence/task02-allocation.txt
timeout 30s Task02/fragmentation_demo | tee evidence/task02-fragmentation.txt
timeout 30s Task02/allocation_lab | tee evidence/task02-costs.txt
timeout 30s Task04/disk_todo | tee evidence/task04-todo.txt
timeout 120s Task04/scheduler_sim | tee evidence/task04-simulation.csv
bash Task03/inspect_filesystem.sh
bash Task04/inspect_scheduler.sh
timeout 120s Task06/streaming_sim | tee evidence/task06-simulation.txt
python3 scripts/plot_results.py

grep -q 'E (10 blocks) contiguous: start 19' evidence/task02-allocation.txt
grep -q 'largest run=7, contiguous 10-block allocation start=-1' evidence/task02-fragmentation.txt
grep -q 'FCFS   53 98 183 37 122 14 124 65 67  -> 640 cylinders' evidence/task04-todo.txt
grep -q 'SSTF   53 65 67 37 14 98 122 124 183  -> 236 cylinders' evidence/task04-todo.txt
grep -q '183 cylinders (+199 for the jump)' evidence/task04-todo.txt
grep -q 'origin_reads=5057/100000' evidence/task06-simulation.txt
test -s evidence/task03-filesystem.txt
test -s evidence/task04-device.txt
test -s plots/scheduler_wait.png && test -s plots/streaming_latency.png
echo 'All Assignment 4 programs ran; expected allocation/SSTF outputs and evidence checks passed.' | tee evidence/verification.txt
