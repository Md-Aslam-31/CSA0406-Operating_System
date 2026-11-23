# A1 - Evaluate Linux system components (7 marks)

## A1.1 Linux architecture

Linux is the kernel, not the whole distribution. It switches runnable processes, provides virtual memory and page tables, enforces file permissions, exposes block/network drivers and implements sockets and TCP/IP. User processes call system calls through the C library (glibc); system libraries provide reusable APIs; the shell parses commands and starts programs; utilities such as `ps`, `ss`, `df`, `journalctl` and `ip` administer or inspect the machine. The VFS gives applications common open/read/write operations over ext4, XFS and other file systems. A reverse proxy and Python application remain user processes; neither is part of the kernel.

The separation is useful: portal code cannot directly program a disk or NIC, so kernel privilege checks and drivers mediate access. A kernel or driver fault can still affect every tenant on that host, which is why minimal modules, timely security updates and VM/cloud isolation matter.

## A1.2 Processes, memory and observation

The assignment asks about CFS. CFS models fair CPU sharing with virtual runtime and a time ordered run queue. Current Linux documentation says CFS is making room for EEVDF; EEVDF selects an eligible task using virtual deadlines. Therefore describe CFS's fairness model, but record the actual kernel and fair scheduler in `/proc/version`, `uname -r` and `/sys/kernel/debug/sched` before claiming the host still uses the original implementation. Fair CPU sharing avoids one CPU-bound query monopolizing the machine; quotas and priorities still require careful tuning.

The page cache retains file data in otherwise unused RAM, speeding repeated reads. It is reclaimable under pressure; low `MemAvailable`, major faults and swap-in/swap-out are stronger pressure signals than the raw `free` number. Swap can absorb a short spike but adds latency and does not substitute for enough RAM. `/proc` provides process, memory, mount and kernel counters; `/sys` exposes device and driver attributes. Read-only inspection is safe; writing many sysfs knobs changes the host and is outside this lab.

## A1.3 Files, permissions and sudo

`/` is the root; `/etc` holds configuration, `/var/log` logs, `/home` user files, `/srv` service data, `/usr` installed programs, `/run` volatile runtime state and `/proc`/`/sys` kernel views. VFS dispatches path operations to a mounted file system. Owner/group/other read-write-execute bits are augmented by ACLs and capabilities. A setuid executable runs with its file owner's effective identity, so keep such programs rare and patched. `sudo` grants selected administrative commands with an audit trail. The portal runs under a dedicated unprivileged service account with no interactive shell; staff administer it through narrowly scoped sudo.

## Alternative: Windows Server

Windows Server Core is the fair comparison to a headless Ubuntu server; Desktop Experience adds GUI components. The exact installed RAM and disk footprint depend on release, roles, update level and selected image, so they should be measured with `Get-CimInstance Win32_OperatingSystem`, `Get-Volume` and a clean VM rather than guessed. Microsoft describes Server Core as a smaller-footprint option. On this Ubuntu 24.04 workflow, the recorded runner values provide the Linux baseline; Ubuntu's published installation minimum is 1.5 GB RAM and 5 GB storage, not a workload recommendation.

PowerShell Desired State Configuration and Windows Admin Center can automate Windows roles, but cross-host administration needs PowerShell/WinRM conventions in place of shell/systemd tooling. Microsoft publishes Windows Server 2025 Standard at $1,176 suggested MSRP for a 16-core license, plus required CALs; Datacenter is $6,771 for 16 cores. Those are USD reference prices and vary by country/reseller. Linux is attractive for this Python/Nginx stack because packages and automation integrate cleanly and there is no per-server Windows license, while Windows may be preferred where campus identity, vendor support or Windows-specific applications dominate. Compare total staffing/support costs too.

**Evaluation:** Linux offers observable kernel interfaces and scriptable administration with low licensing cost, but requires Linux skills and patch discipline. A commercial Windows license buys an integrated vendor-supported environment, but is unnecessary cost for a Python service if the university has no Windows-only dependency.

