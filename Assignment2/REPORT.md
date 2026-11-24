# CSA0406 Operating Systems - Assignment 2 (CO2)

## Cover

**Assignment:** Evaluate and Justify Process-Management and Deadlock-Handling Designs  
**Student:** Mohamed Aslam H  
**Register number:** 192572147  
**Department:** CSE (AI)  
**Academic year:** 2025  
**Faculty:** Dr. Jayasakthi Velmurugan K  
**Nature:** Individual  
**Repository:** https://github.com/Md-Aslam-31/CSA0406-Operating_System/tree/main/Assignment2  
**Execution:** Ubuntu 24.04.5 LTS GitHub Actions runner, GCC 13.3.0. The complete Linux run succeeded in [workflow run 44579401828](https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/44579401828). Raw output is in `evidence/`; `scripts/run_linux_evidence.sh` rebuilds all programs and verifies key expected results.

## Executive summary

Use a bounded worker pool for examination sessions, with asynchronous I/O and durable session state, rather than 2,000 heavyweight processes. For video encoding, use a multilevel feedback queue or a size-aware service with separate capacity for short jobs and long jobs; the measured single-CPU scheduling averages alone cannot describe a real multi-core service, so preserve fairness and throughput with admission control. Use a bounded POSIX thread pool for independent camera frames where shared read-only data makes threads efficient. Use pipes for simple streaming and EOF-based shutdown; use shared memory when repeated large transfers justify explicit synchronization. For ticketing, lock at train or coach granularity according to independent invariants, keeping the protected section short. For the lab cluster, Banker avoidance is appropriate only for a small, closed workload with credible maximum claims; otherwise use prevention for critical resource classes and detection/recovery for the remaining dynamic workload.

## Q1 - Processes and process states (6 marks)

### State model

```text
                 dispatch                 preemption
  New --admit--> Ready ------------------> Running -------> Ready
                   ^                         |  \
                   | I/O/event complete       |   \ exit
                   +------ Waiting <----------+    v
                           ^   block               Terminated
                           +--- I/O/event wait
```

The process control block (PCB) is the kernel's per-process record: PID and parent PID; process state; saved program counter and general registers; scheduling class, priority and CPU accounting; virtual-memory map/page tables; open-file table and current directory; signal mask/handlers; credentials; and pending wait/signal information. A context switch saves the outgoing register/PC state and restores the incoming state. Address-space changes may also disturb TLB/cache locality.

`Task01/process_demo.c` creates three children with `fork`, prints PID/PPID, invokes `ps` to inspect their state, reads each child's `/proc/<pid>/status` fields (state and memory), and waits for all exit statuses. In the recorded sample, all three children were sleeping (`S`), each showed 2,696 kB virtual size and 944 kB resident memory. These are single-run Ubuntu runner values, not a per-process constant; shared pages, allocator behavior and workload change the result. Captured output is `evidence/task01-processes.txt`. The short children can appear as `R` (running/runnable) or `S` (interruptible sleep) in this snapshot; state is momentary. `/proc/<pid>/status` and `/proc/<pid>/stat` expose Linux process details while the process exists.

### Evaluation and design decision

At 2,000 mostly waiting sessions, each process has an independent address space and kernel bookkeeping. Actual physical memory is workload- and kernel-dependent: shared executable/library pages are not 2,000 private copies, while private heaps, stacks, page tables and descriptors do scale. Context-switch cost includes register/kernel scheduling work, possible address-space/TLB effects and cache disruption; I/O-bound tasks spend much of their elapsed time blocked, so adding processes does not increase storage/network capacity. A fork-per-session design is simple to isolate but can waste memory and scheduler time and cause expensive bursts during login.

Decision: use a fixed, measured worker pool (processes where crash isolation matters) with an event-driven or asynchronous front end and bounded per-session state. Threads are lighter and share caches/address space, but a memory-corruption crash can terminate every session in that process and shared state needs synchronization. One process per session gives the strongest address-space boundary but highest memory and process management cost. A small process pool balances fault containment and resource bounds; avoid one worker permanently occupied by a slow socket by using nonblocking I/O or a separate I/O event loop.

