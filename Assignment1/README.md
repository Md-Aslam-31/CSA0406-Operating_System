# CSA04 Operating Systems — Assignment 1

**Student:** Mohamed Aslam H · **Register:** 192572147 · **Department:** CSE (AI) · **Academic year:** 2025 · **Faculty:** Dr. Jayasakthi Velmurugan K

This is the standalone Assignment 1 submission. The formal report answers all 12 tasks. Linux program outputs and measurements were executed on Ubuntu 24.04.5 LTS with GCC 13.3.0 in GitHub Actions; see `evidence/` and the workflow link below. The Windows authoring host reports build 26200 and ProductName Windows 10 Pro; local WSL installation still requires an elevated install and restart.

## Contents

- `REPORT.md`: complete answers, design alternatives, complexity discussion, measurements and references.
- `Task-01/` through `Task-12/`: task-level writeups.
- `Task-06/copy_file.c`: buffered `read()`/`write()` copy program; `test_copy_file.c` covers boundary and error cases.
- `Task-07/shell_jobs.c`: `fork()`/`execlp()`/`waitpid()` example; `flush_demo.c` demonstrates buffered output duplication.
- `scripts/run_linux_evidence.sh`: reproducible compile, execution and measurement workflow.
- `evidence/`: captured raw stdout, `strace`, `/usr/bin/time`, boundary checks and process/thread timings.
- `Results/task_measurements.csv`: tabulated measured values.

## Build and run on Linux

```sh
gcc -std=c11 -Wall -Wextra -pthread Task-06/copy_file.c -o Task-06/task06
gcc -std=c11 -Wall -Wextra -pthread Task-07/shell_jobs.c -o Task-07/task07
(cd Task-06 && ./task06)
(cd Task-07 && ./task07)
```

Task-06 expected read counts are 10,001, 21 and 4 for buffers of 1, 512 and 4,096 bytes (including the EOF read). Task-07 prints `42`, job 1 exit status 0, and job 2 exit status 3. For the complete evidence suite, run `bash scripts/run_linux_evidence.sh`; it requires Linux, GCC, `strace` and `/usr/bin/time`.

## Verified Linux run

The standalone repository GitHub Actions run [#41692640241](https://github.com/Md-Aslam-31/CSA04-192572147-Assignment1/actions/runs/41692640241) succeeded on Ubuntu 24.04.5 LTS with GCC 13.3.0. The retained evidence files are raw workflow outputs. Workflow run [#41692084440](https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/41692084440) also passed the boundary and failure checks used to populate the evidence folder. Each Task folder has a task-scoped commit in the repository history. Timings are single-run samples and are not portable performance guarantees.
