# Assignment 1 — Task 05

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 5 — System calls

System-call categories: process control; file management; device management; information maintenance; communication; and protection. For `read(fd, buf, count)`, a C library wrapper places arguments according to the ABI and executes the syscall instruction/trap [1]. CPU changes privilege to kernel mode and enters the syscall handler; kernel validates descriptor, permissions and user buffer, requests data from cache/device (possibly sleeping), copies available bytes to user space, sets return value/error, then returns to user mode. A read may return fewer bytes than requested [1]. Parameters are commonly in registers, or a pointer to a memory block; some ABIs/stubs also use the stack.

`fopen`/`fread`/`fwrite` are C library interfaces. `fopen` usually makes an `openat`-family syscall. Buffered `fread` can satisfy many reads from a library buffer; with n bytes and buffer B, data calls are roughly ceil(n/B), plus open/close and possible metadata calls, versus about ceil(n/b) for direct reads with b-byte chunks. Library buffering costs approximately B bytes per stream plus bookkeeping and may defer errors until flush/close. Direct syscalls avoid stream buffering but require short-read/short-write handling.

Linux evidence: `evidence/task06-strace-summary.txt` is the actual Ubuntu 24.04 `strace -c` output [2] for the Task-06 executable, which is also the real read/write workload discussed here. It reports 10,027 read calls overall for the complete process (including runtime and file setup), while the copy loops themselves print 10,001 + 21 + 4 = 10,026 reads. This small difference is startup/runtime I/O outside the three measured copy loops. The full workflow output is linked from the repository Actions page.

See the [complete report](../REPORT.md) for the full submission and references.
