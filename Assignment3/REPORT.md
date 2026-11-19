# CSA0406 Operating Systems - Assignment 3 (CO3)

## Cover

**Assignment:** Evaluate and Justify Memory-Management Designs  
**Student:** Mohamed Aslam H  
**Register number:** 192572147  
**Department:** CSE (AI)  
**Academic year:** 2025  
**Faculty:** Dr. Jayasakthi Velmurugan K  
**Nature:** Individual  
**Repository:** https://github.com/Md-Aslam-31/CSA0406-Operating_System/tree/main/Assignment3  
**Execution environment:** Ubuntu 24.04.5 LTS, GCC 13.3.0. The complete run succeeded in [GitHub Actions run #44586278245](https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/44586278245). Raw output and exact compile commands are in `evidence/`.

## Executive summary

For the bank, statically include rarely changing core code but deploy the encryption component as a versioned shared library; use `dlopen` when the feature is optional and handle version/load failures. For the college server, use 4 KiB base pages with huge pages selectively for large stable regions. Use paging with protection for normal Linux applications; segmentation remains a useful logical protection model but causes external fragmentation when implemented with variable-size physical regions. Demand paging can make a 24 GiB virtual workload addressable on a 16 GiB laptop only when its active working set fits available RAM plus backing storage; improve locality and add RAM if sustained access exceeds that capacity. For a web server, Clock is a practical approximation to LRU, with LRU/Optimal fault counts used as comparison bounds. For critical records use RAID 10 or RAID 6 with tested backups and a separate monitored swap policy; RAID is not a backup. On the factory edge device, reserve and prefault a bounded control working set, lock it in memory, isolate CPU and validate deadlines on the target. Keep paging and reclaim away from the real-time path.

## Q1 - Linking, loading and address binding (6 marks)

### Linking experiment

`Task01/crypto.c` is built as a static archive and a position-independent shared object. It includes a 64 KiB read-only demonstration table so linked size differences are visible. `linked_demo.c` is linked against each; the custom library implementation is identical apart from its version string. The executable statically includes the custom archive in one build, while libc remains a normal system shared library. The other executable depends on `libbankcrypto.so`. `dlopen_demo.c` calls `dlopen(RTLD_NOW)` and resolves the API with `dlsym`. The measured files were 81 KiB for the static-library executable, 17 KiB for the dynamic executable and 79 KiB for the shared library. Batches of 100 launches took 0.09 s static and 0.10 s dynamic, with maximum RSS 3,448 and 3,456 KiB. These small, cached-runner measurements are too close to timing noise to establish a general startup penalty. Details are in `evidence/task01-linking.txt`.

Replacing the `.so` with version 2 changes the dynamic executable's reported library version without relinking; the statically included archive continues to report version 1. Both live processes showed shared clean pages for the same object, including a shared 64 KiB table mapping. When the object is missing, the startup-linked executable exits 127 before `main`; the `dlopen` program exits 2 after printing its error. A production optional plugin can instead continue in reduced mode, with explicit symbol/version checks. This defers its mapping/startup work and avoids resident pages when unused, at the cost of recovery logic.

### Address binding and evaluation

- **Compile-time binding:** absolute addresses are fixed by the compiler; code must be loaded at that address or relocated/recompiled.
- **Load-time binding:** the linker emits relocatable code and the loader assigns final addresses when the executable is loaded.
- **Execution-time binding:** hardware address translation permits a process to move during execution; the MMU maps its virtual addresses to physical frames.

The **linker** resolves symbols, combines object files/libraries, applies relocations and emits executable metadata. The **loader** maps executable and dependency segments, performs remaining relocations, establishes the initial stack/arguments and transfers control to the entry point. Position-independent code uses relative addressing/GOT/PLT mechanisms so a shared object can be mapped at different virtual addresses and shared by processes without rewriting its text for each load address.

Static linking increases each executable's size and duplicates library code on disk; clean physical pages can still be shared by the kernel cache, but separately linked versions are harder to patch centrally. Dynamic linking reduces duplicate executable contents and permits a library update without relinking clients, but adds dependency resolution, ABI/version management and possible startup failure. Startup and memory results are measured in the retained evidence; one-run timing/RSS is affected by filesystem and page cache. Two process mappings of the same read-only `.so` can refer to the same file-backed clean physical pages; `/proc/<pid>/smaps` reports `Shared_Clean` when those pages are resident. Writable data is private/copy-on-write. For a security library used on every request, dynamic linking simplifies urgent updates; for optional cryptography, `dlopen` avoids its mapping until needed and must fail closed if unavailable.

## Q2 - Paging (8 marks)

### Fragmentation and translation

```text
Contiguous physical memory, variable-sized allocations
+------+-------+----+--------+----+---------+
| OS   | P1    |gap | P2     |gap | P3      |
+------+-------+----+--------+----+---------+
           free total may exceed a request,
           but no individual gap is large enough
```

Paging maps a logical address `(page, offset)` through a page table to `(frame, offset)`. For the provided table with 1 KiB pages, logical 1,500 gives page 1, offset 476; frame 2 yields physical 2,524. Logical 9,000 gives page 8, outside the eight-entry table; 2,049 gives page 2, which is absent and therefore faults. The completed code output is in `evidence/task02-todo-1024.txt`; the program is also run with 2 KiB pages in `evidence/task02-todo-2048.txt`.

For 32-bit virtual addresses and 4 KiB pages, there are 2^(32-12)=2^20 entries. At 4 bytes per PTE a flat table is 4 MiB per process. For a 48-bit virtual space there are 2^36 entries; at 8 bytes/PTE a flat table is 512 GiB, which motivates hierarchical tables and the fact that real user processes occupy only part of that address range. These totals assume every virtual page has an entry; multi-level allocation commits lower-level tables only where needed. A hashed table hashes virtual page/process identity; an inverted table stores one entry per physical frame, reducing table space to O(physical frames) but requiring a hash/search and process-ID/address-space match.

With memory access M=100 ns and TLB lookup T=20 ns, assuming a TLB hit needs one memory access and a miss needs two, `EAT=h(T+M)+(1-h)(T+2M)`. Hits 80%, 95%, and 99% give 140 ns, 125 ns, and 121 ns. Paging eliminates external fragmentation because any free frame can hold any page. It leaves internal fragmentation in the last partially used page; expected waste for uniformly distributed endings is about half a page.

### Page-size experiment and conclusion

`paging_lab.c` uses a seeded set of 100 process sizes and calculates exact end-page waste for 1 KiB, 4 KiB and 2 MiB pages, reports TLB EAT and the Linux base-page size. Measured internal waste across 100 deterministic process sizes was 57,258 bytes at 1 KiB pages, 221,754 bytes at 4 KiB, and 107,796,554 bytes at 2 MiB. The runner base page is 4,096 bytes; `/usr/bin/time -v` recorded 76 minor faults and zero major faults for the paging experiment. A larger page reduces PTE/TLB pressure and can improve sequential large-image access, but increases internal waste and makes fine-grained protection/sharing less precise. A smaller page reduces average tail waste but increases page-table and TLB pressure. Choose 4 KiB as the general server base, with 2 MiB huge pages for measured large, aligned, long-lived buffers; do not choose 2 MiB universally.

Read-only code and shared libraries can map the same physical frames into multiple page tables; PTE permission bits enforce read/write/execute protection. Copy-on-write initially maps pages read-only and creates a private frame on write. Address tests include the first/last byte of a page and the first address beyond the page-table range.

## Q3 - Segmentation (6 marks)

An address is `(segment, offset)`. The segment table gives base, limit and permissions; if `offset >= limit`, hardware raises a protection fault, otherwise linear address is `base+offset`. `Task03/segment_lab.c` checks eight hand-selected addresses, including exact-limit and one-past-limit cases, shows this process's `/proc/self/maps`, and triggers a `PROT_NONE` access in a child; the kernel delivers SIGSEGV and terminates that child. It also loads 20 seeded variable-size segments into a 64 MiB arena, removes alternating segments and reports total free memory, largest hole and external fragmentation in `evidence/task03-segmentation.txt`. The seeded run allocated 14,102,974 bytes and freed 7,558,222 bytes; total free space was 60,564,112 bytes, while the largest hole was 53,005,890 bytes, demonstrating external fragmentation.

```text
logical (segment, offset)
        | check offset < segment limit; add segment base
        v
linear address -> page number + page offset
        | page table maps page to frame
        v
physical address = frame * page_size + page offset
```

Segments match the programmer's view: code, data, heap and stack can have distinct sizes, growth policies and permissions. Sharing a code segment is natural, and bounds checking can prevent one segment from accessing another. Pure segmentation allocates variable-size physical regions, so freeing/loading segments creates external holes; compaction is expensive. Pure paging avoids external fragmentation and supports uniform protection/sharing but does not represent logical objects directly and can waste part of the last page. Segmentation with paging retains a logical segment table whose entries point to page tables, then translates each page through a frame. Linux x86-64 uses a flat segmentation model for ordinary user code (with special bases such as FS/GS), and relies primarily on paging for isolation. Page tables are O(virtual pages) or sparse multi-level; a segment table is O(number of segments); segmented paging adds per-segment page-table overhead. Modern general-purpose systems favor paging because it works well with virtual memory, allocation, sharing and hardware protection.

## Q4 - Virtual memory and demand paging (8 marks)

### Fault handling and effective time

On a not-present page reference, the CPU traps to the kernel. The kernel validates the address and access permissions, finds/allocates a frame, reads file/backing-store contents or zero-fills it, updates the page table/TLB, then restarts the faulting instruction. Invalid access signals the process. Memory 100 ns and fault service 8 ms produce `EAT=(1-p)*100+p*(8,000,000+100)` ns:

| Fault rate p | EAT | Approx. slow-down |
|---:|---:|---:|
| 1/1,000 | 8,100 ns | 81x |
| 1/100,000 | 180 ns | 1.8x |
| 1/1,000,000 | 108 ns | 1.08x |

For no more than 10% slowdown, EAT <= 110 ns, so `p <= 10/7,999,900 ~= 1.25e-6` (about one fault per 800,000 references). Even a very low fault rate can matter because an 8 ms service is about 80,000 times a 100 ns memory reference.

### Demand paging and measurements

A 24 GiB process can be addressable on a 16 GiB machine because only resident working pages occupy RAM and inactive/file-backed pages can be reclaimed or paged to backing storage. It will thrash if its actively reused working sets exceed available frames: each eviction causes another fault and useful CPU work collapses. The working-set model estimates pages referenced over a recent window; if the sum of working sets exceeds frames, reduce multiprogramming/admission or add RAM. Equal frame allocation is simple but ignores process size; proportional allocation shares frames by resident demand, with priority/fairness constraints. OS working-set or page-fault-frequency controls detect thrashing and can suspend/reduce processes.

`virtual_memory.c` touches a 64 MiB region sequentially and in seeded random page order and records minor/major faults with `getrusage`; `/usr/bin/time -v` captures page fault totals. It reports `/proc/self/smaps_rollup` before/after fork and after child writes to show copy-on-write private pages. Child processes receive a 512 MiB address-space limit; runs touch 64, 128, 256 and 384 MiB and a 768 MiB allocation is expected to be rejected, demonstrating time/fault changes and a safe limit instead of exhausting runner RAM. This runner experiment does not reproduce a 24 GiB workload on a 16 GiB laptop; it validates the fault/COW/limit mechanics. On the laptop, reduce memory traffic and preserve locality first, then add RAM if the working set remains larger than available memory. A 32 GiB machine can keep a 24 GiB resident with headroom and avoid sustained paging; extra RAM costs hardware expense but avoids long fault latency. Demand paging is not a substitute for capacity when all 24 GiB is hot.

The fork experiment uses copy-on-write: parent/child initially share read-only frames, and writing a page allocates/copies a private frame. On this runner, sequential and random 64 MiB touches each took about 0.03 s and raised roughly 16,000 minor faults with no major faults. The COW child grew from 44 KiB to 65,580 KiB private dirty after writes while the parent stayed near 65,660 KiB. The 512 MiB address-space run accepted touches through 384 MiB and rejected a 768 MiB request. See `evidence/task04-memory.txt` for mappings and exact timings. The replacement-policy fault graph is embedded below and generated from the same replacement simulation data.

![Working-set page faults versus frames](plots/page_faults.png)

## Q5 - Page-replacement techniques (8 marks)

The provided TODO implementation in `Task05/lru_todo.c` completes LRU by choosing the resident frame with the smallest last-use index. Verified results for the 20-reference handout are: 3 frames FIFO 15, LRU 12, Optimal 9; 4 frames FIFO 10, LRU 8, Optimal 8. For Belady's string `{1,2,3,4,1,2,5,1,2,3,4,5}`, FIFO faults rise from 9 with 3 frames to 10 with 4. FIFO forgets access recency, so adding a frame changes queue replacement order and can evict a page needed soon. LRU is a stack algorithm: pages resident with k frames form a subset of those resident with k+1 frames, so it cannot suffer this anomaly. Optimal replaces the page whose next use is farthest in the future; it is a lower bound, but future references are unknowable online.

`replacement_lab.c` builds deterministic 100,000-reference strings: strong locality (repeated eight-page windows, shifting every 1,000 references) and random access over 4,096 pages. It compares faults and elapsed time for FIFO, exact LRU, Optimal and Clock at 3-10 frames; `evidence/task05-replacement.csv` is used for the plotted data. With 8 frames, all policies produce 800 faults on the locality trace; with 4 frames they incur 100,000 faults because the eight-page working set does not fit. For random access with 4 frames, FIFO/LRU/Clock each incur 100,000 faults and Optimal incurs 99,928. LRU needs hardware/software recency ordering or timestamps updated on each hit; a timestamp implementation here costs O(frames) victim selection and O(1) hit update. The optimized Optimal simulator precomputes each reference's next use in O(n+page-domain) and chooses the farthest next use in O(frames) per miss; it is a benchmark bound, not an online policy. Clock keeps a reference bit per frame and advances a hand, clearing bits until a zero-bit victim is found; it approximates recency with small per-frame space and amortized scans instead of maintaining exact order. Strong locality should favor LRU/Clock; random access gives little recent-use advantage. For a production web server, recommend Clock or a segmented/working-set approximation: cheaper state and scans than exact LRU, and benchmarked against LRU/Optimal for the actual cache workload. Check `evidence/task05-replacement.csv` for counts and runtimes.

![Replacement-policy fault counts](plots/replacement_faults.png)

## Q6 - Disk management (4 marks)

Low-level formatting creates physical sectors (modern drives perform this internally); partitioning defines address ranges; logical formatting creates a filesystem's metadata and free-space structures. The boot block/EFI system partition holds bootloader code; firmware initializes hardware and selects it, the bootloader loads the kernel/initramfs, and the kernel discovers storage, mounts root and starts user space. Bad blocks are remapped by drive firmware where possible; filesystems/device layers can also mark unreadable blocks and relocate data from backups. Monitor SMART/health signals and replace failing media.

With six 4 TB disks, nominal usable capacities are: RAID 0 = 24 TB; RAID 1 as three mirrored pairs = 12 TB; RAID 5 = 20 TB; RAID 6 = 16 TB; RAID 10 as three mirrored pairs = 12 TB. RAID 0 offers aggregate throughput but no failure tolerance. RAID 1/10 provide mirror reads and tolerate one failure per mirror pair; writes go to both copies. RAID 5 tolerates one drive loss and has parity-write overhead; RAID 6 tolerates two and has additional parity cost. Capacity excludes filesystem/metadata and decimal-vs-binary presentation effects.

For a transparent independent annual disk-failure probability p=0.02 assumption, RAID 1/10 with three independent pairs loses data if both disks fail in any pair: `1-(1-p^2)^3`, or 0.12%. RAID 5 loses data if two or more of six fail: `1-(1-p)^6-6p(1-p)^5`, or 0.5687%. RAID 6 requires three or more failures, about 0.015291%. RAID 0 loses data if any disk fails, about 11.4158%. These simplified probabilities omit correlated failures, rebuild exposure, latent sector errors and controller/site failure; real risk is higher. The computed values are in `evidence/task06-raid.txt`.

Choose RAID 10 for this records server, assuming the nominal 12 TB usable capacity is sufficient: it gives predictable writes and straightforward mirror rebuilds, while tolerating one disk failure in each mirror pair. Keep independent offline/immutable backups and test restore; RAID is availability, not backup. For a concrete swap policy, assuming 64 GiB RAM and no hibernation, start with a 16 GiB encrypted swap file on the protected RAID 10 volume, reserve RAM for the database cache, and alert/throttle before sustained swap-in. Confirm capacity against measured peak and crash-dump policy before deployment; do not put swap on a single unmirrored disk or use it to mask a sustained RAM shortfall. If the data set cannot fit in 12 TB, choose RAID 6 and accept higher write/parity overhead.

Linux device/partition/swap data and a read-throughput sample of the runner's filesystem are in `evidence/task06-disk.txt`. This hosted runner exposes only its own virtualized storage path, not a matched SSD and HDD, so a true cross-device speed comparison was unavailable and is not fabricated. The file read result is labeled by filesystem/device evidence and should not be presented as an SSD-vs-HDD benchmark. In the 6x4 TB RAID 6 alternative, usable capacity is 16 TB, two failures are tolerated, and each write computes two parity values (higher CPU and I/O work than single parity); parity implementations use XOR plus a second independent coding operation such as Reed-Solomon.

## Q7 - First-principles edge-device memory plan (10 marks)

### Stage 1: measurable requirements

| Component | Budget / requirement | Page-fault tolerance |
|---|---|---|
| Real-time control | 128 MiB reserved and prefaulted; fixed loop must meet its deadline with zero demand faults | None during control cycle |
| ML inference | Up to 1,536 MiB model/working data; bounded latency target measured per model | Can tolerate startup/model-load faults; avoid faults in the inference window |
| Logging | 128 MiB bounded queue/cache; drop or rotate low-priority logs under pressure | Tolerates buffered storage delays |
| Kernel/services | 256 MiB planning allowance | Ordinary OS reclaim, outside the RT critical path |
| **Total** | **2,048 MiB; no swap** | Admission control required |

The control requirement is deadline-based: no page faults in the cyclic control path and a target-specific maximum loop delay (for example, 1 ms; the actual plant's safety analysis must set it). The ML task can be throttled or reduced; logging can shed data. Kernel memory is not a hard reservation in Linux, so deployment needs cgroups/reservations and validation under worst-case load.

### Stage 2: candidate mechanisms

| Mechanism | Real-time control | ML inference | Logging / future tasks |
|---|---|---|---|
| Contiguous allocation | Predictable placement but difficult to grow and external fragmentation | Large model can fail despite total free memory | Poor for many variable future tasks |
| Paging | Use fixed resident frames, prefault and `mlock`; enforce permissions | Flexible mapping, shared model pages, demand-load before run | Best general allocation and isolation |
| Segmentation | Natural code/data/stack protection, but variable physical segments fragment | Logical regions help organization | Growth and placement are harder |
| Demand paging | Not allowed in deadline path | Useful during model startup and cold data | Appropriate for noncritical logs/tools |

Lock the control working set with `mlock()` (or `mlockall(MCL_CURRENT|MCL_FUTURE)` after setting `RLIMIT_MEMLOCK`), prefault every page, disable unexpected allocation in the loop, isolate a CPU, use bounded priority scheduling and validate worst-case interrupt/driver behavior. The demonstration locks and prefaults a 16 KiB test hot set; the plan reserves 128 MiB for a real control image and would require sufficient process/system memlock limits.

### Stage 3: verification and evidence

`Task07/memory_plan.c` measures 5,000 hot-set iterations with and without `mlock`, reporting median, 99th-percentile and maximum observed loop duration. It simulates control, ML and logging page references with and without a pinned control working set. Here both modes measured p50 30 ns and p99 41 ns; the locked run had a 22,402 ns maximum outlier. The simulation reports four post-warmup control misses without pinning and zero with pinning. The runner permits only an 8 MiB memlock limit, so the demo locks a 16 KiB hot set rather than the planned 128 MiB reserve. This demonstrates the mechanism, not a hard real-time guarantee: a shared Linux kernel, interrupts and drivers can still create latency, and a finite observed maximum is not a formal worst-case bound. Run the test under representative thermal, I/O and CPU load on the actual device.

### Stage 4: defence and alternative

The plan assigns the fixed real-time hot set first, caps the ML working set to the remainder and makes logging the first reclaim/shedding target. A fourth 500 MiB task exceeds the 2 GiB budget by 500 MiB; do not reclaim control pages. Reduce/quantize the ML model, offload the fourth task, or add RAM and revalidate timing. One remaining limitation is that Linux cannot promise a strict no-delay bound solely from `mlock`; the device requires a real-time kernel/configuration and hardware-level deadline testing if missing a control deadline is unsafe.

On a small RTOS with no virtual memory, preallocate and statically partition the control buffers so paging delay is impossible and worst-case behavior is easier to bound. That leaves less flexible memory for the 1.5 GiB ML workload, and adding tasks requires redesigning fixed pools. Linux with locked control memory offers better dynamic sharing and service growth but needs careful isolation and cannot by itself give the same hard upper bound. The recorded locked/unlocked timings and frame-simulation result are in `evidence/task07-memory-plan.txt`.

## Evidence and reproduction

| Question | Source and retained output |
|---|---|
| Q1 | `Task01/`; `evidence/task01-linking.txt` |
| Q2 | `Task02/`; `evidence/task02-*` |
| Q3 | `Task03/`; `evidence/task03-segmentation.txt` |
| Q4 | `Task04/`; `evidence/task04-*` |
| Q5 | `Task05/`; `evidence/task05-*`; charts in `plots/` |
| Q6 | `Task06/`; `evidence/task06-*` |
| Q7 | `Task07/`; `evidence/task07-memory-plan.txt` |

Rebuild with `bash scripts/run_linux_evidence.sh` from `Assignment3/`. Measurements are from the Ubuntu CI runner and are system-specific; the disk and real-time limitations above are explicit. Reference concepts: Silberschatz, Galvin and Gagne, *Operating System Concepts*, 10th ed.; Linux `ld.so(8)`, `dlopen(3)`, `mmap(2)`, `mlock(2)`, `getrusage(2)`, `proc(5)` and `proc_pid_smaps(5)` manual pages.
