# Assignment 1 — Task 08

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 8 — CPU states and protection

User mode restricts privileged instructions; kernel mode may control devices, page tables and interrupt state. A mode bit/state distinguishes privilege. Hardware interrupts signal external events; traps/exceptions arise synchronously from instructions (including syscalls); timer interrupts return control from a running process so the scheduler can enforce time slices/priorities. If arbitrary monitor code could disable interrupts, it could suppress sensor handling and alarm deadlines indefinitely, so only trusted kernel code can perform that operation. MMU page tables and privilege checks prevent one process from reading another’s memory; DMA requires IOMMU/device isolation as well.

Polling alternative: with polling interval P, worst-case detection wait is approximately P plus processing latency; interrupt-driven detection is bounded by interrupt masking/dispatch latency plus handler time. Polling every 1 ms may meet a 10-ms target but consumes CPU and energy even when readings do not change; a slower interval saves energy but raises worst-case detection delay. An interrupt path can lower idle cost, but must be measured and protected from interrupt storms. The monitor design uses interrupts, bounded high-priority handling and a watchdog, while reserving polling for diagnostics or where sensor hardware requires it.

See the [complete report](../REPORT.md) for the full submission and references.
