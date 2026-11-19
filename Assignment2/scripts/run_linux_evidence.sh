#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence
printf 'OS: '; . /etc/os-release; echo "$PRETTY_NAME"
printf 'Compiler: '; gcc --version | head -n1
for spec in 'Task01/process_demo.c:Task01/process_demo' 'Task02/scheduling.c:Task02/scheduling' 'Task02/round_robin_todo.c:Task02/round_robin_todo' 'Task03/thread_pool.c:Task03/thread_pool' 'Task04/ipc_benchmark.c:Task04/ipc_benchmark' 'Task04/pipe_todo.c:Task04/pipe_todo' 'Task05/bounded_buffer.c:Task05/bounded_buffer' 'Task05/bounded_buffer_todo.c:Task05/bounded_buffer_todo' 'Task06/banker.c:Task06/banker' 'Task06/banker_todo.c:Task06/banker_todo'; do
  src=${spec%%:*}; bin=${spec#*:}
  echo "gcc -std=c11 -Wall -Wextra -Wpedantic -pthread $src -o $bin"
  gcc -std=c11 -Wall -Wextra -Wpedantic -pthread "$src" -o "$bin"
done
Task01/process_demo > evidence/task01-processes.txt 2>&1
Task02/scheduling > evidence/task02-scheduling.txt
Task02/round_robin_todo > evidence/task02-round-robin-todo.txt
Task03/thread_pool > evidence/task03-thread-pool.csv
Task04/ipc_benchmark > evidence/task04-ipc.csv
Task04/pipe_todo > evidence/task04-pipe.txt
Task05/bounded_buffer > evidence/task05-buffer.txt
Task05/bounded_buffer_todo > evidence/task05-todo.txt
Task06/banker > evidence/task06-banker.csv
Task06/banker_todo > evidence/task06-banker-todo.txt
grep -q 'Average TAT = 20.00, average WT = 13.50' evidence/task02-round-robin-todo.txt
grep -q 'analyser: 6 readings, average 35.5, max 41' evidence/task04-pipe.txt
grep -q 'sum = 210 (expected 210)' evidence/task05-todo.txt
grep -q 'initial state safe: P1 P3 P4 P0 P2' evidence/task06-banker.csv
grep -q 'P1 request: granted' evidence/task06-banker.csv
grep -q 'P4 request: must wait' evidence/task06-banker.csv
grep -q 'P0 request: denied (unsafe)' evidence/task06-banker.csv
grep -q 'P0 request: denied (unsafe)' evidence/task06-banker-todo.txt
printf 'All Assignment 2 programs compiled and expected-output assertions passed.\n' | tee evidence/verification.txt