With a thread per session, stacks can be configured to a few hundred KiB or less rather than allocating a full private address space, and thread switches within one process typically avoid address-space switching. The question's “few KB” is a possible minimum committed stack, not a universal default reservation; virtual stack reservation and resident use differ. A thread crash from a fatal process signal can still kill the entire server; isolate untrusted components in processes and use per-session exception/error handling for ordinary failures. A bounded thread pool is preferable to 2,000 active threads.

## Q2 - CPU scheduling (8 marks)

### Dataset and metrics

The supplied Round Robin program's workload is used consistently for algorithm comparisons: P1 (arrival 0, burst 8, priority 2), P2 (1, 4, 1), P3 (2, 9, 4), P4 (3, 5, 3). Lower priority number means higher priority. Completion time CT is finish time; turnaround TAT = CT - arrival; waiting WT = TAT - burst; response RT = first CPU start - arrival. `Task02/scheduling.c` prints Gantt segments, per-process values and means for FCFS, nonpreemptive SJF, SRTF, nonpreemptive priority and RR quantum 1, 3, 5 and 20. The exact TODO completion is separately kept in `Task02/round_robin_todo.c`; its output is checked against the handout's averages (TAT 20.00; WT 13.50).

The averages from `evidence/task02-scheduling.txt` are summarized below. Context switches count process-to-process dispatch changes, not timer interrupts or kernel overhead; the first dispatch is excluded. No wall-clock claims are inferred from these arithmetic schedules.

| Policy | Average TAT | Average WT | Average RT | Switches |
|---|---:|---:|---:|---:|
| FCFS | 15.25 | 8.75 | 8.75 | 3 |
| SJF, non-preemptive | 14.25 | 7.75 | 7.75 | 3 |
| SRTF | 13.00 | 6.50 | 4.25 | 4 |
| Priority, non-preemptive | 14.25 | 7.75 | 7.75 | 3 |
| RR, quantum 1 | 19.00 | 12.50 | 0.75 | 23 |
| RR, quantum 3 | 20.00 | 13.50 | 3.00 | 9 |
| RR, quantum 5 | 17.50 | 11.00 | 5.50 | 5 |
| RR, quantum 20 | 15.25 | 8.75 | 8.75 | 3 |

The completed handout program at `evidence/task02-round-robin-todo.txt` matches its expected Gantt chart and means (TAT 20.00; WT 13.50).

### Evaluation

FCFS is simple and has low scheduling overhead, but a long encoding job causes the convoy effect for 2-second thumbnail work. SJF minimizes average waiting for a known, fixed batch of nonpreemptive bursts, but long films can starve if short jobs continue to arrive. SRTF can reduce short-job response further by preempting a long job, at the cost of preemption and possible starvation. Priority has the same starvation risk for low priority jobs. Ageing periodically increases the effective priority of waiting jobs; a service can also enforce a maximum wait or reserve capacity for long jobs.

RR quantum 1 is responsive but causes many dispatch changes. Quantum 20 approaches FCFS for this workload and can make short jobs wait behind a long time slice. Intermediate quanta trade latency against dispatch overhead. The measured table compares all four quanta on this fixed sample; there is no universally optimal quantum because real context-switch costs and job length distributions matter.

Recommendation: use a multilevel feedback queue (MLFQ) or explicit size classes with short initial quanta, promotion for jobs that block/yield, demotion for jobs that consume full quanta, ageing, and reserved long-job capacity. This gives thumbnail-like jobs a fast path without permanently starving film jobs. A simple SJF queue has O(n) selection (or O(log n) with a heap) and needs burst estimates; a bucketed priority queue can offer O(1) selection with fixed levels, though maintenance and ageing still cost work. An MLFQ does not know true run time and adapts based on observed CPU use, but its fairness depends on promotion/ageing policy.

## Q3 - POSIX threads (6 marks)

