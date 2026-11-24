# Assignment 4 run evidence

The source programs were compiled and run locally with MSYS2 UCRT64 GCC on Windows 11. Those outputs and the derived plots are retained here. Linux-specific inspection commands (`findmnt`, `stat`, `df -i`, `ls -i`, `dumpe2fs`, `lsblk`, and `/sys` scheduler inspection) run in the successful Ubuntu 24.04 GitHub Actions job linked below; its artifact contains the Linux machine outputs, compiler flags, program results, plots, and verification log.

- [Successful Ubuntu workflow run](https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/45046223629)
- [Open the run's artifacts](https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/45046223629#artifacts)
- [Workflow definition](../.github/workflows/assignment4-linux.yml)

Reproduce the Linux run with `bash scripts/run_linux_evidence.sh` in Ubuntu 24.04. Disk and streaming queue timings are stated-assumption simulations, not physical benchmarks. Linux virtual-device values describe the hosted runner.


## OnlineGDB screenshot

- onlinegdb/task02-fragmentation-demo.jpg: OnlineGDB compile/run view for Task02/fragmentation_demo.c.
