# Assignment 1 — Task 11

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 11 — Complete recommendation

| Tier | OS plan | Services / protection / I/O | Main risk and control |
|---|---|---|---|
| Bedside | Minimal hard RTOS | Fixed-priority tasks, sensor interrupts, watchdog, signed updates, minimal logs; no cloud dependency | Deadline miss/driver fault; worst-case timing validation, independent alarm path, fail-safe state, clinical validation |
| Mobile | Managed mobile OS | Encrypted storage, sandbox, MFA, MDM, Wi-Fi/cellular TLS | Lost device/radio outage; remote revocation, local workflow, battery/notification testing |
| Clinical desktop | Managed desktop OS | MFA, endpoint controls, least privilege, audit, encrypted network | Credential theft/malware; patching, segmentation, monitoring |
| Database | Clustered Linux server | RBAC, audit, encrypted storage/network, checksums, synchronous replication as required, quorum/fencing, backups | Split brain, correlated failures, bad update; tested failover/restore, staged patching, independent failure domains |
| Imaging | SMP Linux compute nodes | Parallel job scheduler, DMA-capable I/O, ECC, checksum and access controls | Contention/driver/scanner faults; queueing, resource limits and validated data pipeline |
| Inter-hospital | Distributed services | Mutual TLS, scoped APIs, audit, retries/idempotency, versioned schema | WAN partition/data mismatch; local autonomy, reconciliation and governance |

Scale from one ward to 500 monitors/120 tablets/80 workstations by provisioning and managing devices centrally while retaining local monitor scheduling. For three hospitals, distribute identity and records interfaces, define authoritative record ownership, and test WAN partitions, replication lag and recovery. Risks include common-mode updates, vendor support/certification constraints, human-factor alarm fatigue, cluster misconfiguration and cost. Mitigate with staged rollouts, rollback, hazard analysis, operational drills, access review and measurements on target hardware.

Virtualizing everything on two large servers may improve consolidation and snapshots, but monitor timing can be affected by hypervisor scheduling, device emulation and noisy neighbors; only a real-time hypervisor with reserved CPUs and validated passthrough might qualify, and bedside alarms should remain local. Virtualization consumes host RAM/CPU for the hypervisor, guests and reserved headroom. Two hosts still share facility/network/storage risks and can fail together; HA requires independent power, storage/network paths and tested recovery. Keep safety-critical monitors physical/local; virtualization is appropriate for many server workloads with measured overhead and redundancy.

See the [complete report](../REPORT.md) for the full submission and references.