`Task03/thread_pool.c` applies the same deterministic CPU workload to 64 camera frames using 1, 4, 8, 16 and 64 threads. In the recorded Ubuntu runner sample, 4 threads were fastest at 0.005044 s (the program-reported speed-up was 3.839x against its initial serial timing); 8, 16 and 64 threads measured 0.006388 s, 0.005681 s and 0.007416 s, respectively. A repeated one-thread timing was 0.019140 s and the 64-thread case reached 2.611x. The difference between initial and repeated serial runs shows timing noise, so treat the ranking as this-run evidence, not a stable guarantee. One thread per camera was slower than four workers. Output in `evidence/task03-thread-pool.csv` includes exact times and speed-ups. Each thread has its own registers, stack, scheduling state and thread-local storage. Threads in the process share code, heap, global data, open file descriptors and address space. Per-frame results use separate array slots; no shared counter is updated without protection.

User-level threads are scheduled by a runtime in user space: creation/switching may be cheap, but a blocking system call can block the whole process in a many-to-one mapping. Kernel-level threads are visible to the OS and can run on multiple cores; a one-to-one model is common on Linux, with higher kernel management cost. Many-to-many multiplexes user threads on a smaller or adjustable set of kernel threads, but adds runtime complexity. The familiar models are conceptual; implementation details vary by OS/runtime.

Expected speed-up stops growing when available CPU cores are saturated, thread creation/synchronization overhead and memory bandwidth become significant, or workload imbalance leaves workers idle. If a machine has C effective cores, ideal speed-up is at most min(thread count, C) before overhead. A race on an ordinary shared counter is undefined behavior in C and can lose increments; use a mutex, atomic operation or per-thread counters reduced after joining. Process-based frame workers improve isolation but require process creation and IPC/result collection and generally duplicate/private-map more memory; threads make result collection trivial through shared memory.

## Q4 - Inter-process communication (6 marks)

The TODO program `Task04/pipe_todo.c` reads one integer at a time until `read` returns 0, then prints the expected six-reading summary. The parent closes its write descriptor after the final value; EOF occurs only after every open copy of that write end is closed and buffered bytes have been consumed. If the parent keeps the write end open, the child waits indefinitely for either more bytes or EOF. The exact output is in `evidence/task04-pipe.txt`.

`Task04/ipc_benchmark.c` measures completion of a one-way transfer/scan of 1, 10 and 100 MiB. The pipe sends bytes through the kernel and the child drains them; shared memory maps a POSIX shared-memory object into both fork-related processes, the parent fills it, then a small pipe token synchronizes the child to scan it. Shared memory avoids copying the payload through a kernel pipe, but still incurs page faults/cache and memory-bandwidth costs, and requires explicit ownership/order synchronization. Pipe writes and reads copy data between user and kernel buffers and naturally provide blocking/backpressure and EOF semantics. Both methods have process synchronization; shared memory needs an additional protocol (semaphore, mutex/futex or event) to prevent reading partial/inconsistent data. The measured times and rates were: pipe 1/10/100 MiB in 0.000383/0.004161/0.037252 s; shared memory in 0.001020/0.008881/0.087409 s. Thus this run's pipe path completed faster at all three sizes. The shared-memory path includes object creation, mapping, first-touch zeroing and a child page scan, while the pipe path benefits from the runner's kernel buffering; the benchmark is one sample with unequal work details and cannot establish that pipes are generally faster. Results in `evidence/task04-ipc.csv` are machine-specific, include process/mapping setup and are not a universal per-byte cost model.

For the weather sensor's small readings, choose a pipe: simple lifecycle, reliable stream order, clear EOF, and it was faster in this measured sample. For repeated large image/data exchange, benchmark a persistent shared-memory ring buffer with pages already mapped and an explicit semaphore/sequence protocol; the present results do not show a shared-memory advantage. A message queue adds message boundaries and priority but normally copies messages and has queue-size limits.

## Q5 - Synchronization (11 marks)

The supplied critical-section requirements are mutual exclusion (one owner at a time), progress (if the section is free, non-contenders do not indefinitely delay the choice), and bounded waiting (a contender has a finite bound on overtakes). Peterson's two-process algorithm sets `flag[i]`, sets `turn=j`, and spins while the peer wants entry and has priority. Under sequentially consistent memory it provides mutual exclusion, progress and bounded waiting for two participants. Plain C loads/stores are data races, and modern compiler/CPU reordering can violate the assumed order; use specified atomic memory ordering or OS primitives instead. It does not scale naturally to many agents and wastes CPU while spinning.

