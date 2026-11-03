# Assignment 1 — Task 07

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 7 — Process control

`fork()` creates a child (typically using copy-on-write) [3]; `exec()` replaces the child image [4]; `wait()`/`waitpid()` reaps termination and returns status [5]; `exit()` flushes user-space streams and terminates, whereas `_exit()` terminates without stdio flushing. The shell forks, child execs the requested command, parent waits/reaps and reports status. The completed `Task-07/shell_jobs.c` implements `expr 6 * 7` and a child exiting 3; it ran on Ubuntu 24.04 and produced the expected output preserved in `evidence/task07-output.txt`.

Process sketch:

```text
shell (parent) --fork--> child --exec(expr)--> prints 42, exits 0
       |--waitpid------------------------------^ reports status 0
shell --fork--> child --_exit(3)
       |--waitpid------------------------------^ reports status 3
```

A zombie is a terminated child whose parent has not yet collected status; `wait` reaps it. An orphan has a living process after its parent exits and is reparented to the system’s designated subreaper/init, which can reap it. `fork` plus `exec` allows the shell to set up redirection, pipes and environment between creation and program replacement; `posix_spawn` can combine common steps efficiently. `fflush` before `fork` prevents a buffered line copied into both processes from being emitted twice on later flush. With redirected output, `exit(3)` flushes the inherited duplicate buffer; `_exit(3)` does not. Removing the second flush before forking the second child therefore duplicates the buffered “job 2” line when the child calls `exit`.

Alternative: `posix_spawn` may lower start-up overhead for simple commands; performance for 1,000 launches must be benchmarked on the same host. Threads share address space and most process resources, reducing creation/memory cost, but commands then share mutable state and are not isolated; each thread needs its own stack. Processes have separate virtual address spaces (initially copy-on-write) and stronger fault/security isolation. A database shell should prefer process isolation for arbitrary commands.

See the [complete report](../REPORT.md) for the full submission and references.
