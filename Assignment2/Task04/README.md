# Q4 - Inter-Process Communication

Build each C file with `gcc -std=c11 -Wall -Wextra -Wpedantic -pthread file.c -o program`. `pipe_todo.c` completes the supplied pipe TODO and demonstrates EOF when the writer closes its descriptor. `ipc_benchmark.c` measures pipe and POSIX shared-memory transfer/scan for 1, 10 and 100 MiB. Run each binary. Results: `../evidence/task04-pipe.txt` and `../evidence/task04-ipc.csv`.
