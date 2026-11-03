# Assignment 1 — Task 02

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 2 — OS types

| Environment | Selected type | Alternatives rejected / trade-off |
|---|---|---|
| Monitors | Embedded hard real-time, single-purpose | Soft RT and desktop systems offer no hard bound. Mainframe/cluster types are irrelevant at the sensor. |
| Tablets | Mobile/handheld | Desktop OS wastes power and exposes a broader service set. |
| Workstations | Desktop, centrally managed | Handheld lacks full clinical workstation capabilities; server OS is not optimized for interactive clinician use. |
| Database | Clustered server with failover | A standalone mainframe or large SMP server has a single-machine failure domain. A cluster costs more and needs quorum/fencing/replication engineering but limits downtime. |
| Imaging | Multiprocessor server, SMP Linux | AMP assigns fixed roles and can help specialized devices, but SMP is easier for general parallel reconstruction and load balancing. SMP has lock/contention scaling limits. |
| Partner link | Distributed system over secure network services | A single distributed OS spanning independent hospitals would impose unacceptable trust and failure coupling. Use interoperable services and explicit ownership boundaries. |

**Database alternative — one large multiprocessor:** It avoids cluster replication and may have lower initial integration cost; however, motherboard, power, kernel and maintenance remain single points of failure. A cluster can fail over within seconds if health checks, quorum, fencing and replicated state are configured and practiced; failover is not zero-loss or automatic without a consistency design. For parallel workloads, ideal CPU time can approach T₁/p with p processors (Amdahl’s law limits real speedup); memory capacity is the node’s installed RAM, whereas a cluster’s RAM is generally not one automatically shared address space. Cost comparison depends on vendor, licensing, storage and support quotes and is not asserted numerically here.

See the [complete report](../REPORT.md) for the full submission and references.
