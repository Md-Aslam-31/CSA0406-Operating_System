# Assignment 1 — Task 01

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 1 — Requirements by environment

Ranks: 1 is the most important constraint. Reliability includes safe failure behavior; security is mandatory throughout, even where it is not ranked first.

| Environment | Ranked needs | Reason and OS-history lesson |
|---|---|---|
| Bedside monitors | 1 bounded response (<10 ms), 2 availability/fail-safe, 3 data integrity, 4 security, 5 low memory/power, 6 throughput | A missed alarm is a patient-safety event. A hard real-time scheduler must bound interrupt, dispatch and sensor-to-alarm latency under worst-case load, not merely show a good average. Batch systems maximize jobs but are unsuitable for immediate response. A small embedded RTOS avoids unrelated services. |
| Handheld tablets | 1 battery, 2 security/privacy, 3 availability, 4 responsive UI, 5 wireless communication, 6 throughput | Mobile OS lessons include aggressive sleep, radio/power management, application sandboxing and managed updates. Battery savings must not defer a critical notification without an explicit clinical workflow. |
| Doctor/nurse workstations | 1 confidentiality/integrity, 2 availability, 3 interactive response, 4 compatibility, 5 throughput | Time-sharing and personal-computer systems make interactive multi-user applications practical. Central identity, encryption, endpoint management and least privilege matter more than deterministic sub-10-ms response. |
| Patient-record database | 1 integrity/durability, 2 availability, 3 access control/audit, 4 transactional throughput, 5 predictable latency, 6 maintainability | Mainframe/service OS ideas apply: concurrent transactions, isolation, resource accounting, backup and recovery. Redundancy is needed because a single OS update or hardware fault must not stop records access. |
| MRI/imaging compute | 1 throughput, 2 data integrity, 3 availability, 4 scalable memory/I/O, 5 security | Multiprocessor systems and parallel programming are appropriate. MRI reconstruction can use queued work and exploit SMP; unlike alarms, it usually optimizes completion time rather than a hard deadline. |
| Partner-hospital link | 1 confidentiality/integrity, 2 availability, 3 interoperability, 4 scalable throughput, 5 latency | Distributed systems must tolerate independent failures and network partitions. Exchange only through authenticated, audited interfaces; do not assume a remote partner is a trusted local process. |

Conflicts: hard deadlines can conflict with throughput and memory sharing; encryption, logging and redundancy consume CPU, storage and energy; aggressive tablet sleep conflicts with immediate delivery; database maintenance conflicts with continuous service. Patient-safety-critical functions are local alarm acquisition, alarm signaling, trustworthy timestamps, and safe fallback behavior. Alarm generation must not depend on the database, a cloud service, or partner connectivity.

**Single general-purpose OS alternative:** One desktop/server distribution on all devices reduces image and administration diversity, but ordinary fair-share scheduling cannot guarantee a finite worst-case alarm delay under overload: the worst-case scheduling wait is not bounded by the product’s normal-use latency. A general-purpose image and driver stack also consumes more RAM/flash and power than a tailored RTOS on monitors. Fewer OS families simplify patching, but a universal configuration creates a common-mode failure and still requires per-device security, performance and update policy. It is rejected for the monitor deadline.

See the [complete report](../REPORT.md) for the full submission and references.