A binary semaphore is a 0/1 permit; a counting semaphore tracks multiple identical resources. `wait` atomically decrements when positive or blocks, and `signal` increments/wakes. For the seat inventory, guard the check-and-decrement transaction with a mutex or semaphore so availability test and booking are indivisible. In bounded buffer, `empty` starts at capacity and `full` at zero; producer waits on `empty`, locks to insert/update index, unlocks, then signals `full`. Consumer does the symmetric sequence. The monitor equivalent encapsulates buffer state and uses `notFull`/`notEmpty` condition variables in `while` loops around predicates; only one monitor operation runs at a time, making invariants clearer. Readers-writers uses a reader count protected by a mutex and a writer lock, with an explicit fairness policy to avoid starving writers or readers. Dining philosophers should acquire resources in a global order, use a waiter/semaphore admitting at most N-1 philosophers, or model forks as monitor conditions; breaking circular wait prevents deadlock.

`Task05/bounded_buffer.c` verifies every one of 10,000 uniquely numbered items is consumed exactly once in 1P/1C and 4P/4C cases and repeats 4P/4C 100 times; all 102 tested runs reported zero duplicates/missing items. It benchmarks a contended global mutex, four per-train locks and 32 per-coach locks on 4 worker threads. Recorded throughput was 22.0, 31.6 and 60.9 million protected updates/s, respectively (one short synthetic run). `Task05/bounded_buffer_todo.c` completes the given 20-item code; its single consumer reports the expected sum 210. Outputs are in `evidence/task05-buffer.txt` and `evidence/task05-todo.txt`.

`Task05/sync_primitives_bench.c` measures 500,000 protected/increment updates per thread. Nanoseconds per update for 1/4/16 threads were: mutex 11.90/36.05/35.98; C11 atomic fetch-add 4.98/7.90/9.26; spin lock 5.65/65.34/202.87. The spin lock's 16-thread run took 1.623 s of wall time and 6.433 s aggregate process CPU time (about four CPU-seconds per wall-second) while making less progress than the mutex. It demonstrates the CPU cost of busy waiting under contention. These measurements include thread creation/join and depend on runner scheduling. The single-consumer handout program does not need atomic sum updates; multi-consumer code uses an atomic sum. Removing the buffer mutex in a multi-producer/multi-consumer version leaves shared indices and slots racy; counting semaphores alone do not provide mutual exclusion, and C data races have undefined behavior. The measured lock benchmark is a synthetic counter workload: real lock granularity should follow invariants. A global lock is easiest to prove correct but serializes independent trains. Per-train locks allow unrelated trains to book concurrently and are a sound default if seats are independent within each train. Per-coach locks increased measured throughput in the synthetic test, but cross-coach transactions (adjacent-seat booking, coach reassignment, inventory totals) need ordered multi-lock acquisition or a separate transaction lock; otherwise races or deadlocks are possible. Choose coach-level locking only after defining those invariants. Atomic fetch-add is suitable for a single counter, not for a multi-step “check seat then claim it” invariant. A spin lock can be efficient for a tiny, nonblocking critical section under low contention; under oversubscription or long I/O-bound sections it wastes CPU. Blocking mutex/semaphore is generally preferable for a long critical section.

## Q6 - Deadlock and Banker’s algorithm (13 marks)

### A. Four necessary conditions (2 marks)

Deadlock requires all four Coffman conditions: mutual exclusion (some resource is exclusive), hold-and-wait (a job holds one resource while requesting another), no preemption (resources are released voluntarily), and circular wait (a cycle of jobs/resources). For the lab: processes/jobs are circles and resource instances are squares; `job -> resource` denotes a request and `resource -> job` an allocation.

```text
[GPU] --> J0 --> [Licence] --> J1 --> [Storage] --> J2 --> [GPU]
  ^         holds       request     holds       request     holds     request
```

