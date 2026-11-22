# CSA0406 Operating Systems - Assignment 4 (CO4)

**Student:** Mohamed Aslam H | **Register:** 192572147 | **Department:** CSE (AI) | **Academic year:** 2025 | **Faculty:** Dr. Jayasakthi Velmurugan K

Assignment 4 is in its own folder in the course repository. The complete report is available as `REPORT.pdf`, `REPORT.docx` and `REPORT.md`. `Task01`–`Task06` contain the task work and source. `evidence/` holds local program output and links to the Linux CI run; `plots/` holds charts generated from the local seeded simulation output.

## Build and run

The full Linux suite completed successfully in GitHub Actions on Ubuntu 24.04. [View the successful run and download its Linux evidence artifact](https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/45046223629). Exact commands are in `evidence/build-environment.txt` for local C captures, and the Linux artifact includes its own environment/compiler record. To reproduce in Linux:

    bash scripts/run_linux_evidence.sh

The runner builds all C programs, runs both completed TODO exercises, records filesystem and block-scheduler evidence, simulates disk and streaming workloads, generates figures, and checks expected outputs. Commands requiring Linux include `/proc` and `/sys` inspection, `findmnt`, `dumpe2fs`, `lsblk` and Linux filesystem tools.

## Run individual programs

From this folder in Ubuntu:

| Work | Compile and run |
|---|---|
| First-fit TODO output | gcc -std=c11 -O2 -Wall -Wextra -pthread Task02/allocation_todo.c -o Task02/allocation_todo && Task02/allocation_todo |
| Fragmentation demonstration | gcc -std=c11 -O2 -Wall -Wextra -pthread Task02/fragmentation_demo.c -o Task02/fragmentation_demo && Task02/fragmentation_demo |
| Cost calculations | gcc -std=c11 -O2 -Wall -Wextra -pthread Task02/allocation_lab.c -o Task02/allocation_lab && Task02/allocation_lab |
| File-system commands | bash Task03/inspect_filesystem.sh |
| Completed SSTF TODO | gcc -std=c11 -O2 -Wall -Wextra -pthread Task04/disk_todo.c -o Task04/disk_todo && Task04/disk_todo |
| Scheduling simulation / disk scheduler | gcc -std=c11 -O2 -Wall -Wextra -pthread Task04/scheduler_sim.c -o Task04/scheduler_sim && Task04/scheduler_sim, then bash Task04/inspect_scheduler.sh |
| Streaming design simulation | gcc -std=c11 -O2 -Wall -Wextra -pthread Task06/streaming_sim.c -lm -o Task06/streaming_sim && Task06/streaming_sim |

Task 1 and Task 5 are design/evaluation write-ups. The Linux CI artifact and local command captures are explained in `evidence/README.md`.

| Task | Result |
|---|---|
| T1 | Private student tree and ACL-scoped project sharing design; privacy defence and cloud alternative |
| T2 | Completed first-fit allocator; inode/FAT/bitmap cost calculations; separate real fragmentation demonstration |
| T3 | Seven-filesystem comparison, ext4 crash modes, recommendations and Linux inode/superblock output |
| T4 | Completed SSTF code; all six algorithms, movement calculations, load simulation and scheduler inspection |
| T5 | NFS/distributed/object storage comparison, replication, consistency and device-management analysis |
| T6 | Capacity and throughput design with two alternative queueing simulations |

The Assignment 4 question contains a contradictory sentence about contiguous allocation. The supplied example code and expected output are both retained and run; the report explains the inconsistency and includes a separate fragmentation case where indexed allocation is needed.

## Repository

https://github.com/Md-Aslam-31/CSA0406-Operating_System/tree/main/Assignment4

Linux evidence workflow: https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/workflows/assignment4-linux.yml
