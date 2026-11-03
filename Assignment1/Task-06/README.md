# Assignment 1 — Task 06

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 6 — Cost of system calls and completed program

`Task-06/copy_file.c` fills the loop, retries interrupted calls, handles partial writes, validates buffer size and checks errors. For the 10,000-byte input, with one final EOF read counted, expected calls are ceil(10000/b)+1: 10001 for b=1; 21 for b=512; 4 for b=4096. The program compiled with GCC 13.3.0 on Ubuntu and produced those exact counts. The same output was also reproduced locally with MSYS2 GCC 16.1.0. Linux `strace -c` reports process-wide syscall totals, including startup and stdio, rather than only the function’s read counter.

| Buffer | Copy reads (measured by program) | Approx. user buffer | Assessment |
|---:|---:|---:|---|
| 1 B | 10,001 | 1 B | Extreme syscall overhead; reject. |
| 512 B | 21 | 512 B | Far fewer calls; still more dispatches. |
| 4096 B | 4 | 4096 B | Strong reduction with tiny memory cost for this workload. |

For backup, begin with 64 KiB–1 MiB buffered I/O and benchmark throughput, memory pressure and device behavior on actual Linux hardware; 4 KiB wins among the supplied sizes by syscall count but cannot be called universally fastest without timing. A larger buffer reduces call count to O(n/b), while memory per copy remains O(b); copying itself is O(n). `mmap` can reduce explicit read/write calls and copies [9], but still incurs page faults, mapping calls, page-cache work and possible writeback; touching a huge mapping uses virtual address space O(n), while resident memory is workload-dependent and constrained by cache eviction. For bounded-memory backup, chunked I/O is easier to limit and recover from.

See the [complete report](../REPORT.md) for the full submission and references.
