#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence plots
{ . /etc/os-release; printf 'Linux: %s\nCompiler: ' "$PRETTY_NAME"; gcc --version | head -n1; getconf PAGESIZE | sed 's/^/System base page size: /'; } | tee evidence/build-environment.txt
bash scripts/run_linking.sh
for spec in 'Task02/paging_todo.c:Task02/paging_todo' 'Task02/paging_lab.c:Task02/paging_lab' 'Task03/segment_lab.c:Task03/segment_lab' 'Task04/virtual_memory.c:Task04/virtual_memory' 'Task05/lru_todo.c:Task05/lru_todo' 'Task05/replacement_lab.c:Task05/replacement_lab' 'Task06/disk_analysis.c:Task06/disk_analysis' 'Task07/memory_plan.c:Task07/memory_plan'; do
  src=${spec%%:*}; bin=${spec#*:}; command="gcc -std=c11 -Wall -Wextra -pthread $src -o $bin"
  if [[ "$src" == Task06/* ]]; then command="$command -lm"; fi
  echo "$command" | tee -a evidence/build-environment.txt
  eval "$command"
done
run_capture(){ echo "Run $1"; timeout 120s "$1" > "$2" 2>&1; }
run_capture Task02/paging_todo evidence/task02-todo-1024.txt
sed 's/#define PAGE_SIZE 1024/#define PAGE_SIZE 2048/' Task02/paging_todo.c > Task02/paging_todo_2048.c
gcc -std=c11 -Wall -Wextra Task02/paging_todo_2048.c -o Task02/paging_todo_2048
run_capture Task02/paging_todo_2048 evidence/task02-todo-2048.txt
run_capture Task02/paging_lab evidence/task02-paging.txt
/usr/bin/time -v Task02/paging_lab >/dev/null 2> evidence/task02-time.txt
run_capture Task03/segment_lab evidence/task03-segmentation.txt
ulimit -c 0
/usr/bin/time -v Task04/virtual_memory > evidence/task04-memory.txt 2> evidence/task04-time.txt
run_capture Task05/lru_todo evidence/task05-todo.txt
run_capture Task05/replacement_lab evidence/task05-replacement.csv
run_capture Task06/disk_analysis evidence/task06-raid.txt
{
  echo 'Linux block devices (ROTA=1 rotational, 0 non-rotational):'; lsblk -dn -o NAME,ROTA,TYPE,SIZE || true
  echo 'Partitions:'; (sudo -n fdisk -l 2>&1 || fdisk -l 2>&1 || true)
  echo 'Filesystem and swap:'; findmnt -T /tmp -o SOURCE,FSTYPE,TARGET; swapon --show
  tmpfile=$(mktemp /tmp/os-disk-read.XXXXXX); dd if=/dev/zero of="$tmpfile" bs=1M count=32 conv=fsync status=none
  echo 'Read speed of runner filesystem file (not classified as SSD/HDD unless lsblk identifies the device):'
  /usr/bin/time -f 'elapsed=%e s max_rss=%M KiB' dd if="$tmpfile" of=/dev/null bs=1M
  rm -f "$tmpfile"
} > evidence/task06-disk.txt 2>&1
run_capture Task07/memory_plan evidence/task07-memory-plan.txt
python3 scripts/plot_results.py
grep -q 'physical 5120' evidence/task02-todo-1024.txt
grep -q 'Belady string, FIFO: 3 frames 9 faults, 4 frames 10 faults' evidence/task05-todo.txt
grep -q '3 frames: FIFO 15, LRU 12, Optimal 9' evidence/task05-todo.txt
grep -q 'control faults without pin=.* with pin=0' evidence/task07-memory-plan.txt
test -s plots/replacement_faults.png && test -s plots/page_faults.png
echo 'All Assignment 3 programs ran and key expected-output checks passed.' | tee evidence/verification.txt
