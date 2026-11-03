# Assignment 1 — Task 03

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 3 — OS structures

| Structure | Reliability / performance / update assessment |
|---|---|
| Monolithic | Fast direct calls; large trusted kernel and faulty in-kernel driver can crash or corrupt the whole system. |
| Layered | Clear interfaces ease reasoning and testing; extra layer crossings can add overhead and rigid boundaries can constrain design. |
| Microkernel | Small privileged core, isolated services and message passing improve fault containment; IPC/context switches add latency and require careful worst-case analysis. |
| Modular | Loadable modules combine fast kernel calls with extensibility; modules still run privileged and can destabilize the kernel. |
| Hybrid | Combines kernel and service ideas to balance compatibility and performance; complexity can weaken the benefits of separation. |

Linux is a monolithic kernel with loadable modules; Windows NT is hybrid; Android uses the Linux kernel with Android user-space framework/services; QNX is a microkernel RTOS, while FreeRTOS is a small real-time kernel rather than a full POSIX desktop OS [6][7]. Verify the exact target version/configuration before making safety or certification claims.

**Choices:** For a monitor, use a microkernel or small RTOS design with critical sensor/alarm paths isolated from optional services, provided measured worst-case IPC and interrupt latency fit the 10-ms budget. For the database, use a mature modular kernel (e.g., Linux) for drivers, observability and storage/network performance, backed by a tested HA cluster; kernel modularity does not itself deliver HA.

**Monolithic monitor alternative:** A sensor service request through a syscall is typically one trap and kernel dispatch; a microkernel design may add IPC and a context switch or two. Those fixed costs are hardware and configuration dependent and must be measured against the deadline; they are not safely represented by generic nanosecond claims. A monolithic kernel can reduce memory overhead and call overhead, but a faulty driver has a broader failure domain. QNX documents user-space isolation of drivers and services as a core microkernel property [8]. Microkernel memory overhead includes servers and message buffers; isolation can justify that cost for patient safety.

See the [complete report](../REPORT.md) for the full submission and references.
