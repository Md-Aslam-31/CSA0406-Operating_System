# Q6 - Deadlock and Banker’s Algorithm

Build both with `gcc -std=c11 -Wall -Wextra -Wpedantic -pthread banker_todo.c -o banker_todo` and `gcc -std=c11 -Wall -Wextra -Wpedantic -pthread banker.c -o banker`; run both. `banker_todo.c` completes the supplied TODO code; `banker.c` adds measurements of repeated safety scans on reverse-ordered safe states with 10, 100 and 1,000 processes. Evidence: `../evidence/task06-banker*.{csv,txt}`; analysis in `../REPORT.md`.
