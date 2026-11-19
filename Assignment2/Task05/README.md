# Q5 - Synchronisation

Build with `gcc -std=c11 -Wall -Wextra -Wpedantic -pthread file.c -o program`. `bounded_buffer_todo.c` completes the supplied semaphore/mutex program. `bounded_buffer.c` checks exact-once delivery with 1/1 and 4/4 producers/consumers, repeats the 4/4 test 100 times, then benchmarks global, per-train and per-coach lock scopes. Results: `../evidence/task05-buffer.txt` and `../evidence/task05-todo.txt`.