This illustrative cycle is a single-instance cycle: J0 holds a GPU and requests a licence; J1 holds the licence and requests storage; J2 holds storage and requests the GPU (cycle J0 -> Licence -> J1 -> Storage -> J2 -> GPU -> J0). The cycle demonstrates circular wait; mutual exclusion, hold-and-wait and no-preemption are also present. With multiple instances, a graph cycle is necessary but may not alone prove deadlock.

### B. Compare strategies (3 marks)

| Strategy | Mechanism | Cluster trade-off |
|---|---|---|
| Prevention | Structurally break a Coffman condition, e.g. impose global resource order or reserve all required resources before start | Strong guarantee and little runtime checking; can reduce utilization and cause jobs to reserve scarce GPUs/licences while idle. Global ordering is often practical for heterogeneous resources. |
| Avoidance | Banker grants only if the tentative state remains safe | Avoids deadlock while preserving some concurrency; requires maximum demand known and credible, plus per-request O(m n^2) worst-case safety work for n jobs and m resource types. Dynamic/interactive jobs make claims difficult. |
| Detection and recovery | Allow allocation; periodically detect cycles/unsafe allocations and recover by abort, rollback or preemption | Fits uncertain dynamic demand and avoids checking every request; detection costs CPU, and aborting long simulations loses work. Check interval trades detection latency against overhead. |
| Ignore (ostrich) | Accept rare deadlocks and manually restart | Low overhead, acceptable only when deadlocks are rare and recovery is cheap. Unacceptable as the sole plan for critical shared cluster service without monitoring and recovery. |

### C. Banker implementation and requests (4 marks)

`Task06/banker.c` implements `Need = Max - Allocation`, scans for a process whose need is no greater than `Work`, then releases its allocation into `Work`; the tentative request is applied only if the resulting state is safe, otherwise rolled back. For the given state it reports safe order P1, P3, P4, P0, P2. P1's `(1,0,2)` request is granted with that safe order. P4's `(3,3,0)` must wait because the request exceeds currently available resources. P0's `(0,2,0)` is within availability/need but is denied: after P1's earlier grant and P0's tentative allocation, `Available=(2,1,0)`. No unfinished process can satisfy its remaining need, so no complete safe sequence exists. The exact output is `evidence/task06-banker.csv`.

### D/E. Complexity prediction and measurement (2 marks)

For each of up to n completion rounds, the safety scan examines n processes and compares up to m resource types: worst case O(m n^2); memory O(mn) for allocation/need plus O(n) work/finish/sequence. If many processes complete in one pass, actual runtime can be closer to O(mn). The included benchmark creates reverse-ordered safe states that expose repeated scans, uses a fixed three-resource-shaped model with one active resource, and averages repeated runs for n=10, 100 and 1,000. Measured per-scan means were 0.000000049 s, 0.000004431 s and 0.000379954 s. Increasing n by 10x increased observed time by about 90x and then 86x, near the O(n^2) trend but below 100x because of fixed overhead and cache/timer effects. See `evidence/task06-banker.csv`. These are measurements, not a proof of complexity; the safety-check code and benchmark scenario are provided for reproduction.

### F. Final design (2 marks)

For a research cluster with known, bounded job classes, use admission control and reservation/scheduling for exclusive GPUs and licences, plus Banker-style avoidance for the small set of jobs that can accurately declare maximum resource claims. For long, dynamic simulations whose future demand is unknowable, strict Banker avoidance can reject useful work; use a global resource acquisition order where possible and detection with checkpoint/rollback for remaining resources. Monitor wait-for graphs and set recovery policies before deployment. This hybrid limits deadlock risk without assuming every job can predict its maximum demand. The cost is lower utilization under conservative reservations, metadata/checking overhead, and operational cost for checkpointing and recovery.

<!-- pagebreak -->

## Appendix - Reproduction and evidence index

