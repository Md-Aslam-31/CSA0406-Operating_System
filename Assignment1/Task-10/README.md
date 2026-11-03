# Assignment 1 — Task 10

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 10 — Measurements

Ubuntu 24.04 run evidence is in `evidence/`. GCC 13.3 compiled both programs with `-std=c11 -Wall -Wextra -pthread`; Task-06 and Task-07 outputs matched expected behavior. One timed trial measured 1,000 sequential fork+wait children at 0.139150 s and creation/join of 1,000 no-op pthreads at 0.043700 s. In this run threads were about 3.2x faster, consistent with sharing the process address space, but this is one short runner sample and must not be generalized; scheduler load, CPU, libc and kernel affect it. The accompanying `/usr/bin/time -v` reports 0.18 s elapsed and 10,024 KiB maximum RSS for the combined benchmark. Task-06 total process timing was 0.01 s (user 0.00, system 0.01) for all three buffer copies; this timer is too coarse to distinguish individual sizes. The 1,000-process and 1,000-thread comparison includes wait/join costs. Processes provide stronger isolation and initially share pages copy-on-write; threads share code/data/address space but each has a stack and synchronization requirements.

See the [complete report](../REPORT.md) for the full submission and references.
