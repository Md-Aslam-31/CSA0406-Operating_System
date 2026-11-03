# Assignment 1 — Task 04

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 4 — Services

| Service | Monitor | Tablet | Workstation | Database | Imaging | Partner link |
|---|---|---|---|---|---|---|
| UI | Minimal alarm/status | Touch UI | Full desktop | Admin/monitor only | Operator/queue | API/service only |
| Program execution | Fixed trusted tasks | Clinical apps | Clinical apps | DB engine/agents | Reconstruction workers | Gateway services |
| I/O | Sensor, alarm, watchdog | Display, radio, camera as needed | Display, keyboard, network | Storage/network | Scanner, fast storage/network | Network interfaces |
| File system | Minimal config/log | Encrypted managed storage | Encrypted managed storage | Journals, data, backups | Scratch and image archive | Queue/audit logs |
| Communication | Isolated local bus | Wi-Fi/cellular/VPN | Hospital network | Cluster and client links | Scanner/cluster | TLS APIs |
| Error detection | Watchdog, range checks, alarm | Health/MDM | Endpoint monitoring | ECC, checksums, replication | ECC/checksums | Validation/retries |
| Allocation/accounting | Static priority/bounded CPU | OS-managed energy/resources | Multi-user scheduling | QoS, quotas, telemetry | Scheduler/GPU/CPU | Rate limits/quotas |
| Protection/security | Secure boot, signed updates, least privilege | Device encryption, sandbox, MFA | MFA, least privilege | RBAC, audit, encryption, separation of duties | Least privilege, audit | Mutual auth, scoped consent, audit |

Remove from monitor: general-purpose shell, package manager, unused network daemons, broad UI, printing, general user accounts and nonessential filesystems. Keep only services needed for safe sensing, alarms, bounded logs, time, secure update and diagnostics. Database essentials include virtual memory, concurrency, durable filesystems, transactional synchronization, networking, audit, privilege separation, backup/restore and error reporting. Patient records are isolated using authenticated identities, database row/column policy where needed, process address spaces, least privilege, encrypted transport/storage, audit, and tested backup/restore. OS process isolation alone does not replace database authorization.

**Desktop OS on tablets alternative:** Extra services increase RAM/flash footprint, startup work, background wakeups and attack surface; sleep/radio settings may reduce battery duration. Exact MB/minutes require measurement on the chosen hardware and workload. A mobile OS is selected because power-aware scheduling and a narrower managed app profile better fit a shift-long device; full desktop compatibility may be preferable only if a clinical app requires it and battery testing supports it.

See the [complete report](../REPORT.md) for the full submission and references.