| Question | Program / output |
|---|---|
| Q1 | `Task01/process_demo.c`; `evidence/task01-processes.txt` |
| Q2 | `Task02/scheduling.c`, `Task02/round_robin_todo.c`; `evidence/task02-*` |
| Q3 | `Task03/thread_pool.c`; `evidence/task03-thread-pool.csv` |
| Q4 | `Task04/pipe_todo.c`, `Task04/ipc_benchmark.c`; `evidence/task04-*` |
| Q5 | `Task05/bounded_buffer_todo.c`, `Task05/bounded_buffer.c`, `Task05/sync_primitives_bench.c`; `evidence/task05-*` |
| Q6 | `Task06/banker_todo.c`, `Task06/banker.c`; `evidence/task06-banker*` |

Run all with `bash scripts/run_linux_evidence.sh` from `Assignment2/`. The workflow uploads the raw logs as an Actions artifact; the same captured files are committed in `evidence/`. The process snapshot is intentionally transient, and all performance measurements depend on runner load/hardware. References: Silberschatz, Galvin and Gagne, *Operating System Concepts*, 10th ed.; Linux `fork(2)`, `pipe(2)`, `shm_open(3)`, `pthread(7)` and `proc(5)` manual pages.

## OnlineGDB output screenshot

This capture shows `Task02/scheduling.c` compiled and run in OnlineGDB. It displays the FCFS Gantt output and the first completion-time rows; the complete output for every scheduling policy remains in `evidence/task02-scheduling.txt`.

![OnlineGDB run of Assignment 2 CPU scheduling simulator](evidence/onlinegdb/task02-scheduling.jpg)
## Full OnlineGDB program runs

The complete C sources were pasted into OnlineGDB and executed. Longer terminal output is split into beginning and ending views; the committed raw output files remain the complete transcripts.

### Q1 — process creation and state

The full `Task01/process_demo.c` source is retained in the repository. It created three short-lived children, captured a `ps` process snapshot, inspected each child through `/proc`, and reaped all children with exit statuses. The expanded OnlineGDB terminal screenshots show the process table, per-child status, and final wait results; the terminal output is split across views to keep the text readable.

![Expanded OnlineGDB process run and complete terminal output](evidence/onlinegdb/process-demo-full.jpg)

![Process snapshot output, beginning view](evidence/onlinegdb/process_demo-output-first.jpg)

### Q2 — scheduling algorithms

The full `Task02/scheduling.c` source compares FCFS, non-preemptive SJF, SRTF, non-preemptive priority and Round Robin at four quanta. The expanded OnlineGDB views show per-process completion, turnaround, waiting and response values, Gantt sequences, average metrics, context switches and the successful exit status. The raw transcript remains available in `evidence/task02-scheduling.txt`.

![Expanded OnlineGDB scheduling run and final output](evidence/onlinegdb/scheduling-full.jpg)

![Scheduling policy output, beginning view](evidence/onlinegdb/scheduling-output-start.jpg)

### Q3 — parallel thread pool

![Thread pool source and benchmark output](evidence/onlinegdb/thread_pool-full.jpg)

### Q4 — IPC benchmark

OnlineGDB reported the pipe/shared-memory measurements before its sandbox terminated the 100 MiB POSIX shared-memory case with exit code 135 (SIGBUS). The complete 1/10/100 MiB benchmark succeeded on Ubuntu and is recorded in `evidence/task04-ipc.csv`; use that CSV as the completed benchmark result. The OnlineGDB screenshot documents the sandbox limitation.

![OnlineGDB IPC run and sandbox termination](evidence/onlinegdb/ipc_benchmark-full.jpg)

### Q5 — bounded buffer and synchronization

The bounded-buffer program verified all 10,000 items exactly once in the 1-producer/1-consumer and 4-producer/4-consumer cases, then ran 100 repeated stress runs and lock-scope measurements. The second run compares mutex, spin-lock and atomic counter approaches.

![Bounded buffer correctness, stress and performance output](evidence/onlinegdb/bounded_buffer-full.jpg)

![Synchronization primitive benchmark output](evidence/onlinegdb/sync_primitives_bench-full.jpg)

### Q6 — Banker safety algorithm

![Banker safe/unsafe state and request results](evidence/onlinegdb/banker-full.jpg)
