# Assignment 3 execution evidence

Captured from GitHub Actions run #44586278245 (Ubuntu 24.04.5 LTS, GCC 13.3.0):
https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/44586278245

The workflow built all seven task programs, executed the completed TODO programs, generated the page-fault charts, and passed the expected-output checks (`verification.txt`). `build-environment.txt` preserves compiler and command details; the remaining files preserve raw output and measurements. Times and hardware-specific behavior are specific to the hosted runner. Its virtual disk and shared Linux kernel cannot certify physical disk comparisons or hard real-time deadlines.


## OnlineGDB screenshot

- onlinegdb/task05-replacement-lab.jpg: OnlineGDB compile/run view for Task05/replacement_lab.c.

## Complete OnlineGDB run screenshots

- `onlinegdb/linked_demo-full.jpg` — linked program with `crypto.c` and checksum output.
- `onlinegdb/dlopen_demo-full.jpg` — OnlineGDB missing `.so` diagnostic; native task artifacts are retained in evidence.
- `onlinegdb/paging_lab-full.jpg`, `segment_lab-full.jpg`, `virtual_memory-full.jpg` — Q2–Q4 outputs.
- `onlinegdb/replacement_lab-full.jpg` and `replacement_lab-output-start.jpg` — Q5 benchmark end and locality table.
- `onlinegdb/disk_analysis-full.jpg`, `memory_plan-full.jpg` — Q6/Q7 outputs.
