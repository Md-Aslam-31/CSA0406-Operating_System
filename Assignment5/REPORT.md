# CSA0406 Operating Systems - Assignment 5 (CO5)

## Evaluate Linux, Containers and Virtualisation for the University Result Portal

**Student:** Mohamed Aslam H  
**Register number:** 192572147  
**Department / section:** CSE (AI)  
**Academic year:** 2025  
**Faculty:** Dr. Jayasakthi Velmurugan K  
**Nature:** Individual  
**Course repository:** [Assignment 5 folder](https://github.com/Md-Aslam-31/CSA0406-Operating_System/tree/main/Assignment5)  
**Linux run evidence:** [successful Ubuntu CI run](https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/45236369527)

## Executive summary

The assignment builds a small synthetic result portal and follows the requested workflow: understand Linux, design administration, virtualise, containerise, orchestrate, measure, interpret, and decide. The code contains no actual student result data or credentials. The Linux workflow records host observations, runs a hardened systemd service behind Nginx, executes the completed awk exercise and cron schedule, builds and inspects four container image variants, measures the Flask application directly and in Docker, attempts a QEMU guest run, then deploys it to a three-node kind cluster and verifies pod recovery and scaling under load. On the hosted runner QEMU TCG booted the Ubuntu guest and started cloud-init, but the application health check did not become reachable within 360 seconds; the VM result is clearly marked unavailable, not presented as a successful benchmark.

The engineering decision is a hybrid: keep routine traffic and a three-replica baseline on the university's existing machines, then provision cloud capacity before each of the two annual result releases. Keep the authoritative records behind a private database endpoint with encryption, backups and audited access. The measured request rate and computed replica/cost table are in `evidence/benchmark.csv` and `evidence/b4-capacity-cost.txt`; all figures are specific to this hosted test and must not be presented as a capacity guarantee for the real database-backed portal.

## Part A - Design (25 marks)

### A1 - Evaluate Linux system components (7 marks)

#### Linux architecture

Linux names the kernel. A distribution adds system libraries (such as glibc), a shell, utilities and package-management tools. Applications call kernel services through system calls; the kernel schedules processes, maps virtual memory, enforces files and credentials, drives devices and implements network sockets. The VFS dispatches common path/open/read/write operations to a mounted file system. A Flask process and Nginx proxy run in user space, so neither can directly issue disk or NIC instructions. This boundary reduces accidental hardware access, although a kernel/driver defect can affect every process on the host.

#### Process, memory and observation

The task asks for the Completely Fair Scheduler (CFS): its virtual-runtime model tries to give runnable tasks a fair CPU share and uses an ordered run queue. Linux's current documentation says CFS is making room for EEVDF. EEVDF still aims at fair sharing, but selects an eligible task by virtual deadline and lag. Therefore this report explains CFS as the assignment topic and captures the actual Ubuntu kernel version rather than implying that every current kernel uses the original CFS implementation unchanged.

The page cache uses available RAM to retain file data and is reclaimable when programs need memory. `MemAvailable`, major faults, swap-in/out and `vmstat` are more useful pressure indicators than `free` RAM alone. Swap can bridge a short peak but is much slower than memory. `/proc` exposes per-process and kernel counters; `/sys` exposes device/driver attributes and tunables. The evidence uses read-only observations and does not change host scheduling or memory settings.

#### Files and permissions

`/etc` stores configuration, `/var/log` logs, `/home` user data, `/srv` service data, `/usr` installed software, `/run` volatile runtime files, while `/proc` and `/sys` expose kernel state. Owner/group/other mode bits can be extended with ACLs and capabilities. Setuid executables run with the file owner's effective identity and should be rare. `sudo` grants selected administrative commands with auditing; the web worker uses a dedicated non-login identity instead of root.

#### Windows Server alternative

The comparable headless installation is Windows Server Core; Desktop Experience adds a GUI and more packages. Memory and disk consumption depend on edition, roles, updates and image, so compare a clean VM using `Get-CimInstance Win32_OperatingSystem` and `Get-Volume` instead of making an unsupported universal claim. Server Core has a smaller disk footprint than Desktop Experience. Ubuntu 24.04's published installation minimum is 1.5 GB RAM and 5 GB disk, not a web-application sizing recommendation.

PowerShell/DSC or Windows Admin Center can automate Windows services, but Windows uses different service, firewall and remote-management tooling from systemd, Nginx and shell scripts. Microsoft's Windows Server 2025 Standard reference price is USD $1,176 for a 16-core license; CALs are additional. Datacenter is $6,771 for 16 cores. Prices vary by country and reseller. For this Python stack, Linux avoids a per-server Windows license and has direct integration with the selected tools; Windows can be justified when campus identity integration, support or Windows-only software offsets license and operating cost.

### A2 - Design system administration (5 marks)

**Identities and privilege:** use named human accounts, MFA-backed SSH keys and groups such as `portal-ops`, `portal-readonly` and `db-ops`. Avoid shared administrator accounts. Run the portal as `result-portal` with `/usr/sbin/nologin`; PostgreSQL owns its data as a separate service account. Review membership and sudo logs. Store secrets in an approved secret store, not Git or command history.

**Service and logs:** `TaskB1/result-portal.service` uses two Gunicorn workers, a dedicated account, restart-on-failure, a restrictive umask, private temporary storage, read-only system paths and no Linux capabilities. Nginx is the only public HTTP entry and proxies to `127.0.0.1:8000`; production terminates TLS at Nginx or a managed load balancer. Forward journald and Nginx access/error logs to central storage. Alert on failed health checks, repeated 5xx, disk above 80%, swap pressure and certificate expiry. Retain student identifiers only under the university retention policy.

**Network:** use DHCP reservation/IPAM for private node addresses and managed DNS for the portal name. Publish only the load balancer. Default-deny inbound traffic; permit SSH only from VPN/bastion and TCP 80/443 at the web edge. Disable SSH password and root login. Keep PostgreSQL on a private subnet accepting only portal nodes. The CI firewall evidence uses only TCP 22, 80 and 443; a production rule should restrict port 22 to the campus management CIDR.

**Automation:** rotate logs, renew certificates, take encrypted database-consistent backups, test restores, check health and disk, and verify replication. `TaskB1/cron.txt` installs the hourly disk alert under the service account. systemd timers are a better production default for missed-run handling and journal integration.

**Ansible alternative:** `TaskA2/ansible/site.yml` expresses packages, account, files, service and firewall as desired state and runs in batches of five. A one-host hand-written setup may take an illustrative 15-30 minutes. Applying and checking k shell steps serially to 20 hosts is O(20k) operator actions and can take 5-10 engineer-hours, with configuration drift risk. Ansible has an illustrative 1-2 hour upfront role/inventory/key setup; with parallelism p, wall time is approximately O(k + n/p) batches (total machine work remains O(nk)). Idempotent modules make repeat runs converge. A faulty role can also propagate quickly, so use staging, code review and limited rollout batches. These are planning estimates, not a measurement on 20 campus servers.

### A3 - Evaluate virtualisation (5 marks)

Type-1 hypervisors manage physical hosts or provide a privileged host-kernel virtualization layer: examples include KVM, ESXi, Xen and Hyper-V. Type-2 hypervisors such as VirtualBox and VMware Workstation are applications hosted by a general-purpose OS. Full virtualization presents virtual devices to an unmodified guest; Intel VT-x or AMD-V accelerates guest execution. Paravirtual drivers such as virtio provide efficient I/O paths without changing the application.

VMs add kernel-level isolation, consolidate services, support controlled snapshots and may live-migrate when storage/network and processor compatibility allow it. They also consume RAM for each guest kernel/cache, add virtual I/O and need separate image backup and guest patching. A type-2 VM is convenient for student laptops and compatibility labs. Its CPU work passes through the host scheduler, while I/O crosses virtual devices and host paths; memory includes the host OS and each guest OS. Production should use a managed KVM/type-1 host or cloud VM rather than a desktop VirtualBox host. The comparative run labels KVM or QEMU TCG mode; TCG results are software emulation and cannot stand in for hardware-assisted VM performance.

For one VM, provisioning is O(1) operator workflow; maintaining n independent guest OSes still involves O(n) patch and backup streams. A type-1 cluster costs more to design initially, but centralized APIs allow scheduling and migration across hosts. VM count does not remove per-guest security maintenance.

### A4 - Evaluate containerisation (4 marks)

Linux namespaces scope a container's view of process IDs, mounts, network, IPC, hostname and users. cgroups account for and constrain CPU, memory and I/O. An image is an immutable stack of content-addressed layers; a container adds a writable layer. Containers share the host kernel, unlike VMs. This gives quick start and high density, but a host-kernel escape or weak privilege setup can cross container boundaries.

`TaskB2/Dockerfile` completes the worksheet: it sets `/app`, installs cached dependencies, copies the app, declares 8000, runs as `portal` and starts Gunicorn on `0.0.0.0:8000`. `Dockerfile.multistage` copies only installed runtime packages from its builder stage. Compose runs portal and PostgreSQL with a named database volume, health check, restart policy and CPU/memory caps. `docker history`, `docker image inspect`, `docker stats` and the `whoami` command provide layer, size, usage/limit and non-root evidence.

Use containers for the stateless web workers, with read-only filesystem, dropped capabilities, pinned/updated images and network policies. Use a managed database or a separately isolated VM for authoritative records. VMs provide a stronger separate-kernel boundary but add guest boot and memory. One VM per web/database service also creates O(n) guest patch streams; an image plus container scheduler centralizes common runtime changes. Actual overheads are measured below rather than inferred from image download size.

### A5 - Design cloud deployment and orchestration (4 marks)

**Service models:** IaaS rents VMs, disks and networks for the university to administer; PaaS operates much of the OS/runtime/database; SaaS supplies a finished service with least infrastructure control. For results, keep authoritative records in a private, encrypted, backed-up database endpoint. Place stateless portal replicas behind a TLS load balancer/WAF across zones. Cache only approved read-only content after validating invalidation and privacy. Pre-warm replicas, database connections and images before the announced release time.

Pods are Kubernetes scheduling units. A Deployment/ReplicaSet keeps a desired pod count; a Service supplies stable discovery and load-balancing; resource requests guide scheduling and HPA utilization while limits cap container use. Readiness probes keep unready pods out of endpoints. A failed pod is replaced; a failed node still requires spare healthy node capacity. HPA changes the desired replica count based on metrics; cluster/node autoscaling is separately needed when nodes are full. `TaskA5/deployment.yaml` completes three replicas, matching labels, 250m CPU/256Mi requests and 500m CPU/512Mi limits. `hpa.yaml` illustrates CPU-target scaling from 3 to 50 replicas.

```text
ALGORITHM 1: PREWARM RESULT PORTAL
BEGIN
    INPUT: predictedPeakRPS, measuredSafeRPSPerPod, targetUtilization
    SET requiredPods <- CEILING(predictedPeakRPS / (measuredSafeRPSPerPod * targetUtilization))
    IF requiredPods < 3 THEN
        SET requiredPods <- 3
    END IF
    SET portalDeployment.replicas <- requiredPods
    WAIT until all requiredPods pass readinessProbe
    IF readyPods < requiredPods THEN
        SEND capacityAlert to portal-ops
    END IF
    OUTPUT: readyPods, predictedPeakRPS, capacityAlertStatus
END
```

The arithmetic is O(1); reconciling and checking n replicas is O(n). Fixed result-day VMs respond immediately but leave peak-sized CPU/RAM idle for 364 days. Autoscaling saves idle capacity but adds metric/image-pull lag; scheduled pre-scaling and warm images mitigate that risk. The chosen hybrid keeps a minimum private baseline and adds cloud capacity before each result day. It avoids buying servers but requires cloud-region/data governance, a rehearsed network path and an explicit provider cost budget.

## Part B - Implementation and experimental investigation (25 marks)

### B1 - Administer the Linux server (7 marks)

#### Observation commands

The Ubuntu workflow runs `ps`, `top`, `free -h`, `vmstat 1 3`, `iostat -xz 1 2`, `df -h`, `ss -tulpen`, `uname -r`, `/proc/meminfo` and the CPU online view under `/sys`. The exact output is in `evidence/b1-observations.txt`. `ps` and `top` show process CPU/RSS; `free` shows available memory; `vmstat` shows run queue, paging and I/O waits over time; `iostat` reports device queue/latency; `df` reports filesystem capacity; `ss` shows bound/listening sockets. Runner values are a short-lived hosted Linux VM snapshot, not campus hardware capacity.

#### Completed awk code and automation

The TODO program uses `awk`, `sort` and `uniq -c`. The sample's quoted request is split by awk on spaces; status is field 7 and response bytes field 8. The run matches the provided expected output:

```text
Requests per status code:
  200 4
  302 1
  403 1
  404 1
Busiest client:
  10.0.0.5 4
Total bytes sent: 43200
```

`log_summary_corrected.sh` independently emits the same result. The disk alert uses an 80% default threshold; CI also sets 0% to exercise its warning branch safely without filling the runner. The exact hourly schedule is `0 * * * * /usr/local/sbin/disk-alert.sh >> /var/log/disk-alert.log 2>&1`. The cron service runs the line under the non-login `result-portal` account.

#### Service, network and design comparison

`setup-service.sh` installs the app under `/opt/result-portal`, creates the service identity and starts `result-portal.service` with systemd. Nginx proxies port 80 to loopback port 8000; UFW default-denies inbound and permits TCP 22/80/443. Health and synthetic result responses plus `systemctl`, `journalctl` and firewall state are saved in `evidence/b1-*.txt`. The built design aligns with Part A; in production the SSH rule must be limited to the campus VPN/bastion, HTTPS/TLS must be configured, and firewall changes must follow change control.

**Central logging alternative:** with local scripts, finding an error across ten hosts means ten SSH/log scans (O(n) visits). A central journal/log index lets an operator issue one indexed query, approximately O(log N) retrieval after ingestion. The index, replicas and buffers consume additional disk/RAM. Size them using daily log bytes x retention days x index/replication factor. TLS, access roles, retention, clock sync, ingestion alarms and local fallback spooling add setup work. Do not log passwords or unnecessary student identifiers.

### B2 - Containerise the portal (5 marks)

The Dockerfile is built and run with Compose. The portal is non-root; PostgreSQL uses a named `result-db` volume, and the portal waits on a database health check. Compose applies 0.5 CPU/512 MiB per service and binds app ports to loopback. `/results/10001` reads synthetic seed data. The database is restarted while retaining the volume; the same row must remain. `/results/99999` returns 404 as the not-found boundary case. The workflow captures `docker ps`, `docker stats`, image layers, bytes, package counts and `docker exec ... whoami`.

The evidence compares `python:3.12`, `python:3.12-slim`, a slim multi-stage build and Ubuntu 24.04. `evidence/b2-images.txt` records build duration, image bytes, layer history and installed Debian package count; `plots/image-size.png` visualizes size. This run measured 1,079.5 MiB (full Python), 135.1 MiB (slim Python), 127.4 MiB (multi-stage) and 141.9 MiB (Ubuntu), respectively. Fewer OS packages reduce image transfer and patch surface, but package count is only a proxy, not a CVE scan. The multi-stage build avoids carrying builder artifacts into the runtime. A transfer lower bound at effective bandwidth B is image_bytes x 8 / B; actual pulls also depend on shared cached layers and network contention. Use the measured sizes rather than assuming the slim or multi-stage image wins for every Python app.

If a full Ubuntu image were chosen, it would carry a general OS base and more packages to maintain; a multi-stage build keeps only runtime dependencies. Build time and actual disk sizes are recorded. This app's pure-Python packages need no compiler in runtime. A real vulnerability count requires a current scanner/database and a selected advisory policy; the package-count comparison alone does not prove the image is vulnerability-free.

### B3 - Compare bare metal, VM and container (8 marks)

The same Flask source, two Gunicorn workers and CPU-cost `/simulate/login` endpoint are benchmarked on the Ubuntu host and in Docker. `/health` is a lightweight readiness route. The benchmark runs ApacheBench with 2,000 requests and concurrency 25 after readiness. It records first-health startup time, RSS, disk/image footprint, requests/second and mean latency in `evidence/benchmark.csv`; raw ApacheBench summaries are `benchmark-host.txt` and `benchmark-container.txt`. A QEMU/KVM Ubuntu guest was also attempted with 1,536 MiB assigned RAM. This hosted environment had no usable KVM device, so QEMU used TCG; the guest booted and started its service, but the forwarded health endpoint remained unreachable during the 360-second check. Accordingly, no VM performance row is fabricated. For a complete VM timing, rerun `scripts/run_vm_benchmark.sh` on a Linux host with working KVM.

| Deployment | Startup, RSS, size, requests/s and mean latency |
|---|---|
| Direct Linux | 0.339 s startup; 83.5 MiB RSS; 857.97 requests/s; 29.138 ms mean latency |
| Docker | 0.429 s startup; 46.6 MiB RSS; 135.1 MiB image; 263.61 requests/s; 94.837 ms mean latency |
| QEMU VM | Unavailable: TCG guest health endpoint did not become reachable within 360 s. See `evidence/vm-benchmark.txt`. |

The run values are from a hosted Ubuntu CI VM. A short synthetic CPU endpoint omits database locks, TLS, real query mix, connection pooling, client think time and a result-day arrival curve. It is evidence for relative environment overhead on this runner only; use the lower p95 capacity from a representative campus test for production.

#### Kubernetes deployment, failure and scale

`TaskB3/deployment.yaml` applies three replicas with 250m/256Mi requests and 500m/512Mi limits. kind is a three-node local Kubernetes cluster; its Service uses ClusterIP because the lab has no cloud load-balancer controller. The production sample retains LoadBalancer. The evidence includes `kubectl get pods`, `describe`, and resource fields; the runner deletes one pod, waits for the Deployment replacement and captures the new three-pod set. A 30-thread request load then runs while the Deployment is raised to six replicas; the request count/error summary, scale command and six Ready pods are retained.

Bare metal minimizes layers but one process/host failure removes service. A VM isolates its guest kernel but adds guest memory and boot time. Containers start quickly and share image layers, but a host kernel or misconfigured privilege can affect every container. Docker Compose can scale replicas on one host (O(n) containers) but cannot recover from loss of that host or schedule across machines. Kubernetes adds rollout, service discovery and rescheduling across nodes, at the cost of a cluster control plane, metrics, networking and operational expertise. Use direct Linux for a single controlled legacy workload, VMs for separate kernels/OSes, containers for portable stateless services, and Kubernetes where multi-node capacity and automated recovery justify the added operations.

### B4 - Final engineering decision (5 marks)

The `lms` Deployment adds a second service with two replicas, 200m CPU/128Mi requests and 400m CPU/256Mi limits. `evidence/b4-lms-resources.yaml` records the applied values; each service has its own selector and Service. The sample image is shared for demonstration only; production should use separate application images, secrets, data services and network policies.

#### Capacity estimate

150,000 students in two hours implies 20.83 logins/second if each logs in once. The calculator tests 1, 5 and 10 HTTP requests per student, applies a documented 5x flash-crowd factor and holds pods to 70% of the measured container request rate. For each scenario:

```text
average_rps = students * requests_per_student / 7200
modeled_peak_rps = average_rps * 5
required_pods = ceil(modeled_peak_rps / (measured_container_rps * 0.70))
```

It keeps a three-replica fault-tolerance floor. The actual arithmetic and low/high boundary cases are saved in `evidence/b4-capacity-cost.txt`, `capacity-low.txt` and `capacity-high.txt`. The synthetic PBKDF2 route is still optimistic versus a real database lookup; repeat the same load shape with an anonymized, representative query before choosing a production number.

#### Production recommendation and cost sensitivity

Use existing campus servers for routine load, containerize portal/LMS services, and burst into a managed public-cloud tier for the two annual release windows. Pre-scale ahead of the known event and keep the authoritative database private, encrypted, backed up and audited. This meets the “no new campus servers” constraint while preserving a private baseline; a cloud-only deployment offers simpler elasticity but places all student data and recurring service cost under provider contracts.

The reproducible cost model uses USD $0.0835 per Linux `t3.large` instance-hour as the published AWS US East (N. Virginia) example. Let N be the conservative measured capacity result, with at least three instances. A fixed cloud fleet costs N x 8,760 x $0.0835/year. Cloud-only with autoscaling keeps three instances all year, then adds max(0,N-3) instances for eight burst hours (one four-hour window each semester). Hybrid uses existing campus machines for the baseline and pays only max(0,N-3) x 8 burst hours x $0.0835 for cloud compute; existing campus costs are sunk/excluded. The three totals are printed with N in `evidence/b4-capacity-cost.txt`. They exclude managed Kubernetes control plane, database, load balancer, disks, backups, network egress, taxes, support and campus-host cost, so they are a sensitivity comparison rather than a quote. A pre-warmed pool can add capacity in minutes; a cold VM/cluster start is slower, so scale before the announcement. Replace the unit price with the procurement region's current calculator quote before approval.

## Boundary and failure evidence

| Check | Expected behavior | Evidence |
|---|---|---|
| Log sample: status and busiest host | 200x4; 10.0.0.5x4; 43,200 bytes | `b1-log-summary.txt` |
| Disk alert lower threshold | Warning branch without filling disk | `b1-disk-alert-boundary.txt` |
| Database missing row | HTTP 404 | `b2-boundary.txt` |
| DB restart | Synthetic row survives because data is in named volume | `b2-volume-persistence.json` |
| Pod deletion | Deployment restores three Ready replicas | `b3-recovery.txt`, `b3-pods-recovered.txt` |
| Load + scale | Request load completes and six replicas become Ready | `b3-load-under-scale.txt`, `b3-six-ready.txt` |
| Capacity extremes | Low measured rate scales up; high rate respects three-pod floor | `capacity-low.txt`, `capacity-high.txt` |

## Reproduction

Reproduce from Ubuntu 24.04 with `bash scripts/run_linux_evidence.sh`; command output and charts are saved under `evidence/` and `plots/`. The GitHub Actions workflow runs the same script and commits the generated evidence.

## References

- Linux: [CFS](https://docs.kernel.org/scheduler/sched-design-CFS.html), [EEVDF](https://docs.kernel.org/scheduler/sched-eevdf.html), [cgroups](https://docs.kernel.org/admin-guide/cgroup-v2.html), [namespaces](https://man7.org/linux/man-pages/man7/namespaces.7.html).
- Platforms: [Ubuntu 24.04 requirements](https://ubuntu.com/server/docs/reference/installation/system-requirements/), [Windows Server Core](https://learn.microsoft.com/windows-server/administration/server-core/what-is-server-core), [Windows pricing](https://www.microsoft.com/en-us/windows-server/pricing).
- Operations: [systemd](https://www.freedesktop.org/software/systemd/man/latest/systemd.service.html), [Ansible](https://docs.ansible.com/projects/ansible/latest/playbook_guide/playbooks_intro.html), [Docker](https://docs.docker.com/build/building/best-practices/), [Kubernetes](https://kubernetes.io/docs/concepts/workloads/controllers/deployment/), [kind](https://kind.sigs.k8s.io/docs/user/quick-start/), [AWS T3 pricing](https://aws.amazon.com/ec2/instance-types/t3/).

## Preliminary capacity arithmetic screenshot

This first standalone calculation is preliminary. The complete OnlineGDB file-reading run below captured the preceding benchmark sample at 303.12 requests/s. A later Linux evidence run (#45451421902) refreshed the committed benchmark CSV to 270.20 requests/s; the current capacity table and cost figures in this report use that latest run.

![Preliminary OnlineGDB capacity arithmetic](evidence/onlinegdb/taskb4-capacity-calculation.jpg)
## Complete capacity estimator run in OnlineGDB

The full `TaskB4/capacity_estimate.py` was entered and run with the then-committed benchmark CSV. OnlineGDB uses a flat project directory, so that run copy changed only the default CSV path from `evidence/benchmark.csv` to `benchmark.csv`; the script logic and input rows were unchanged. The captured run measured 303.12 requests/s and shows all 1/5/10 request-per-student scenarios and annual compute-only costs. The subsequent Linux evidence run #45451421902 remeasured 270.20 requests/s; the latest CSV and capacity/cost transcript in this repository are authoritative for the current written analysis. This difference is expected for a noisy synthetic benchmark.

![Capacity estimator scenarios and final summary](evidence/onlinegdb/capacity_estimate-full.jpg)

![Capacity estimator scenario rows](evidence/onlinegdb/capacity_estimate-output-start.jpg)

OnlineGDB cannot run the Linux shell, Docker Compose, PostgreSQL, Kubernetes/kind or system-service tasks in this assignment. Their complete command outputs and health/recovery evidence are retained under `evidence/` and are reproducible with `bash scripts/run_linux_evidence.sh` on Ubuntu 24.04.

## Appendix - Full source code listings

This appendix reproduces all assignment programs, implementation scripts, and deployment/workflow configuration. Evidence screenshots and raw outputs appear earlier in this report and under `evidence/`. The exact source files are also retained at the listed repository paths. Report line wrapping is visual only.

### `scripts/plot_results.py`

```python
#!/usr/bin/env python3
"""Render workload throughput/startup and container image comparison figures."""
import csv
import re
from pathlib import Path

import matplotlib.pyplot as plt

base = Path(__file__).resolve().parent.parent
with (base / "evidence/benchmark.csv").open(newline="", encoding="utf-8") as stream:
    measurements = list(csv.DictReader(stream))
usable = [row for row in measurements if row["requests_per_second"]]
if not usable:
    raise SystemExit("No benchmark request rates found")

fig, axes = plt.subplots(1, 2, figsize=(10, 4.2))
labels = [row["deployment"] for row in usable]
rates = [float(row["requests_per_second"]) for row in usable]
startup = [float(row["startup_seconds"]) for row in usable]
axes[0].bar(labels, rates, color=["#175cd3", "#12b76a", "#f79009"][: len(labels)])
axes[0].set_ylabel("Requests per second (higher is better)")
axes[0].set_title("Same /simulate/login route, ab -n 2000 -c 25")
axes[0].grid(axis="y", alpha=.25)
axes[1].bar(labels, startup, color=["#175cd3", "#12b76a", "#f79009"][: len(labels)])
axes[1].set_ylabel("Seconds to first successful health check")
axes[1].set_title("Measured startup path")
axes[1].grid(axis="y", alpha=.25)
fig.tight_layout()
fig.savefig(base / "plots/deployment-comparison.png", dpi=180)

text = (base / "evidence/b2-images.txt").read_text(encoding="utf-8")
variants, sizes = [], []
for match in re.finditer(r'image=\["?result-portal:([^\]" ]+)"?\] bytes=(\d+)', text):
    variants.append(match.group(1))
    sizes.append(int(match.group(2)) / 1024 / 1024)
if not variants:
    raise SystemExit("No Docker image sizes found in evidence/b2-images.txt")
fig, ax = plt.subplots(figsize=(8, 4.2))
ax.bar(variants, sizes, color="#7f56d9")
ax.set_ylabel("Image size (MiB)")
ax.set_title("Measured portal image variants")
ax.grid(axis="y", alpha=.25)
fig.tight_layout()
fig.savefig(base / "plots/image-size.png", dpi=180)
```

### `scripts/run_linux_evidence.sh`

```bash
#!/usr/bin/env bash
set -Eeuo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence plots
: > evidence/verification.txt
exec > >(tee evidence/run.log) 2>&1
trap 'code=$?; echo "run failed at line $LINENO (exit $code): $BASH_COMMAND" | tee -a evidence/verification.txt; exit "$code"' ERR

echo '=== Environment ==='
{
  . /etc/os-release
  printf 'Linux: %s\n' "$PRETTY_NAME"
  uname -a
  gcc --version | head -n1
  python3 --version
  docker --version
  docker compose version
  if [[ -e /dev/kvm ]]; then echo 'Virtualization: KVM'; else echo 'Virtualization: QEMU TCG fallback'; fi
} | tee evidence/environment.txt

sudo apt-get update -qq
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -qq sysstat nginx ufw cron apache2-utils qemu-system-x86 qemu-utils cloud-image-utils > evidence/apt-install.log 2>&1

echo '=== B1.1 Linux observations ===' | tee evidence/b1-observations.txt
{
  echo '$ ps -eo pid,ppid,user,%cpu,%mem,rss,comm --sort=-%cpu | head -n 15'
  ps -eo pid,ppid,user,%cpu,%mem,rss,comm --sort=-%cpu | head -n 15
  echo '$ top -b -n1 | head -n 15'; top -b -n1 | head -n 15
  echo '$ free -h'; free -h
  echo '$ vmstat 1 3'; vmstat 1 3
  echo '$ iostat -xz 1 2'; iostat -xz 1 2
  echo '$ df -h'; df -h
  echo '$ ss -tulpen'; ss -tulpen
  echo '$ uname -r'; uname -r
  echo '$ head /proc/meminfo'; head /proc/meminfo
  echo '$ cat /sys/devices/system/cpu/online'; cat /sys/devices/system/cpu/online
} | tee -a evidence/b1-observations.txt

echo '=== B1.2 log analysis, disk alert and cron ==='
bash TaskB1/log_summary.sh | tee evidence/b1-log-summary.txt
bash TaskB1/log_summary_corrected.sh | tee evidence/b1-log-summary-independent.txt
grep -Fq '  200 4' evidence/b1-log-summary.txt
grep -Fq '  10.0.0.5 4' evidence/b1-log-summary.txt
grep -Fq 'Total bytes sent: 43200' evidence/b1-log-summary.txt
ALERT_THRESHOLD=0 bash TaskB1/disk-alert.sh | tee evidence/b1-disk-alert-boundary.txt
grep -F '0 * * * *' TaskB1/cron.txt | tee evidence/b1-cron.txt

echo '=== B1.3 systemd, Nginx and firewall ==='
if [[ ! -d /run/systemd/system ]] || ! command -v systemctl >/dev/null; then
  echo 'systemd is unavailable on this runner; service execution cannot be measured.' | tee evidence/b1-services.txt
  exit 1
fi
sudo bash TaskB1/setup-service.sh
sudo install -o result-portal -g result-portal -m 0640 /dev/null /var/log/disk-alert.log
sudo -u result-portal crontab -l 2>/dev/null > evidence/b1-cron-current.txt || true
sudo -u result-portal crontab TaskB1/cron.txt
sudo systemctl enable --now cron
sudo -u result-portal crontab -l | tee -a evidence/b1-cron.txt
sudo ufw --force reset >/dev/null
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow 22/tcp
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
sudo ufw --force enable
sudo ufw status verbose | tee evidence/b1-firewall.txt
sudo systemctl status --no-pager result-portal nginx | tee evidence/b1-services.txt
curl --retry 12 --retry-connrefused --retry-delay 2 -fsS http://127.0.0.1/health | tee evidence/b1-service-health.json
curl -fsS http://127.0.0.1/results/10001 | tee evidence/b1-service-result.json
sudo journalctl -u result-portal -n 15 --no-pager | tee evidence/b1-journal.txt
# The system service used host port 8000; stop it after preserving B1 evidence so
# the separately managed Compose deployment can publish that port in B2.
sudo systemctl stop result-portal nginx

echo '=== B3.1 bare-metal deployment ==='
python3 -m venv .venv
.venv/bin/pip install --disable-pip-version-check -q -r webapp/requirements.txt
printf 'deployment,startup_seconds,rss_mib,size_mib,requests_per_second,mean_latency_ms,notes\n' > evidence/benchmark.csv
host_start=$(date +%s%3N)
APP_NAME='University Result Portal' .venv/bin/gunicorn --chdir "$PWD/webapp" --workers 2 --bind 127.0.0.1:8100 --access-logfile "$PWD/evidence/host-access.log" --error-logfile "$PWD/evidence/host-error.log" app:app >/dev/null 2>&1 &
host_pid=$!
for _ in $(seq 1 40); do
  if curl -fsS http://127.0.0.1:8100/health >/dev/null; then break; fi
  sleep .25
done
host_end=$(date +%s%3N)
host_start_s=$(awk -v s="$host_start" -v e="$host_end" 'BEGIN {printf "%.3f",(e-s)/1000}')
curl -fsS http://127.0.0.1:8100/health | tee evidence/benchmark-host-health.json
ab -n 2000 -c 25 -k http://127.0.0.1:8100/simulate/login > evidence/benchmark-host.txt
host_rss_kib=$(ps -o rss= -p "$host_pid" --ppid "$host_pid" | awk '{s+=$1} END {print s+0}')
host_rss_mib=$(awk -v x="$host_rss_kib" 'BEGIN {printf "%.1f",x/1024}')
app_size_mib=$(du -sm webapp | awk '{print $1}')
python3 - evidence/benchmark-host.txt "$host_start_s" "$host_rss_mib" "$app_size_mib" >> evidence/benchmark.csv <<'PY'
import re,sys
s=open(sys.argv[1]).read(); r=re.search(r"Requests per second:\s*([0-9.]+)",s); l=re.search(r"Time per request:\s*([0-9.]+)\s+\[ms\] \(mean\)",s)
print(f"bare-metal,{sys.argv[2]},{sys.argv[3]},{sys.argv[4]},{r.group(1) if r else ''},{l.group(1) if l else ''},Linux host")
PY
echo '=== B3.1 VM deployment ==='
bash scripts/run_vm_benchmark.sh

echo '=== B2 image comparison and Compose deployment ==='
full_s=$(date +%s%3N)
docker build -f TaskB2/Dockerfile.full -t result-portal:full . > evidence/docker-build-full.txt
full_e=$(date +%s%3N)
slim_s=$(date +%s%3N)
docker build -f TaskB2/Dockerfile -t result-portal:compose . > evidence/docker-build-slim.txt
slim_e=$(date +%s%3N)
multi_s=$(date +%s%3N)
docker build -f TaskB2/Dockerfile.multistage -t result-portal:multistage . > evidence/docker-build-multistage.txt
multi_e=$(date +%s%3N)
ubuntu_s=$(date +%s%3N)
docker build -f TaskB2/Dockerfile.ubuntu -t result-portal:ubuntu . > evidence/docker-build-ubuntu.txt
ubuntu_e=$(date +%s%3N)
{
  docker image ls result-portal
  printf 'build_ms full_python=%s slim_python=%s multistage=%s ubuntu_base=%s\n' "$((full_e-full_s))" "$((slim_e-slim_s))" "$((multi_e-multi_s))" "$((ubuntu_e-ubuntu_s))"
  for tag in full compose multistage ubuntu; do
    echo "docker history result-portal:$tag"
    docker history result-portal:"$tag" | sed -n '1,12p'
    docker image inspect result-portal:"$tag" --format 'image={{.RepoTags}} bytes={{.Size}}'
    docker run --rm --entrypoint sh result-portal:"$tag" -c 'printf "installed_deb_packages="; dpkg-query -W -f="\${binary:Package}\n" | wc -l' 2>/dev/null || true
  done
} | tee evidence/b2-images.txt
docker compose -f TaskB2/compose.yaml up -d --build | tee evidence/b2-compose-up.txt
ready=0
for _ in $(seq 1 60); do
  if curl -fsS http://127.0.0.1:8000/health >/dev/null && curl -fsS http://127.0.0.1:8001/health >/dev/null; then ready=1; break; fi
  sleep 2
done
[[ $ready -eq 1 ]]
curl -fsS http://127.0.0.1:8000/health | tee evidence/b2-portal-health.json
curl -fsS http://127.0.0.1:8000/results/10001 | tee evidence/b2-db-result.json
unknown_code=$(curl -sS -o evidence/b2-unknown-result.json -w '%{http_code}' http://127.0.0.1:8000/results/99999)
echo "unknown_student_http_status=$unknown_code" | tee evidence/b2-boundary.txt
[[ "$unknown_code" == 404 ]]
docker compose -f TaskB2/compose.yaml ps | tee evidence/b2-compose-ps.txt
docker stats --no-stream | tee evidence/b2-stats.txt
docker exec "$(docker compose -f TaskB2/compose.yaml ps -q portal)" whoami | tee evidence/b2-container-user.txt
docker compose -f TaskB2/compose.yaml down | tee evidence/b2-compose-down.txt
docker compose -f TaskB2/compose.yaml up -d | tee evidence/b2-compose-restart.txt
for _ in $(seq 1 30); do
  if curl -fsS http://127.0.0.1:8000/results/10001 >/dev/null; then break; fi
  sleep 2
done
curl -fsS http://127.0.0.1:8000/results/10001 | tee evidence/b2-volume-persistence.json
docker compose -f TaskB2/compose.yaml down

echo '=== B3.1 container deployment and benchmark ==='
container_start=$(date +%s%3N)
docker run -d --rm --name portal-bench --cpus=0.5 --memory=512m -p 127.0.0.1:8000:8000 result-portal:compose | tee evidence/benchmark-container-id.txt
for _ in $(seq 1 40); do
  if curl -fsS http://127.0.0.1:8000/health >/dev/null; then break; fi
  sleep .25
done
container_end=$(date +%s%3N)
container_start_s=$(awk -v s="$container_start" -v e="$container_end" 'BEGIN {printf "%.3f",(e-s)/1000}')
ab -n 2000 -c 25 -k http://127.0.0.1:8000/simulate/login > evidence/benchmark-container.txt
docker stats --no-stream portal-bench | tee evidence/benchmark-container-stats.txt
container_size_bytes=$(docker image inspect result-portal:compose --format '{{.Size}}')
container_size_mib=$(awk -v x="$container_size_bytes" 'BEGIN {printf "%.1f",x/1048576}')
container_rss_mib=$(docker stats --no-stream --format '{{.MemUsage}}' portal-bench | python3 -c 'import re,sys; s=sys.stdin.read().split(" /")[0]; m=re.fullmatch(r"([0-9.]+)(B|KiB|MiB|GiB)",s); print(round(float(m[1])*{"B":1/1048576,"KiB":1/1024,"MiB":1,"GiB":1024}[m[2]],1) if m else 0)')
python3 - evidence/benchmark-container.txt "$container_start_s" "$container_rss_mib" "$container_size_mib" >> evidence/benchmark.csv <<'PY'
import re,sys
s=open(sys.argv[1]).read(); r=re.search(r"Requests per second:\s*([0-9.]+)",s); l=re.search(r"Time per request:\s*([0-9.]+)\s+\[ms\] \(mean\)",s)
print(f"container,{sys.argv[2]},{sys.argv[3]},{sys.argv[4]},{r.group(1) if r else ''},{l.group(1) if l else ''},Docker image; 0.5 CPU / 512 MiB cap")
PY
docker rm -f portal-bench >/dev/null

echo '=== B3.3 kind three-pod cluster, deletion recovery, load and scale ==='
mkdir -p "$RUNNER_TEMP/assignment5-bin"
curl -fsSLo "$RUNNER_TEMP/assignment5-bin/kind" https://kind.sigs.k8s.io/dl/v0.33.0/kind-linux-amd64
chmod +x "$RUNNER_TEMP/assignment5-bin/kind"
export PATH="$RUNNER_TEMP/assignment5-bin:$PATH"
if ! command -v kubectl >/dev/null; then
  curl -fsSLo "$RUNNER_TEMP/assignment5-bin/kubectl" https://dl.k8s.io/release/v1.37.0/bin/linux/amd64/kubectl
  chmod +x "$RUNNER_TEMP/assignment5-bin/kubectl"
fi
kind create cluster --name assignment5 --config TaskB3/kind-config.yaml --wait 180s | tee evidence/b3-kind-create.txt
kind load docker-image result-portal:compose --name assignment5 | tee evidence/b3-kind-load.txt
kubectl apply -f TaskB3/deployment.yaml | tee evidence/b3-apply.txt
kubectl rollout status deployment/result-portal --timeout=180s | tee evidence/b3-initial-rollout.txt
kubectl get pods -l app=result-portal -o wide | tee evidence/b3-pods-three.txt
old_pod=$(kubectl get pod -l app=result-portal -o jsonpath='{.items[0].metadata.name}')
kubectl describe pod "$old_pod" | tee evidence/b3-pod-resources.txt
echo "Deleting pod $old_pod" | tee evidence/b3-pod-delete.txt
kubectl delete pod "$old_pod" | tee -a evidence/b3-pod-delete.txt
kubectl rollout status deployment/result-portal --timeout=180s | tee evidence/b3-recovery.txt
kubectl get pods -l app=result-portal -o wide | tee evidence/b3-pods-recovered.txt

echo '=== B4.1 second workload with independent resource limits ==='
kubectl apply -f TaskB4/additional-service.yaml | tee evidence/b4-apply.txt
kubectl rollout status deployment/lms --timeout=180s | tee evidence/b4-lms-ready.txt
kubectl get deployment lms -o yaml | tee evidence/b4-lms-resources.yaml
kubectl port-forward service/result-portal 18000:80 > evidence/b3-port-forward.txt 2>/dev/null &
pf_pid=$!
for _ in $(seq 1 30); do
  if curl -fsS http://127.0.0.1:18000/health >/dev/null; then break; fi
  sleep 1
done
python3 TaskB3/load_burst.py http://127.0.0.1:18000/simulate/login 20 30 > evidence/b3-load-under-scale.txt &
load_pid=$!
sleep 2
kubectl scale deployment/result-portal --replicas=6 | tee evidence/b3-scale-six.txt
kubectl rollout status deployment/result-portal --timeout=180s | tee evidence/b3-six-ready.txt
wait "$load_pid"
kill "$pf_pid" 2>/dev/null || true
kubectl get deployments,pods -o wide | tee evidence/b3-final-pods.txt
kind delete cluster --name assignment5 | tee evidence/b3-kind-delete.txt
kill "$host_pid" 2>/dev/null || true

echo '=== B4 capacity, annual compute-only sensitivity and plots ==='
python3 TaskB4/capacity_estimate.py evidence/benchmark.csv | tee evidence/b4-capacity-cost.txt
printf 'deployment,startup_seconds,rss_mib,size_mib,requests_per_second,mean_latency_ms,notes\ncontainer,0,0,0,1,0,test\n' > evidence/capacity-low.csv
python3 TaskB4/capacity_estimate.py evidence/capacity-low.csv > evidence/capacity-low.txt
printf 'deployment,startup_seconds,rss_mib,size_mib,requests_per_second,mean_latency_ms,notes\ncontainer,0,0,0,10000,0,test\n' > evidence/capacity-high.csv
python3 TaskB4/capacity_estimate.py evidence/capacity-high.csv > evidence/capacity-high.txt
grep -Fq 'conservative_10_request_replica_count=ceil(peak/(measured_rps*0.70))=1489' evidence/capacity-low.txt
grep -Fq 'three-replica_safety_floor=3' evidence/capacity-high.txt
python3 scripts/plot_results.py
printf 'OK: B1 TODO output, Nginx/systemd/ufw health, Docker build/user/limits/DB volume, three deployment rates, pod replacement, six replicas during load, LMS limits, capacity model and charts.\n' | tee evidence/verification.txt
```

### `scripts/run_vm_benchmark.sh`

```bash
#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence
if ! command -v qemu-system-x86_64 >/dev/null || ! command -v cloud-localds >/dev/null; then
  echo 'VM result: SKIPPED; QEMU or cloud-localds is not installed.' | tee evidence/vm-benchmark.txt
  exit 0
fi
work=$(mktemp -d)
pidfile="$work/vm.pid"
cleanup() {
  if [[ -f "$pidfile" ]]; then kill "$(cat "$pidfile")" 2>/dev/null || true; fi
  rm -rf "$work"
}
trap cleanup EXIT
mkdir -p "$work/wheels"
python3 -m pip download --only-binary=:all: --dest "$work/wheels" Flask==3.1.2 gunicorn==23.0.0
tar -czf "$work/wheels.tar.gz" -C "$work/wheels" .
app_b64=$(base64 -w0 webapp/app.py)
wheels_b64=$(base64 -w0 "$work/wheels.tar.gz")
cat > "$work/server.py" <<'PY'
import sys
sys.path.insert(0, "/opt/site-packages")
sys.argv = ["gunicorn", "--workers", "2", "--bind", "0.0.0.0:8000", "app:app"]
from gunicorn.app.wsgiapp import run
run()
PY
server_b64=$(base64 -w0 "$work/server.py")
cat > "$work/install-wheels.py" <<'PY'
import os
import tarfile

os.makedirs("/opt/site-packages", exist_ok=True)
with tarfile.open("/opt/wheels.tar.gz", "r:gz") as archive:
    archive.extractall("/opt/site-packages", filter="data")
PY
installer_b64=$(base64 -w0 "$work/install-wheels.py")
image="$work/noble-cloud.img"
overlay="$work/portal-vm.qcow2"
seed="$work/seed.iso"
curl -fsSL --retry 3 https://cloud-images.ubuntu.com/noble/current/noble-server-cloudimg-amd64.img -o "$image"
qemu-img create -q -f qcow2 -F qcow2 -b "$image" "$overlay" 12G
cat > "$work/user-data" <<EOF
#cloud-config
users:
  - name: portalvm
    system: true
    shell: /usr/sbin/nologin
write_files:
  - path: /opt/app.py
    encoding: b64
    permissions: '0644'
    content: $app_b64
  - path: /opt/server.py
    encoding: b64
    permissions: '0755'
    content: $server_b64
  - path: /opt/install-wheels.py
    encoding: b64
    permissions: '0644'
    content: $installer_b64
  - path: /opt/wheels.tar.gz
    encoding: b64
    permissions: '0644'
    content: $wheels_b64
  - path: /etc/systemd/system/vm-portal.service
    permissions: '0644'
    content: |
      [Unit]
      Description=Result portal VM benchmark
      After=network.target
      [Service]
      Type=simple
      User=portalvm
      WorkingDirectory=/opt
      Environment="PYTHONPATH=/opt/site-packages"
      Environment="APP_NAME=University Result Portal"
      ExecStart=/usr/bin/python3 /opt/server.py
      Restart=on-failure
      [Install]
      WantedBy=multi-user.target
runcmd:
  - [python3, /opt/install-wheels.py]
  - [systemctl, enable, --now, vm-portal.service]
EOF
cat > "$work/meta-data" <<'EOF'
instance-id: assignment5-vm-01
local-hostname: result-portal-vm
EOF
cloud-localds "$seed" "$work/user-data" "$work/meta-data"
if [[ -e /dev/kvm && -r /dev/kvm && -w /dev/kvm ]]; then
  accel=kvm
  accel_args=(-accel kvm)
else
  accel=tcg
  accel_args=(-accel tcg,thread=multi)
fi
start=$(date +%s%3N)
qemu-system-x86_64 "${accel_args[@]}" -cpu max -m 1536 -smp 2 \
  -display none -serial "file:$PWD/evidence/vm-console.log" -daemonize -pidfile "$pidfile" \
  -drive "file=$overlay,if=virtio,format=qcow2" \
  -drive "file=$seed,if=virtio,media=cdrom,readonly=on" \
  -netdev user,id=net0,hostfwd=tcp:127.0.0.1:18080-:8000 -device virtio-net-pci,netdev=net0
ready=0
for _ in $(seq 1 180); do
  if curl -fsS http://127.0.0.1:18080/health >/dev/null; then ready=1; break; fi
  sleep 2
done
end=$(date +%s%3N)
if [[ $ready -ne 1 ]]; then
  echo "VM result: UNAVAILABLE; QEMU mode=$accel; guest did not become healthy within 360s." | tee evidence/vm-benchmark.txt
  grep -E 'Started vm-portal|Failed to start|cloud-init.*finished|login:' evidence/vm-console.log | tail -n 12 >> evidence/vm-benchmark.txt || true
  exit 0
fi
startup=$(awk -v s="$start" -v e="$end" 'BEGIN {printf "%.3f",(e-s)/1000}')
printf 'mode=%s\nstartup_seconds=%s\nassigned_ram_mib=1536\n' "$accel" "$startup" | tee evidence/vm-benchmark.txt
ab -n 2000 -c 25 -k http://127.0.0.1:18080/health > evidence/benchmark-vm.txt
qemu_pid=$(cat "$pidfile")
vm_rss_kib=$(ps -o rss= -p "$qemu_pid" | tr -d ' ')
vm_rss_mib=$(awk -v x="${vm_rss_kib:-0}" 'BEGIN {printf "%.1f",x/1024}')
vm_size_mib=$(awk -v a="$(stat -c%s "$image")" -v b="$(stat -c%s "$overlay")" 'BEGIN {printf "%.1f",(a+b)/1048576}')
python3 - evidence/benchmark-vm.txt "$startup" "$vm_rss_mib" "$vm_size_mib" "$accel" >> evidence/benchmark.csv <<'PY'
import re,sys
text=open(sys.argv[1]).read()
rps=re.search(r"Requests per second:\s*([0-9.]+)",text)
lat=re.search(r"Time per request:\s*([0-9.]+)\s+\[ms\] \(mean\)",text)
print(f"vm,{sys.argv[2]},{sys.argv[3]},{sys.argv[4]},{rps.group(1) if rps else ''},{lat.group(1) if lat else ''},QEMU-{sys.argv[5]} guest 1536 MiB")
PY
```

### `TaskA2/ansible/requirements.yml`

```yaml
---
collections:
  - name: community.general
    version: "10.3.0"
```

### `TaskA2/ansible/site.yml`

```yaml
---
- name: Configure result portal nodes
  hosts: portal
  become: true
  serial: 5
  vars:
    portal_user: result-portal
  tasks:
    - name: Install portal operating packages
      ansible.builtin.apt:
        name: [nginx, python3, python3-venv, ufw, rsyslog]
        state: present
        update_cache: true
    - name: Create non-login service identity
      ansible.builtin.user:
        name: "{{ portal_user }}"
        system: true
        create_home: false
        shell: /usr/sbin/nologin
    - name: Create application directory
      ansible.builtin.file:
        path: /opt/result-portal
        state: directory
        owner: "{{ portal_user }}"
        group: "{{ portal_user }}"
        mode: "0750"
    - name: Copy portal Python source and requirements
      ansible.builtin.copy:
        src: "{{ item.src }}"
        dest: "/opt/result-portal/{{ item.dest }}"
        owner: "{{ portal_user }}"
        group: "{{ portal_user }}"
        mode: "0640"
      loop:
        - { src: "../../webapp/app.py", dest: "app.py" }
        - { src: "../../webapp/requirements.txt", dest: "requirements.txt" }
    - name: Create Python virtual environment
      ansible.builtin.command:
        cmd: python3 -m venv /opt/result-portal/.venv
        creates: /opt/result-portal/.venv/bin/python
    - name: Install portal dependencies
      ansible.builtin.pip:
        requirements: /opt/result-portal/requirements.txt
        virtualenv: /opt/result-portal/.venv
    - name: Install portal systemd unit
      ansible.builtin.copy:
        src: ../../TaskB1/result-portal.service
        dest: /etc/systemd/system/result-portal.service
        owner: root
        group: root
        mode: "0644"
      notify: Restart portal
    - name: Install Nginx reverse proxy site
      ansible.builtin.copy:
        src: "../../TaskB1/nginx-result-portal.conf"
        dest: /etc/nginx/sites-available/result-portal
        owner: root
        group: root
        mode: "0644"
      notify: Restart Nginx
    - name: Enable portal Nginx site and disable default
      ansible.builtin.file:
        src: /etc/nginx/sites-available/result-portal
        dest: /etc/nginx/sites-enabled/result-portal
        state: link
        force: true
    - name: Remove default Nginx site
      ansible.builtin.file:
        path: /etc/nginx/sites-enabled/default
        state: absent
    - name: Ensure services are enabled
      ansible.builtin.systemd:
        name: "{{ item }}"
        enabled: true
        state: started
        daemon_reload: true
      loop: [nginx, result-portal]
    - name: Set restrictive incoming firewall policy
      community.general.ufw:
        direction: incoming
        policy: deny
    - name: Set outgoing firewall policy
      community.general.ufw:
        direction: outgoing
        policy: allow
    - name: Allow SSH and web entry points
      community.general.ufw:
        rule: allow
        port: "{{ item }}"
        proto: tcp
      loop: ["22", "80", "443"]
    - name: Enable host firewall
      community.general.ufw:
        state: enabled
  handlers:
    - name: Restart portal
      ansible.builtin.systemd:
        name: result-portal
        state: restarted
    - name: Restart Nginx
      ansible.builtin.systemd:
        name: nginx
        state: restarted
```

### `TaskA5/deployment.yaml`

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: result-portal
spec:
  replicas: 3
  selector:
    matchLabels:
      app: result-portal
  template:
    metadata:
      labels:
        app: result-portal
    spec:
      containers:
        - name: portal
          image: simats/result-portal:1.0
          ports:
            - containerPort: 8000
          resources:
            requests:
              cpu: 250m
              memory: 256Mi
            limits:
              cpu: 500m
              memory: 512Mi
          readinessProbe:
            httpGet:
              path: /health
              port: 8000
            initialDelaySeconds: 2
            periodSeconds: 5
---
apiVersion: v1
kind: Service
metadata:
  name: result-portal
spec:
  type: LoadBalancer
  selector:
    app: result-portal
  ports:
    - port: 80
      targetPort: 8000
```

### `TaskA5/hpa.yaml`

```yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: result-portal
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: result-portal
  minReplicas: 3
  maxReplicas: 50
  metrics:
    - type: Resource
      resource:
        name: cpu
        target:
          type: Utilization
          averageUtilization: 70
  behavior:
    scaleUp:
      stabilizationWindowSeconds: 0
      policies:
        - type: Pods
          value: 10
          periodSeconds: 30
    scaleDown:
      stabilizationWindowSeconds: 300
      policies:
        - type: Percent
          value: 25
          periodSeconds: 60
```

### `TaskB1/disk-alert.sh`

```bash
#!/usr/bin/env bash
# Report a warning for any mounted filesystem above ALERT_THRESHOLD (default 80%).
set -euo pipefail
threshold=${ALERT_THRESHOLD:-80}
df -P -x tmpfs -x devtmpfs | awk -v limit="$threshold" 'NR > 1 {used=$5; gsub(/%/, "", used); if (used+0 >= limit) printf "ALERT filesystem=%s used=%s%% mount=%s\n", $1, used, $6; else printf "OK filesystem=%s used=%s%% mount=%s\n", $1, used, $6}'
```

### `TaskB1/log_summary.sh`

```bash
#!/usr/bin/env bash
# Completed worksheet TODOs; all non-TODO worksheet lines remain unchanged.
set -euo pipefail
log=$(mktemp)
cat > "$log" <<'LOG'
10.0.0.5 - - [08/Oct/2026:10:01:02] "GET /index.html" 200 512
10.0.0.7 - - [08/Oct/2026:10:01:05] "GET /login" 200 1024
10.0.0.5 - - [08/Oct/2026:10:01:09] "POST /login" 302 0
10.0.0.9 - - [08/Oct/2026:10:02:11] "GET /admin" 403 128
10.0.0.5 - - [08/Oct/2026:10:02:15] "GET /report.pdf" 200 40960
10.0.0.7 - - [08/Oct/2026:10:03:01] "GET /missing" 404 64
10.0.0.5 - - [08/Oct/2026:10:03:30] "GET /index.html" 200 512
LOG
echo "Requests per status code:"
awk '{print $7}' "$log" | sort | uniq -c | awk '{printf "  %s %d\n", $2, $1}'
echo "Busiest client:"
awk '{print $1}' "$log" | sort | uniq -c | sort -k1,1nr -k2,2 | awk 'NR == 1 {printf "  %s %d\n", $2, $1}'
echo "Total bytes sent: $(awk '{s += $8} END {print s}' "$log")"
rm -f "$log"
```

### `TaskB1/log_summary_corrected.sh`

```bash
#!/usr/bin/env bash
# Independently written correct field parser for the provided sample log.
set -euo pipefail
log=$(mktemp)
trap 'rm -f "$log"' EXIT
cat > "$log" <<'LOG'
10.0.0.5 - - [08/Oct/2026:10:01:02] "GET /index.html" 200 512
10.0.0.7 - - [08/Oct/2026:10:01:05] "GET /login" 200 1024
10.0.0.5 - - [08/Oct/2026:10:01:09] "POST /login" 302 0
10.0.0.9 - - [08/Oct/2026:10:02:11] "GET /admin" 403 128
10.0.0.5 - - [08/Oct/2026:10:02:15] "GET /report.pdf" 200 40960
10.0.0.7 - - [08/Oct/2026:10:03:01] "GET /missing" 404 64
10.0.0.5 - - [08/Oct/2026:10:03:30] "GET /index.html" 200 512
LOG
echo "Requests per status code:"
awk '{print $7}' "$log" | sort | uniq -c | awk '{printf "  %s %d\n", $2, $1}'
echo "Busiest client:"
awk '{print $1}' "$log" | sort | uniq -c | sort -k1,1nr -k2,2 | awk 'NR == 1 {printf "  %s %d\n", $2, $1}'
echo "Total bytes sent: $(awk '{s += $8} END {print s+0}' "$log")"
```

### `TaskB1/nginx-result-portal.conf`

```ini
server {
    listen 80 default_server;
    server_name _;
    access_log /var/log/nginx/result-portal.access.log;
    error_log /var/log/nginx/result-portal.error.log warn;
    location / {
        proxy_set_header Host $host;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_connect_timeout 2s;
        proxy_read_timeout 10s;
        proxy_pass http://127.0.0.1:8000;
    }
}
```

### `TaskB1/result-portal.service`

```ini
[Unit]
Description=University result portal sample web application
After=network.target

[Service]
Type=simple
User=result-portal
Group=result-portal
WorkingDirectory=/opt/result-portal
Environment="APP_NAME=University Result Portal"
Environment=PORT=8000
ExecStart=/opt/result-portal/.venv/bin/gunicorn --workers 2 --bind 127.0.0.1:8000 --access-logfile - --error-logfile - app:app
Restart=on-failure
RestartSec=2
UMask=0027
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ProtectKernelTunables=true
ProtectControlGroups=true
RestrictSUIDSGID=true
LockPersonality=true
CapabilityBoundingSet=

[Install]
WantedBy=multi-user.target
```

### `TaskB1/setup-service.sh`

```bash
#!/usr/bin/env bash
set -euo pipefail
repo_root=$(cd "$(dirname "$0")/.." && pwd)
if [[ $(id -u) -ne 0 ]]; then echo 'Run with sudo.' >&2; exit 1; fi
id result-portal >/dev/null 2>&1 || useradd --system --home-dir /opt/result-portal --shell /usr/sbin/nologin result-portal
install -d -o result-portal -g result-portal -m 0750 /opt/result-portal
install -o result-portal -g result-portal -m 0640 "$repo_root/webapp/app.py" /opt/result-portal/app.py
install -o result-portal -g result-portal -m 0640 "$repo_root/webapp/requirements.txt" /opt/result-portal/requirements.txt
python3 -m venv /opt/result-portal/.venv
/opt/result-portal/.venv/bin/pip install --disable-pip-version-check -r /opt/result-portal/requirements.txt
install -m 0644 "$repo_root/TaskB1/result-portal.service" /etc/systemd/system/result-portal.service
install -m 0644 "$repo_root/TaskB1/nginx-result-portal.conf" /etc/nginx/sites-available/result-portal
ln -sfn /etc/nginx/sites-available/result-portal /etc/nginx/sites-enabled/result-portal
rm -f /etc/nginx/sites-enabled/default
install -m 0755 "$repo_root/TaskB1/disk-alert.sh" /usr/local/sbin/disk-alert.sh
install -o result-portal -g result-portal -m 0640 /dev/null /var/log/disk-alert.log
systemctl daemon-reload
systemctl enable --now result-portal
nginx -t
systemctl enable --now nginx
```

### `TaskB2/compose.yaml`

```yaml
services:
  portal:
    build:
      context: ..
      dockerfile: TaskB2/Dockerfile
    image: result-portal:compose
    environment:
      APP_NAME: University Result Portal
      DATABASE_URL: postgresql://portal:local-demo-only@database:5432/results
    ports:
      - "8000:8000"
    cpus: 0.50
    mem_limit: 512m
    depends_on:
      database:
        condition: service_healthy
    restart: unless-stopped
  database:
    image: postgres:16-alpine
    environment:
      POSTGRES_DB: results
      POSTGRES_USER: portal
      POSTGRES_PASSWORD: local-demo-only
    volumes:
      - result-db:/var/lib/postgresql/data
      - ./init-db.sql:/docker-entrypoint-initdb.d/01-demo.sql:ro
    cpus: 0.50
    mem_limit: 512m
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U portal -d results"]
      interval: 5s
      timeout: 3s
      retries: 12
    restart: unless-stopped
  lms:
    build:
      context: ..
      dockerfile: TaskB2/Dockerfile
    image: result-portal:compose
    environment:
      APP_NAME: Learning Management Service
    command: ["gunicorn", "--bind", "0.0.0.0:8000", "--workers", "2", "--access-logfile", "-", "--error-logfile", "-", "app:app"]
    ports:
      - "8001:8000"
    cpus: 0.50
    mem_limit: 512m
    restart: unless-stopped
volumes:
  result-db:
```

### `TaskB2/Dockerfile`

```dockerfile
# Container image for the campus result-portal web service (a small Flask app).
FROM python:3.12-slim
WORKDIR /app
# Install the dependencies first so that this layer is cached between builds.
COPY webapp/requirements.txt ./requirements.txt
RUN pip install --no-cache-dir -r requirements.txt \
    && groupadd --system portal \
    && useradd --system --gid portal --home-dir /app --shell /usr/sbin/nologin portal
COPY webapp/app.py ./app.py
RUN chown -R portal:portal /app
USER portal
EXPOSE 8000
CMD ["gunicorn", "--bind", "0.0.0.0:8000", "--workers", "2", "--access-logfile", "-", "--error-logfile", "-", "app:app"]
```

### `TaskB2/init-db.sql`

```sql
CREATE TABLE IF NOT EXISTS demo_results (
  student_id INTEGER PRIMARY KEY,
  semester TEXT NOT NULL,
  grade TEXT NOT NULL
);
INSERT INTO demo_results (student_id, semester, grade)
VALUES (10001, '2025-odd', 'A')
ON CONFLICT (student_id) DO NOTHING;
```

### `TaskB3/deployment.yaml`

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: result-portal
spec:
  replicas: 3
  selector:
    matchLabels:
      app: result-portal
  template:
    metadata:
      labels:
        app: result-portal
    spec:
      containers:
        - name: portal
          image: result-portal:compose
          imagePullPolicy: IfNotPresent
          ports:
            - containerPort: 8000
          resources:
            requests:
              cpu: 250m
              memory: 256Mi
            limits:
              cpu: 500m
              memory: 512Mi
          readinessProbe:
            httpGet:
              path: /health
              port: 8000
            initialDelaySeconds: 2
            periodSeconds: 5
---
apiVersion: v1
kind: Service
metadata:
  name: result-portal
spec:
  type: ClusterIP
  selector:
    app: result-portal
  ports:
    - port: 80
      targetPort: 8000
```

### `TaskB3/kind-config.yaml`

```yaml
kind: Cluster
apiVersion: kind.x-k8s.io/v1alpha4
nodes:
  - role: control-plane
  - role: worker
  - role: worker
```

### `TaskB3/load_burst.py`

```python
#!/usr/bin/env python3
"""Small standard-library GET load generator; reports completed request rate."""
import concurrent.futures
import sys
import threading
import time
import urllib.request

url = sys.argv[1]
duration = float(sys.argv[2]) if len(sys.argv) > 2 else 15.0
workers = int(sys.argv[3]) if len(sys.argv) > 3 else 25
stop = threading.Event()
ok = 0
errors = 0
lock = threading.Lock()


def worker():
    global ok, errors
    while not stop.is_set():
        try:
            with urllib.request.urlopen(url, timeout=3) as response:
                response.read()
                with lock:
                    if response.status == 200:
                        ok += 1
                    else:
                        errors += 1
        except Exception:
            with lock:
                errors += 1


start = time.perf_counter()
with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
    futures = [pool.submit(worker) for _ in range(workers)]
    time.sleep(duration)
    stop.set()
    for future in futures:
        future.result()
elapsed = time.perf_counter() - start
print(f"url={url} duration_s={elapsed:.3f} concurrency={workers} ok={ok} errors={errors} requests_per_second={ok/elapsed:.2f}")
if errors:
    raise SystemExit(1)
```

### `TaskB4/additional-service.yaml`

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: lms
spec:
  replicas: 2
  selector:
    matchLabels:
      app: lms
  template:
    metadata:
      labels:
        app: lms
    spec:
      containers:
        - name: lms
          image: result-portal:compose
          imagePullPolicy: IfNotPresent
          env:
            - name: APP_NAME
              value: Learning Management Service
          ports:
            - containerPort: 8000
          resources:
            requests:
              cpu: 200m
              memory: 128Mi
            limits:
              cpu: 400m
              memory: 256Mi
          readinessProbe:
            httpGet:
              path: /health
              port: 8000
---
apiVersion: v1
kind: Service
metadata:
  name: lms
spec:
  selector:
    app: lms
  ports:
    - port: 80
      targetPort: 8000
```

### `TaskB4/capacity_estimate.py`

```python
#!/usr/bin/env python3
"""Capacity lower-bound and annual EC2 compute-only sensitivity model."""
import csv
import math
import sys
from pathlib import Path

source = Path(sys.argv[1] if len(sys.argv) > 1 else "evidence/benchmark.csv")
rates = {}
with source.open(newline="", encoding="utf-8") as f:
    for row in csv.DictReader(f):
        if row["requests_per_second"]:
            rates[row["deployment"]] = float(row["requests_per_second"])

students, event_seconds, flash_factor, target_util = 150_000, 7_200, 5, 0.70
rate = rates.get("container", 0)
if rate <= 0:
    raise SystemExit("Missing positive measured container requests_per_second in benchmark.csv")
replica_scenarios = {}
for per_student in (1, 5, 10):
    avg = students * per_student / event_seconds
    peak = avg * flash_factor
    required = math.ceil(peak / (rate * target_util))
    replica_scenarios[per_student] = required
    print(f"requests_per_student={per_student}; average_rps={avg:.2f}; modeled_5x_peak_rps={peak:.2f}; required_at_70pct={required}")
required = replica_scenarios[10]
baseline = 3
nodes_fixed = max(baseline, required)
burst_hours_per_year = 2 * 4
usd_per_hour = 0.0835
fixed_peak_fleet_usd = nodes_fixed * 8760 * usd_per_hour
burst_compute_usd = max(0, required - baseline) * burst_hours_per_year * usd_per_hour
cloud_only_usd = baseline * 8760 * usd_per_hour + burst_compute_usd
hybrid_incremental_usd = burst_compute_usd

print("Input: 150,000 students / 2 hours; 1/5/10 HTTP requests per student; modeled 5x peak; 70% target utilization")
print(f"measured_container_rps={rate:.2f}; conservative_10_request_replica_count=ceil(peak/(measured_rps*0.70))={required}")
print(f"three-replica_safety_floor={baseline}; fixed_instances={nodes_fixed}")
print(f"fixed_peak_fleet_cloud_compute_only_usd_year={fixed_peak_fleet_usd:.2f}")
print(f"cloud_only_3_vm_baseline_plus_{burst_hours_per_year}h_burst_compute_only_usd_year={cloud_only_usd:.2f}")
print(f"hybrid_incremental_cloud_burst_compute_only_usd_year={hybrid_incremental_usd:.2f}; existing campus baseline cost excluded")
print("Excludes database, cluster control plane, load balancer, disks, backups, egress, taxes, support and campus-host cost. Synthetic password-hash throughput is an optimistic lower bound for database-backed result lookup; load-test the real query before production sizing.")
```

### `webapp/app.py`

```python
"""Small stateless portal used for reproducible deployment measurements."""
from flask import Flask, jsonify
import hashlib
import os
from urllib.parse import urlparse

app = Flask(__name__)


@app.get("/")
def index():
    return f"{os.getenv('APP_NAME', 'University Result Portal')} is ready.\n"


@app.get("/health")
def health():
    return jsonify(status="ok", service=os.getenv("APP_NAME", "result-portal"))


@app.get("/simulate/login")
def simulate_login():
    # Deterministic CPU work approximates a small password-hash cost without
    # storing credentials or any real student data in the assignment.
    digest = hashlib.pbkdf2_hmac("sha256", b"synthetic-password", b"assignment5-salt", 10_000)
    return jsonify(authenticated=bool(digest), service=os.getenv("APP_NAME", "result-portal"))


@app.get("/results/<int:student_id>")
def result(student_id):
    # Fixed mock result; deliberately no real student data is used in this lab.
    database_url = os.getenv("DATABASE_URL")
    if database_url:
        import psycopg2
        from psycopg2.extras import RealDictCursor
        parsed = urlparse(database_url)
        with psycopg2.connect(
            host=parsed.hostname, port=parsed.port or 5432,
            dbname=parsed.path.lstrip("/"), user=parsed.username,
            password=parsed.password, connect_timeout=2,
        ) as connection:
            with connection.cursor(cursor_factory=RealDictCursor) as cursor:
                cursor.execute("SELECT student_id, semester, grade FROM demo_results WHERE student_id = %s", (student_id,))
                row = cursor.fetchone()
        if row is None:
            return jsonify(error="not found"), 404
        return jsonify(dict(row) | {"service": os.getenv("APP_NAME", "result-portal")})
    return jsonify(student_id=student_id, semester="2025-odd", grade="A", service=os.getenv("APP_NAME", "result-portal"))


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=int(os.getenv("PORT", "8000")))
```
