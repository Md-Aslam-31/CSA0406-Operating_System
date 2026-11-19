#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence
{ . /etc/os-release; printf 'OS: %s\n' "$PRETTY_NAME"; printf 'Compiler: '; gcc --version | head -n1; } | tee evidence/build-environment.txt
for spec in 'Task01/process_demo.c:Task01/process_demo' 'Task02/scheduling.c:Task02/scheduling' 'Task02/round_robin_todo.c:Task02/round_robin_todo' 'Task03/thread_pool.c:Task03/thread_pool' 'Task04/ipc_benchmark.c:Task04/ipc_benchmark' 'Task04/pipe_todo.c:Task04/pipe_todo' 'Task05/bounded_buffer.c:Task05/bounded_buffer' 'Task05/bounded_buffer_todo.c:Task05/bounded_buffer_todo' 'Task05/sync_primitives_bench.c:Task05/sync_primitives_bench' 'Task06/banker.c:Task06/banker' 'Task06/banker_todo.c:Task06/banker_todo'; do
  src=${spec%%:*}; bin=${spec#*:}
  echo "gcc -std=c11 -Wall -Wextra -Wpedantic -pthread $src -o $bin" | tee -a evidence/build-environment.txt
  gcc -std=c11 -Wall -Wextra -Wpedantic -pthread "$src" -o "$bin"
done
run_capture() { echo "Running $1"; timeout 120s "$1" > "$2" 2>&1; }
run_capture Task01/process_demo evidence/task01-processes.txt
run_capture Task02/scheduling evidence/task02-scheduling.txt
run_capture Task02/round_robin_todo evidence/task02-round-robin-todo.txt
run_capture Task03/thread_pool evidence/task03-thread-pool.csv
run_capture Task04/ipc_benchmark evidence/task04-ipc.csv
run_capture Task04/pipe_todo evidence/task04-pipe.txt
run_capture Task05/bounded_buffer evidence/task05-buffer.txt
run_capture Task05/bounded_buffer_todo evidence/task05-todo.txt
run_capture Task05/sync_primitives_bench evidence/task05-primitives.csv
run_capture Task06/banker evidence/task06-banker.csv
run_capture Task06/banker_todo evidence/task06-banker-todo.txt
grep -q 'Average TAT = 20.00, average WT = 13.50' evidence/task02-round-robin-todo.txt
grep -q 'analyser: 6 readings, average 35.5, max 41' evidence/task04-pipe.txt
grep -q 'sum = 210 (expected 210)' evidence/task05-todo.txt
grep -q 'initial state safe: P1 P3 P4 P0 P2' evidence/task06-banker.csv
grep -q 'P1 request: granted' evidence/task06-banker.csv
grep -q 'P4 request: must wait' evidence/task06-banker.csv
grep -q 'P0 request: denied (unsafe)' evidence/task06-banker.csv
grep -q 'P0 request: denied (unsafe)' evidence/task06-banker-todo.txt
printf 'All Assignment 2 programs compiled and expected-output assertions passed.\n' | tee evidence/verification.txt
