# CSA0406 Operating Systems - Assignment 2 (CO2)

**Student:** Mohamed Aslam H | **Register:** 192572147 | **Department:** CSE (AI) | **Academic year:** 2025 | **Faculty:** Dr. Jayasakthi Velmurugan K

This is a separate Assignment 2 submission under the existing course repository. The full written response is in [REPORT.md](REPORT.md). Each task folder contains its C implementation and task notes. `evidence/` contains captured Linux output; `scripts/run_linux_evidence.sh` rebuilds and regenerates it on Linux.

## Contents

- `Task01` through `Task06`: source code and task notes.
- `REPORT.md`: all responses, design evaluations, diagrams, complexity and interpretation.
- `evidence/`: stdout and measurement tables produced by the workflow.
- `.github/workflows/assignment2-linux.yml`: Ubuntu 24.04 build/run workflow.

## Build and run

Environment used for the retained run: Ubuntu 24.04 GitHub Actions runner and GCC 13.3.0. Compile command for all programs:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -pthread source.c -o program
```

From this folder, run the complete suite:

```sh
bash scripts/run_linux_evidence.sh
```

Run an individual binary after compiling it with the commands in its task README. Linux is required for `fork`, `/proc`, POSIX shared memory, pipes and POSIX semaphores. Timings are measurements of this runner, not portable guarantees. Process/thread creation overhead and CI CPU scheduling affect them.

## Repository

https://github.com/Md-Aslam-31/CSA0406-Operating_System/tree/main/Assignment2
