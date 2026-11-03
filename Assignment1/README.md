# Assignment 1 — Repository Guide

This folder contains the standalone Assignment 1 submission: `REPORT.md`, Task06 and Task07 C programs, and reproducible Linux evidence scripts. Assignment 2 should be kept in a separate folder/repository as instructed by the course.

Student: Mohamed Aslam H · Register number: 192572147. The source PDFs specify Linux execution and strace/time evidence. Both programs and Linux measurements ran successfully in GitHub Actions on Ubuntu 24.04.5 LTS with GCC 13.3.0. Captured stdout, strace and timing output are committed under `evidence/`. The local authoring host is Windows with MSYS2 GCC 16.1.0.

## Files

- `REPORT.md`: answers all Tasks 1–12, alternatives, complexity, architecture and design challenge.
- `Task06/copy_file.c`: completed buffered `read()`/`write()` copy exercise.
- `Task07/shell_jobs.c`: completed `fork()`/`execlp()`/`waitpid()` exercise.
- `scripts/run_linux_evidence.sh`: compile/run and measurement helper for Ubuntu/Debian Linux.
- `evidence/`: captured Ubuntu CI output for Task06, Task07, strace, time, and process/thread measurements.

## Local compilation

Linux command required by assignment:

```sh
gcc -std=c11 -Wall -Wextra -pthread Task06/copy_file.c -o Task06/copy_file
gcc -std=c11 -Wall -Wextra -pthread Task07/shell_jobs.c -o Task07/shell_jobs
```

Run from each task folder. Task06 expected read counts are 10001, 21, and 4 for the three input buffer sizes. Task07 expects `42`, then exit status 0, then job 2 exit status 3. `expr`, `strace`, `/usr/bin/time`, and pthreads are required for the full Linux evidence run.

## Verified Linux result

GitHub Actions run 41688186418 completed successfully on Ubuntu 24.04.5 LTS using GCC 13.3.0. Task06 returned read counts 10001, 21, and 4. Task07 printed 42, job 1 status 0, and job 2 status 3. The run also captured `strace -c`, `/usr/bin/time -v`, and timings for 1,000 sequential processes and 1,000 threads. See `evidence/` and the [workflow run](https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/41688186418).
