# CSA0406 Operating Systems - Assignment 4 (CO4)

## Cover

**Assignment:** Optimise File Storage and Disk Access  
**Student:** Mohamed Aslam H  
**Register number:** 192572147  
**Department:** CSE (AI)  
**Academic year:** 2025  
**Faculty:** Dr. Jayasakthi Velmurugan K  
**Nature:** Individual  
**Course repository:** https://github.com/Md-Aslam-31/CSA0406-Operating_System/tree/main/Assignment4  
**Execution environment:** Ubuntu 24.04 LTS and GCC 13 on GitHub Actions. This folder is in the existing course repository to keep Assignments 1-4 together. Run details and outputs are in evidence/.

## Executive summary

Use per-student directories, explicit ACLs and audited group shares instead of a flat unprotected folder. For surveillance files, use indexed/extent allocation so time offsets remain addressable while files grow, and keep a 30-day retention job. ext4 is a sound Linux database default; select other filesystems for device compatibility, snapshots or end-to-end checksums. The provided allocation exercise has a mismatch: its expected output says the 10-block contiguous allocation succeeds at block 19, while a following sentence claims it cannot fit. The code and output agree: 13 adjacent blocks remain there, so it does fit. I retain the expected output and add a separate fragmented example where indexed allocation solves a real contiguous-space failure. For scheduling, compare seek movement and queue wait under a stated deterministic model. For 200,000 stream viewers, a CDN edge cache is needed to meet aggregate bandwidth; keep replicated origins and SSD hot data behind it.

## T1 - File-system design review (6 marks)

The proposal has at least six weaknesses: user-chosen names collide; a flat namespace is harder to delegate and may need linear search O(n); contiguous extents can fail growth and fragment free space; no ACL permits cross-student disclosure; backups/recovery/retention are unspecified; and sharing through copies creates stale versions. A directory index can improve name lookup toward O(log n) or hashed expected O(1), but does not fix privacy or ownership.

The design in Task01/design.md uses /srv/student-files/<register-prefix>/<register-number>/... for private data and /groups/<course>/<project>/ for collaboration. Register number plus server-generated object ID supplies stable identity; original names remain metadata. Private directories default to mode 0700; staff and group membership use scoped ACLs, audited grants, quotas, timestamps, checksums, MIME type and retention tags. Acyclic tree directories preserve simple traversal; project ACLs grant same-object sharing. Encrypted immutable backups live in a separate failure domain and restores are tested.

### Privacy defence (under 100 words)

Every personal directory is owned by its student and has private default permissions; the service checks identity and ACLs on every open. Staff receive course-scoped access only when required and access is audited. A project group gets a dedicated shared directory with an explicit membership ACL, so collaborators edit the same current files without exposing personal folders. Backups preserve permissions and are encrypted. Filenames do not grant access, and changing a link cannot bypass server-side authorization.

### Alternative: managed cloud drive

Use institution-managed accounts, SSO/MFA, group drives, versioning, retention, audit logs and geographic/legal controls. An indexed name lookup is typically O(log N) or a key lookup O(1) average, then opening adds network/authentication latency; caching can improve repeat reads. Space grows O(D+V) with live bytes D and retained versions V; provider replication is hidden in the service charge, while explicit offline copies use additional storage. The college controls identity policy and contracts but delegates disk placement, repair and filesystem internals to the provider. Measure search/open p95 across campus and home networks and verify export and recovery before migration.

## T2 - Allocation for the video recorder (8 marks)

### Operations and choice

32 cameras create one append-only growing file per camera, sequential writes continuously, sequential playback/export, occasional indexed seeks by timestamp, and deletion/metadata updates after 30 days. A time-to-file index maps camera+timestamp to byte offset. Extent/indexed allocation supports growth and random offset lookup; preallocate near-term extents to limit fragmentation, keep an index/extent map, and use checksums. A retention service deletes whole expired files, not arbitrary blocks within active recordings.

| Method | Sequential and direct reads | Growth and fragmentation | Reliability |
|---|---|---|---|
| Contiguous/extents | Very fast sequential; direct block = start + offset/block size, O(1) | File extension needs adjacent space or extent tree; external free-space holes can prevent growth | Few metadata pointers; a corrupt extent loses affected range |
| Linked blocks | Sequential follows pointers; block k needs k link reads; direct access is O(k) | Easy append and no external fragmentation; pointer stored with each block | A damaged pointer can lose the rest of the chain |
| FAT-linked | Sequential follows FAT entries; if FAT is resident, k steps are RAM lookups; arbitrary block still O(k) | Easy growth, but one huge central table consumes RAM and is a metadata hotspot | FAT corruption can disconnect many chains; keep redundant FAT/checks |
| Indexed/inode extents | Sequential and direct access are efficient; with an indexed tree, lookup is O(log n) metadata or a few levels | Easy non-contiguous growth; tree/index blocks add metadata | Checksummed/redundant metadata can limit damage; more indirection than contiguous |

### 4 GiB file cost and inode maximum

Assume binary 4 GiB file = 4,294,967,296 bytes, 4 KiB = 4,096-byte blocks and exclude directory/inode lookup from the comparison. The file has 1,048,576 blocks.

* Contiguous: calculate the final block address; 1 target data-block read.
* On-disk linked pointers: follow preceding links then read data, about 1,048,576 block reads in the worst case.
* FAT held fully in RAM: 1,048,575 RAM next-pointer lookups, then 1 target data read. If FAT is not cached, FAT block reads also occur.
* Indexed tree with 1,024 four-byte pointers per 4 KiB index block: use a root plus one leaf for this 1,048,576-block file, then read the target data block: 2 index reads + 1 data read on a cold path. A cached root or leaf reduces the count.

For an inode with 12 direct, 1 single, 1 double and 1 triple indirect pointer, a 4 KiB pointer block stores 4,096/4 = 1,024 addresses. Maximum addressable blocks = 12 + 1,024 + 1,024^2 + 1,024^3 = 1,074,791,436. At 4 KiB/block that is 4,402,345,721,856 bytes, about 4.004 TiB. This is a pointer-format limit before filesystem, device, reserved-block and implementation limits.

For a decimal 4 TB disk with 4 KiB blocks, there are 976,562,500 whole blocks. A bit vector needs ceil(blocks/8) = 122,070,313 bytes (116.42 MiB). A linked free list uses a pointer per free block but discovers free blocks by traversal; grouping stores batches of block addresses per free block; counting stores start+length for runs and is compact for contiguous free ranges. A FAT with one 4-byte entry per block would require about 3.64 GiB resident, illustrating why a global full FAT is costly at this scale. Task02/allocation_lab.c prints these values.

### Completed allocation code and consistency note

Task02/allocation_todo.c replaces only the requested TODO, uses first fit and matches the supplied expected output. After releasing B and D, the disk has 21 free blocks, including a 13-block run at 19-31; therefore alloc_contiguous('E',10) succeeds at block 19. This contradicts the assignment sentence claiming it cannot be stored contiguously. The later indexed allocation is a second E allocation: after the first E uses 19-28, exactly 11 free blocks remain (8 at 6-13 and 3 at 29-31), enough for one index block plus ten data blocks. Task02/fragmentation_demo.c separately constructs 17 free blocks with largest run 7; a 10-block contiguous file fails but indexed allocation succeeds. This demonstrates the intended fragmentation concept without altering the handout program.

### Alternative: in-memory FAT

With a FAT resident, traversal to k still takes O(k) RAM table lookups and one final data read; an inode extent/index tree takes O(log n) or a few index reads. FAT memory for 4 TB decimal/4 KiB blocks is around 3.64 GiB for 4-byte entries. A damaged FAT can break many unrelated files, so checksummed redundant copies and repair logs are necessary. Contiguous access is O(1) address arithmetic but has growth/fragmentation costs.

## T3 - Modern file systems (9 marks)

Limits vary with format versions, cluster/block sizes, OS support and hardware. The table gives common/practical figures or explains when a universal figure is misleading.

| File system | File / volume scale | Journal or consistency | Snapshots | Checksums | CoW |
|---|---|---|---|---|---|
| FAT32 | File <4 GiB; common Windows volume limit 2 TiB (format/OS dependent) | No journal; power loss can leave allocation chains inconsistent | No | No end-to-end data checksums | No |
| NTFS | Modern Windows default 4 KiB cluster: up to 16 TB; larger clusters support much larger volumes/files (Windows-version dependent) | Metadata journal via NTFS log | VSS/Windows snapshots are external services, not general filesystem snapshots | No general end-to-end file-data checksum | No for ordinary data; metadata updates logged |
| ext4 | 4 KiB blocks: max extent file 16 TiB; 32-bit filesystem ceiling 16 TiB, 64-bit format theoretical ceiling 64 ZiB | JBD2 metadata journal; journal, ordered (default) and writeback modes | External LVM/storage snapshot | Metadata/journal checksums supported; file data not generally checksummed by default | No general data CoW |
| XFS | Very large, commonly up to 8 EiB theoretical file/filesystem ceilings; kernel/device formats bound real use | Metadata log/journal; recovery replays log | External LVM/storage snapshot; reflink shares extents but is not a full snapshot service | V5 metadata checksums | Reflink/CoW for shared extents; ordinary overwrite behavior depends on feature/use |
| Btrfs | 16 EiB format file limit; Linux VFS practical file limit 8 EiB; filesystem is device/capacity bound | Transactional CoW consistency rather than a traditional metadata journal | Native subvolume snapshots | Data and metadata checksums by default | Yes |
| ZFS | 16 EiB theoretical file limit; pool limits are implementation/platform dependent | Transactional CoW tree; no journal replay required for consistency | Native dataset snapshots/clones | End-to-end block checksums; can self-heal with redundancy | Yes |
| APFS | Published Apple guide: file size up to 2^63 bytes (8 EiB); usable container is hardware/OS bounded | CoW metadata and atomic updates | Native snapshots | Public user documentation does not promise general end-to-end data checksums | Yes, metadata and clones/snapshots |

### Power loss and ext4 modes

A partially written block may contain old/new mixed data if a device tears writes; filesystem metadata can otherwise point to incomplete allocations. A journal writes a transaction record and replays committed metadata after reboot. data=journal writes data and metadata through the journal (extra writes, strongest data ordering). data=ordered journals metadata and flushes associated data blocks before committing metadata; it is the usual default. data=writeback journals metadata but does not preserve data-before-metadata ordering, so recently written files can expose stale contents after a crash. Journaling helps metadata consistency; it is not a backup or a guarantee that an application-level multi-file transaction is atomic. CoW filesystems write new blocks and switch a root/pointer atomically, preserving an older tree until commit. End-to-end checksums detect corruption; redundancy may repair it.

### Device choices

* **Windows and Linux pen drive:** exFAT for large cross-platform files and broad modern OS support; FAT32 only when legacy device compatibility matters and files stay below 4 GiB. Safely eject and keep a second copy because neither is a backup.
* **Linux database server:** ext4 with barriers/flush semantics and monitored backups is a mature, low-overhead default. Benchmark XFS for very large parallel files. Evaluate Btrfs/ZFS if snapshots and end-to-end integrity are required, after database direct-I/O/fsync compatibility tests.
* **Laptop:** NTFS for a Windows-only laptop; APFS for a Mac. Use full-disk encryption and versioned backups.
* **Snapshot backup server:** ZFS with redundant vdevs, scheduled snapshots, scrubs and off-site replication; Btrfs is a Linux alternative. Snapshots on the same pool do not protect against pool/site loss.

### Real filesystem inspection

Task03/inspect_filesystem.sh records stat, df -i, ls -i, findmnt and dumpe2fs -h. GitHub's job filesystem may be an overlay instead of a directly addressable ext4 block device; the script reports its host mount and creates a 64 MiB ext4 image to inspect block size, inode/free-space counts and features with dumpe2fs. This provides genuine Linux ext4 superblock evidence without pretending the runner root is a physical ext4 disk. On a directly accessible ext4 host, it also attempts tune2fs -l.

### Alternative: Btrfs/ZFS for database

CoW small random overwrites allocate new blocks and update a logarithmic tree path (O(log N) metadata writes by tree height), which can add write amplification and fragmentation compared with ext4 in-place data updates plus a journal transaction. Checksums consume metadata/CPU and snapshots retain changed blocks: if fraction f of a dataset changes between snapshots, incremental retained payload is roughly f times dataset size (plus metadata), not a full copy; long retention or high churn can approach multiple dataset copies. A snapshot root is created in O(1) metadata operations. Transactional tree recovery avoids a long metadata journal replay, but application recovery and database WAL replay still take time. Benchmark transaction latency, fsync, discard, compression and snapshot churn using the actual database; keep backups separate.

## T4 - Disk scheduling (7 marks)

For cylinders 0-199, head=53, queue 98,183,37,122,14,124,65,67. Initial direction for SCAN/LOOK is toward 0; for C-SCAN/C-LOOK it is toward 199. Count physical movement, including the C-SCAN/C-LOOK wrap.

| Policy | Service order (edge/wrap shown) | Movement |
|---|---|---:|
| FCFS | 98,183,37,122,14,124,65,67 | 640 |
| SSTF | 65,67,37,14,98,122,124,183 | 236 |
| SCAN | 37,14,0,65,67,98,122,124,183 | 236 |
| C-SCAN | 65,67,98,122,124,183,199,0,14,37 | 382 (=183 service travel +199 wrap) |
| LOOK | 37,14,65,67,98,122,124,183 | 208 |
| C-LOOK | 65,67,98,122,124,183,14,37 | 322 (includes 183-to-14 wrap) |

The handout's completed SSTF program prints 236. Its C-SCAN expected output displays 183 cylinders plus the 199-cylinder wrap separately; total physical movement is 382.

Task04/scheduler_sim.c simulates 10,000 seeded arrivals on cylinders 0-199, with light/medium/heavy arrival intervals 30/10/3 ms. The explicit model uses 0.08 ms per cylinder and 2 ms service per request. It reports average head movement, average queue wait and maximum queue wait for all six policies. These are deterministic model measurements, not measurements of a physical disk. FCFS is fair in arrival order but can have long seeks. SSTF minimizes immediate distance but can starve distant requests under a sustained closer stream. SCAN/LOOK bound directional waits better; LOOK avoids traveling to an unused edge. C-SCAN gives more uniform directional service intervals than SCAN by treating the return as a wrap, at the cost of jump movement. The simulation appends 200 requests at cylinder 60 every millisecond to show SSTF can leave a request at 183 waiting at least 400 ms in its stated 2 ms service model; an unending closer stream has no finite SSTF wait bound.

For SSDs, mechanical seek reordering has little benefit. Linux's none leaves ordering largely to hardware/blk-mq; mq-deadline can enforce latency/fairness without pretending seeks dominate. Task04/inspect_scheduler.sh records lsblk, each available /sys/block/*/queue/scheduler and device rotation flags. The actual hosted runner result is in evidence/task04-device.txt.

![Maximum queue wait by policy and offered load](plots/scheduler_wait.png)

### Alternative: LOOK and C-LOOK complexity

At a batch of n pending requests, a simple scan for the nearest or last request costs O(n); sorting the queue once costs O(n log n), after which selection is O(1) or O(log n) insertion. The given queue: LOOK moves 208 cylinders, C-LOOK moves 322 including wrap. Wait depends on arrival times and service rate, so report queue measurements from the provided simulator rather than infer an average from total seek distance alone. Compared with SCAN/C-SCAN, LOOK/C-LOOK avoid empty travel to physical edges.

## T5 - Distributed and cloud storage (7 marks)

NFS offers a familiar shared filesystem and is simplest for home directories; HA configuration and cache semantics matter. HDFS is optimized for large sequential write-once/read-many files; a common replication factor three costs roughly 3x storage. Ceph distributes object/file/block storage and provides scaling/failure-domain choices but needs operational expertise. Cloud object storage is key/API-based, highly available and elastic, but charges for stored versions, requests, retrieval and egress and is not a POSIX filesystem by itself.

Replication keeps copies on separate nodes/racks. One failed device can be replaced from surviving replicas; copies in the same rack or site do not survive that domain's loss. Strong consistency makes acknowledged writes visible to later reads under the documented scope; eventual consistency allows temporary stale views and requires conflict handling. The OS uses device drivers to translate block requests, buffering to smooth rates, page-cache for repeated reads/writes, I/O schedulers to merge/order queued work, spooling for queued device jobs, and a block-device interface exposing logical block requests. Flushes and write-cache behavior affect durability.

### One NFS server alternative

With N users each demanding R bytes/s and server/link capacity B, ideal per-user throughput is bounded by B/N = O(1/N) after saturation while aggregate throughput remains O(B). One copy costs O(D) and three replicas cost O(3D); actual throughput falls as queues and metadata contention grow. One data copy is storage efficient; RAID/backups cost more but are not automatically independent service replicas. If the single server fails, all clients lose access until repair; clustered HA improves availability but needs shared metadata, fencing and tested failover. For example, a 10 Gbit/s link is 1.25 GB/s before overhead; 1,000 readers at 2 MB/s require 2 GB/s and saturate it. Three distributed replicas use about 3x space but provide parallel reads and node-failure tolerance. A separate immutable/off-site backup is still needed.

## T6 - Streaming startup design (13 marks)

### Stage 1: workload and measurable targets

Uploads occur once per video; reads dominate, most views happen during week one, and a minute seek is a random offset followed by sequential chunk reads. Design scenario: 10,000 videos averaging 1 GiB, 200,000 possible students, 20,000 simultaneous viewers at 4 Mbit/s. Test peak delivery of 20,000 x 4 Mbit/s = 80 Gbit/s; with a measured 95% CDN cache hit ratio, origin egress target is at most 4 Gbit/s. Test p95 first-frame startup under 3 seconds from each campus/region, correct seek to the requested minute, and no lost acknowledged uploads after one node failure. Those targets must be validated with real users and video profiles.

### Stages 2-3: architecture and decisions

Store authoritative video objects in a replicated origin across three failure-separated nodes. Each node has two 16 TB disks mirrored (16 TB decimal usable per node), enough for a 10.74 TB decimal catalog plus staging/headroom; three nodes hold three logical copies. Keep a 3 TiB NVMe/SSD hot cache for videos trending in week one; fill by popularity and evict cold items. Put a CDN in front of origin for campus/home delivery. Use ext4 or XFS on Linux origin nodes with 4 KiB filesystem blocks, extent allocation and 1 MiB HTTP segments. File offset indexes map timestamps to segment offsets. Use none/mq-deadline on NVMe; validate scheduler for the disk. Replication factor 3 across nodes; mirrored disks address a separate local disk failure. Alert on cache hit below 95%, origin saturation, checksum/replica lag and p95 startup.

    Upload -> checksum/metadata validation -> 3 mirrored origin nodes
                                               |
                                        3 TiB SSD hot cache
                                               |
                                   CDN edges in campus regions
                                               |
                                  campus and home viewers

### Stage 4: capacity, bandwidth and seek cost

Logical catalog = 10,000 x 1 GiB = 10,000 GiB = 9.77 TiB (10.74 TB decimal). Three logical copies require 30,000 GiB =29.30 TiB (32.21 TB decimal), excluding filesystem and staging overhead. At 20,000 users x4 Mbit/s, aggregate egress is 80 Gbit/s. A 95% edge hit ratio leaves 4 Gbit/s average origin egress; three 10 GbE origin links have nominal capacity 30 Gbit/s before overhead. The CDN must be provisioned for the full 80 Gbit/s and validated for cache misses and flash crowds.

At 4 Mbit/s, a 1 MiB segment represents about 2.10 seconds of playback. A random time seek calculates the target segment/byte offset, follows the inode extent index (one or a few metadata reads on a cold path), then issues a contiguous segment read: typically 1 data read when metadata is cached, or about 2-3 disk reads cold depending on extent-tree depth. A 2-hour video has about 3,600/2.10 =1,714 such segments. A 4 KiB allocation block means each 1 MiB segment covers 256 filesystem blocks.

For a cloud-only alternative, assumed (not vendor quote) catalog storage at $0.023/GiB-month costs 10,000 x 0.023 = $230/month. If each of 200,000 users watches two hours/month at 4 Mbit/s, delivery is 200,000 x 3.6 decimal GB = 720,000 GB; at an illustrative $0.05/GB egress, transfer is $36,000/month before CDN discounts, requests, cache tiers, taxes and regional pricing. Replace both unit rates with a current quote for the deployment region. Near-edge cache lowers start latency across cities; cloud/CDN reduces hardware operations but shifts cost to egress/requests and requires configuration, access control, cache invalidation and vendor governance.

### Stage 5: two-design simulation

Task06/streaming_sim.c compares a four-HDD origin directly serving a burst of 100,000 random 1 MiB segments at 2,000 starts/s against a modeled 95%-hit SSD/CDN edge with the same HDD origin. The simulation uses 8 ms seek + 180 MiB/s HDD transfer, 0.15 ms SSD seek + 1,800 MiB/s transfer, and 20 ms edge RTT. It reports p50/p95/p99/max request-start latency and origin reads; results are in evidence/task06-simulation.txt and charted in plots/streaming_latency.png. It measures the stated queue model, not a physical benchmark. A remote city should use a nearby CDN edge: with a modeled 20 ms RTT and 1 MiB buffer, the first-buffer transfer bound at 4 Mbit/s is about 2.1 seconds; a distant user without an edge adds WAN RTT and cannot be promised the same p95. Real evaluation should replay a representative popularity trace against fio or production-like storage and measure CDN hit ratio/RTT from multiple campuses.

![Startup latency for the two modeled designs](plots/streaming_latency.png)

## Reproduction and evidence

Run all experiments on Ubuntu with GCC:

    bash scripts/run_linux_evidence.sh

The runner compiles every C program with gcc -std=c11 -Wall -Wextra -pthread, captures output under evidence/, generates figures under plots/, and checks key expected output. The GitHub Actions link is in Assignment4/README.md. All algorithms have source, deterministic inputs and results retained. Values that depend on filesystem, kernel, queue parameters or assumed prices are labeled as such.

### References

* Microsoft Learn, [NTFS overview](https://learn.microsoft.com/windows-server/storage/file-server/ntfs-overview) and [filesystem functionality comparison](https://learn.microsoft.com/windows/win32/fileio/filesystem-functionality-comparison).
* Linux kernel, [ext4 documentation](https://www.kernel.org/doc/html/latest/admin-guide/ext4.html) and [XFS documentation](https://docs.kernel.org/next/admin-guide/xfs.html).
* [Btrfs documentation](https://btrfs.readthedocs.io/en/stable/btrfs-man5.html), [OpenZFS copy-on-write](https://openzfs.github.io/openzfs-docs/Basic%20Concepts/Copy-on-write.html), and [Apple APFS overview](https://support.apple.com/en-asia/guide/security/seca6147599e/web) and the [published APFS size table](https://developer.apple.com/library/archive/documentation/FileManagement/Conceptual/APFS_Guide/VolumeFormatComparison/VolumeFormatComparison.html).
* Apache Hadoop, [HDFS architecture guide](https://hadoop.apache.org/docs/r3.4.3/hadoop-project-dist/hadoop-hdfs/HdfsDesign.html); AWS, [S3 consistency model](https://docs.aws.amazon.com/AmazonS3/latest/userguide/Welcome.html).

## OnlineGDB output screenshot

This capture shows `Task02/fragmentation_demo.c` compiled and run in OnlineGDB. It demonstrates the fragmented free-space layout, failure of the contiguous request, successful indexed allocation, and exit code 0.

![OnlineGDB run of Assignment 4 fragmentation demo](evidence/onlinegdb/task02-fragmentation-demo.jpg)
## Full OnlineGDB program runs

These captures show complete C program runs with output and exit status. The task source files in this repository contain the full implementations.

### Task 2 — contiguous and indexed allocation

![Allocation comparison output](evidence/onlinegdb/allocation_lab-full.jpg)

The earlier Task 2 demo above shows the contiguous-allocation failure and indexed-allocation success.

### Task 4 — scheduling simulator

The simulator processed 10,000 seeded requests for the light, medium and heavy arrival loads and reported average movement, average wait and maximum wait for each policy. Its ending output includes the SSTF starvation illustration and exit code 0.

![Scheduler simulation full run output](evidence/onlinegdb/scheduler_sim-full.jpg)

### Task 6 — streaming workload model

The run reports p50/p95/p99/max latencies and origin reads for the local HDD pool and SSD/CDN cache model, plus the deterministic model assumptions and exit code 0.

![Streaming simulation full run output](evidence/onlinegdb/streaming_sim-full.jpg)

## Appendix - Full source code listings

This appendix reproduces all assignment programs, implementation scripts, and deployment/workflow configuration. Evidence screenshots and raw outputs appear earlier in this report and under `evidence/`. The exact source files are also retained at the listed repository paths. Report line wrapping is visual only.

### `scripts/plot_results.py`

```python
from pathlib import Path
import csv
import re
import matplotlib.pyplot as plt

base = Path(__file__).resolve().parent.parent
lines = (base / "evidence/task04-simulation.csv").read_text().splitlines()
header_index = next(i for i, line in enumerate(lines) if line.startswith("load,policy,"))
rows = list(csv.DictReader(lines[header_index:]))
fig, axes = plt.subplots(1, 3, figsize=(10, 3.7), sharey=True)
for ax, load in zip(axes, ["light", "medium", "heavy"]):
    group = [r for r in rows if r["load"] == load]
    ax.bar([r["policy"] for r in group], [float(r["max_wait_ms"]) for r in group], color="#2673b8")
    ax.set_title(load.title() + " load")
    ax.tick_params(axis="x", rotation=45)
    ax.grid(axis="y", alpha=.25)
axes[0].set_ylabel("Maximum queue wait (ms)")
fig.suptitle("Disk scheduling: maximum wait in seeded 10,000-request model")
fig.tight_layout()
fig.savefig(base / "plots/scheduler_wait.png", dpi=170)

text = (base / "evidence/task06-simulation.txt").read_text()
names = ["Local HDD pool (4 disks, no edge cache)", "SSD edge cache plus HDD origin (95% hits)"]
values = []
for name in names:
    m = re.search(re.escape(name) + r":.*?p95=([0-9.]+) ms", text)
    if m:
        values.append(float(m.group(1)))
fig, ax = plt.subplots(figsize=(7, 4))
ax.bar(["Four HDDs\n(no edge cache)", "SSD/CDN cache\n(95% modeled hits)"], values, color=["#b54708", "#18805a"])
ax.set_ylabel("95th percentile startup lookup latency (ms)")
ax.set_title("Streaming start requests in stated queue model")
ax.grid(axis="y", alpha=.25)
fig.tight_layout()
fig.savefig(base / "plots/streaming_latency.png", dpi=170)
```

### `scripts/run_linux_evidence.sh`

```bash
#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence plots
{
  . /etc/os-release
  printf 'Linux: %s\nCompiler: ' "$PRETTY_NAME"
  gcc --version | head -n1
  echo 'Compile flags: gcc -std=c11 -O2 -Wall -Wextra -pthread'
  echo 'Filesystem mount:'
  findmnt -T . -o TARGET,SOURCE,FSTYPE
} | tee evidence/build-environment.txt

for spec in \
  'Task02/allocation_todo.c:Task02/allocation_todo:' \
  'Task02/fragmentation_demo.c:Task02/fragmentation_demo:' \
  'Task02/allocation_lab.c:Task02/allocation_lab:' \
  'Task04/disk_todo.c:Task04/disk_todo:' \
  'Task04/scheduler_sim.c:Task04/scheduler_sim:' \
  'Task06/streaming_sim.c:Task06/streaming_sim:-lm'; do
  src=$(echo "$spec" | cut -d: -f1)
  bin=$(echo "$spec" | cut -d: -f2)
  libs=$(echo "$spec" | cut -d: -f3)
  command="gcc -std=c11 -O2 -Wall -Wextra -pthread $src -o $bin $libs"
  echo "$command" | tee -a evidence/build-environment.txt
  eval "$command"
done

timeout 30s Task02/allocation_todo | tee evidence/task02-allocation.txt
timeout 30s Task02/fragmentation_demo | tee evidence/task02-fragmentation.txt
timeout 30s Task02/allocation_lab | tee evidence/task02-costs.txt
timeout 30s Task04/disk_todo | tee evidence/task04-todo.txt
timeout 120s Task04/scheduler_sim | tee evidence/task04-simulation.csv
bash Task03/inspect_filesystem.sh
bash Task04/inspect_scheduler.sh
timeout 120s Task06/streaming_sim | tee evidence/task06-simulation.txt
python3 scripts/plot_results.py

grep -q 'E (10 blocks) contiguous: start 19' evidence/task02-allocation.txt
grep -q 'largest run=7, contiguous 10-block allocation start=-1' evidence/task02-fragmentation.txt
grep -q 'FCFS   53 98 183 37 122 14 124 65 67  -> 640 cylinders' evidence/task04-todo.txt
grep -q 'SSTF   53 65 67 37 14 98 122 124 183  -> 236 cylinders' evidence/task04-todo.txt
grep -q '183 cylinders (+199 for the jump)' evidence/task04-todo.txt
grep -q 'origin_reads=5057/100000' evidence/task06-simulation.txt
test -s evidence/task03-filesystem.txt
test -s evidence/task04-device.txt
test -s plots/scheduler_wait.png && test -s plots/streaming_latency.png
echo 'All Assignment 4 programs ran; expected allocation/SSTF outputs and evidence checks passed.' | tee evidence/verification.txt
```

### `Task02/allocation_lab.c`

```c
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    const uint64_t file_bytes = UINT64_C(4) * 1024 * 1024 * 1024;
    const uint64_t block_bytes = 4 * 1024;
    const uint64_t blocks = file_bytes / block_bytes;
    const uint64_t pointers_per_block = block_bytes / sizeof(uint32_t);
    const uint64_t inode_blocks = 12 + pointers_per_block +
        pointers_per_block * pointers_per_block +
        pointers_per_block * pointers_per_block * pointers_per_block;
    const uint64_t disk_bytes = UINT64_C(4000000000000); /* decimal 4 TB */
    const uint64_t disk_blocks = disk_bytes / block_bytes;
    const uint64_t bitmap_bytes = (disk_blocks + 7) / 8;
    const uint64_t fat_bytes = disk_blocks * sizeof(uint32_t);

    printf("4 GiB file / 4 KiB blocks: %" PRIu64 " data blocks\n", blocks);
    printf("Last-block access (inode/directory metadata excluded): contiguous=1 data read; linked=%" PRIu64 " block reads; FAT-in-memory=%" PRIu64 " RAM next-pointer lookups + 1 data read; two-level indexed=2 index reads + 1 data read\n",
           blocks, blocks - 1);
    printf("4-byte inode pointers per 4 KiB block: %" PRIu64 "\n", pointers_per_block);
    printf("12 direct + single + double + triple indirect: %" PRIu64 " blocks = %.0f bytes = %.2f TiB (theoretical pointer-address limit)\n",
           inode_blocks, (double)inode_blocks * (double)block_bytes,
           (double)inode_blocks * (double)block_bytes / (1024.0 * 1024 * 1024 * 1024));
    printf("4 TB decimal disk, 4 KiB blocks: %" PRIu64 " whole blocks\n", disk_blocks);
    printf("free-space bitmap: %" PRIu64 " bytes = %.2f MiB\n", bitmap_bytes,
           (double)bitmap_bytes / (1024.0 * 1024));
    printf("4-byte FAT for that many blocks: %" PRIu64 " bytes = %.2f MiB\n", fat_bytes,
           (double)fat_bytes / (1024.0 * 1024));
    return 0;
}
```

### `Task02/allocation_todo.c`

```c
#include <stdio.h>
#include <string.h>
#define BLOCKS 32
char disk[BLOCKS];                           /* '.' = free, otherwise the file letter */
/* Contiguous allocation, first fit: returns the start block or -1. */
int alloc_contiguous(char file, int size)
{
    if (size <= 0 || size > BLOCKS) return -1;
    int run = 0;
    for (int b = 0; b < BLOCKS; b++) {
        if (disk[b] == '.') {
            run++;
            if (run == size) {
                int start = b - size + 1;
                for (int i = start; i <= b; i++) disk[i] = file;
                return start;
            }
        } else {
            run = 0;
        }
    }
    return -1;
}
/* Indexed allocation: one index block plus any free data blocks. */
int alloc_indexed(char file, int size, int index[])
{
    int got = 0, idx = -1;
    for (int b = 0; b < BLOCKS && got <= size; b++) {
        if (disk[b] != '.') continue;
        if (idx < 0) idx = b;                /* first free block holds the index */
        else index[got++] = b;
        if (got == size) break;
    }
    if (idx < 0 || got < size) return -1;
    disk[idx] = (char)(file + 32);           /* lowercase letter = index block */
    for (int i = 0; i < size; i++) disk[index[i]] = file;
    return idx;
}
void release(char file)
{
    for (int b = 0; b < BLOCKS; b++) if (disk[b] == file) disk[b] = '.';
}
int main(void)
{
    memset(disk, '.', BLOCKS);
    alloc_contiguous('A', 6);
    alloc_contiguous('B', 8);
    alloc_contiguous('C', 5);
    alloc_contiguous('D', 7);
    release('B');
    release('D');
    printf("%.32s\n", disk);
    printf("E (10 blocks) contiguous: start %d\n", alloc_contiguous('E', 10));
    int index[16];
    int ib = alloc_indexed('E', 10, index);
    printf("E (10 blocks) indexed: index block %d, data blocks", ib);
    for (int i = 0; i < 10; i++) printf(" %d", index[i]);
    printf("\n%.32s\n", disk);
    return 0;
}
```

### `Task02/fragmentation_demo.c`

```c
#include <stdio.h>
#include <string.h>

#define BLOCKS 32
static char disk[BLOCKS];
static int contiguous(int size)
{
    int run = 0;
    for (int i = 0; i < BLOCKS; i++) {
        run = disk[i] == '.' ? run + 1 : 0;
        if (run == size) return i - size + 1;
    }
    return -1;
}
static int indexed(int size, int *index, int *data)
{
    int got = 0, root = -1;
    for (int i = 0; i < BLOCKS; i++) if (disk[i] == '.') {
        if (root < 0) root = i;
        else data[got++] = i;
        if (got == size) break;
    }
    if (root < 0 || got < size) return -1;
    disk[root] = 'i'; *index = root;
    for (int i = 0; i < size; i++) disk[data[i]] = 'X';
    return 0;
}
int main(void)
{
    memset(disk, '.', sizeof disk);
    memset(disk, 'A', 5); memset(disk + 10, 'C', 5); memset(disk + 20, 'E', 5);
    printf("fragmented layout: %.32s\n", disk);
    printf("free blocks=17, largest run=7, contiguous 10-block allocation start=%d\n", contiguous(10));
    int index, data[10];
    if (indexed(10, &index, data) != 0) return 1;
    printf("indexed 10-block allocation succeeds: index block=%d, data blocks", index);
    for (int i = 0; i < 10; i++) printf(" %d", data[i]);
    putchar('\n');
    return 0;
}
```

### `Task03/inspect_filesystem.sh`

```bash
#!/usr/bin/env bash
set -euo pipefail
mkdir -p evidence
{
  echo '=== Host filesystem containing the assignment ==='
  findmnt -T . -o TARGET,SOURCE,FSTYPE,OPTIONS
  sample=$(mktemp)
  printf 'assignment-4-filesystem-inspection\n' > "$sample"
  stat -c 'stat: inode=%i size=%s bytes block_size=%o blocks=%b mode=%A' "$sample"
  echo 'df -i for working filesystem:'
  df -i .
  echo 'ls -i sample:'
  ls -i "$sample"
  rm -f "$sample"
  src=$(findmnt -n -o SOURCE -T .)
  echo "tune2fs attempt for host source: $src"
  if command -v tune2fs >/dev/null 2>&1 && [[ -b "$src" ]]; then
    sudo -n tune2fs -l "$src" 2>&1 | head -n 25 || true
  else
    echo 'Host mount is not a directly accessible ext filesystem block device in this runner.'
  fi
  echo '=== Created ext4 image inspected with dumpe2fs ==='
  img=$(mktemp --suffix=.img)
  truncate -s 64M "$img"
  mkfs.ext4 -F -q "$img"
  dumpe2fs -h "$img" 2>&1 | grep -E 'Filesystem UUID|Filesystem features|Block count|Free blocks|Block size|Inode count|Free inodes|Inode size|Journal features|Checksum' || true
  rm -f "$img"
} | tee evidence/task03-filesystem.txt
```

### `Task04/disk_todo.c`

```c
#include <stdio.h>
#include <stdlib.h>
#define N 8
#define MAXCYL 199
int req[N] = {98, 183, 37, 122, 14, 124, 65, 67};
int cmp(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }
void show(const char *name, const int order[], int n, int head, int extra)
{
    int total = 0, pos = head;
    printf("%-6s %d", name, head);
    for (int i = 0; i < n; i++) {
        printf(" %d", order[i]);
        total += abs(order[i] - pos);
        pos = order[i];
    }
    printf("  -> %d cylinders\n", total + extra);
}
int main(void)
{
    int head = 53, order[N + 2];
    show("FCFS", req, N, head, 0);
    /* SSTF: always serve the nearest pending request */
    int done[N] = {0}, pos = head;
    for (int k = 0; k < N; k++) {
        int best = -1, distance = MAXCYL + 1;
        for (int j = 0; j < N; j++) {
            int d = abs(req[j] - pos);
            if (!done[j] && d < distance) { best = j; distance = d; }
        }
        done[best] = 1;
        order[k] = req[best];
        pos = req[best];
    }
    show("SSTF", order, N, head, 0);
    /* SCAN (towards cylinder 0 first, like an elevator) */
    int s[N];
    for (int i = 0; i < N; i++) s[i] = req[i];
    qsort(s, N, sizeof s[0], cmp);
    int n = 0, i;
    for (i = N - 1; i >= 0; i--) if (s[i] < head) order[n++] = s[i];
    order[n++] = 0;
    for (i = 0; i < N; i++) if (s[i] > head) order[n++] = s[i];
    show("SCAN", order, n, head, 0);
    /* C-SCAN (towards the last cylinder, then jump back to 0) */
    n = 0;
    for (i = 0; i < N; i++) if (s[i] > head) order[n++] = s[i];
    order[n++] = MAXCYL;
    int part = 0, p = head;
    for (int k = 0; k < n; k++) { part += abs(order[k] - p); p = order[k]; }
    int rest[N], m = 0;
    for (i = 0; i < N; i++) if (s[i] < head) rest[m++] = s[i];
    int back = 0; p = 0;
    for (int k = 0; k < m; k++) { back += abs(rest[k] - p); p = rest[k]; }
    printf("C-SCAN %d ... %d, jump to 0 ... %d  -> %d cylinders (+%d for the jump)\n",
           head, MAXCYL, rest[m - 1], part + back, MAXCYL);
    return 0;
}
```

### `Task04/inspect_scheduler.sh`

```bash
#!/usr/bin/env bash
set -euo pipefail
{
  echo 'Block devices and rotation flags:'
  lsblk -dn -o NAME,ROTA,TYPE,SIZE,MODEL || true
  echo 'Available Linux block schedulers:'
  found=0
  for sched in /sys/block/*/queue/scheduler; do
    [[ -r "$sched" ]] || continue
    found=1
    printf '%s: ' "$sched"
    cat "$sched"
  done
  if [[ $found -eq 0 ]]; then echo 'No host block scheduler sysfs paths exposed to this environment.'; fi
  echo 'The CI runner is virtualized; these values describe its guest-visible device, not the faculty/student laptop.'
} | tee evidence/task04-device.txt
```

### `Task04/scheduler_sim.c`

```c
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CYLINDERS 200
#define REQUESTS 10000
#define SEEK_MS_PER_CYL 0.08
#define IO_MS 2.0

enum policy { FCFS, SSTF, SCAN, CSCAN, LOOK, CLOOK, POLICY_COUNT };
static const char *policy_name[] = {"FCFS", "SSTF", "SCAN", "C-SCAN", "LOOK", "C-LOOK"};
struct request { int cyl, state; double arrival; };
struct stats { double avg_move, max_wait, avg_wait; };
static uint32_t rng = 0x0406a55aU;
static uint32_t random_u32(void)
{
    rng = rng * 1664525U + 1013904223U;
    return rng;
}
static void add_arrivals(struct request r[], int *next, double now)
{
    while (*next < REQUESTS && r[*next].arrival <= now) {
        r[*next].state = 1;
        (*next)++;
    }
}
static int queued(const struct request r[], int count)
{
    for (int i = 0; i < count; i++) if (r[i].state == 1) return 1;
    return 0;
}
static int choose_fifo(const struct request r[])
{
    for (int i = 0; i < REQUESTS; i++) if (r[i].state == 1) return i;
    return -1;
}
static int choose_sstf(const struct request r[], int head)
{
    int best = -1, distance = CYLINDERS + 1;
    for (int i = 0; i < REQUESTS; i++) if (r[i].state == 1) {
        int d = abs(r[i].cyl - head);
        if (d < distance) { best = i; distance = d; }
    }
    return best;
}
static int choose_direction(const struct request r[], int head, int direction, int extreme)
{
    int best = -1;
    for (int i = 0; i < REQUESTS; i++) if (r[i].state == 1) {
        int c = r[i].cyl;
        if ((direction > 0 && c >= head) || (direction < 0 && c <= head)) {
            if (best < 0 || (direction > 0 ? c < r[best].cyl : c > r[best].cyl)) best = i;
        }
    }
    if (best >= 0 || !extreme) return best;
    for (int i = 0; i < REQUESTS; i++) if (r[i].state == 1) {
        int c = r[i].cyl;
        if (best < 0 || (direction > 0 ? c < r[best].cyl : c > r[best].cyl)) best = i;
    }
    return best;
}
static struct stats simulate(enum policy policy, double interval)
{
    struct request *r = calloc(REQUESTS, sizeof *r);
    if (!r) exit(2);
    rng = 0x0406a55aU;
    for (int i = 0; i < REQUESTS; i++) {
        r[i].cyl = (int)(random_u32() % CYLINDERS);
        r[i].arrival = i * interval;
    }
    int next = 0, head = 53, direction = policy == SCAN ? -1 : 1;
    double now = 0, total_move = 0, total_wait = 0, max_wait = 0;
    for (int served = 0; served < REQUESTS;) {
        add_arrivals(r, &next, now);
        if (!queued(r, REQUESTS)) {
            now = r[next].arrival;
            add_arrivals(r, &next, now);
        }
        int pick = -1;
        if (policy == FCFS) pick = choose_fifo(r);
        else if (policy == SSTF) pick = choose_sstf(r, head);
        else if (policy == SCAN || policy == CSCAN || policy == LOOK || policy == CLOOK) {
            pick = choose_direction(r, head, direction, 0);
            if (pick < 0 && (policy == LOOK || policy == CLOOK)) {
                int other = 0;
                for (int j = 0; j < REQUESTS; j++) if (r[j].state == 1) other = 1;
                if (other && policy == LOOK) {
                    direction = -direction;
                    pick = choose_direction(r, head, direction, 0);
                } else if (other && policy == CLOOK) {
                    direction = 1;
                    int far = -1;
                    for (int j = 0; j < REQUESTS; j++) if (r[j].state == 1 &&
                        (far < 0 || r[j].cyl < r[far].cyl)) far = j;
                    pick = far;
                    if (pick >= 0) {
                        total_move += abs(head - r[pick].cyl);
                        now += abs(head - r[pick].cyl) * SEEK_MS_PER_CYL;
                        head = r[pick].cyl;
                    }
                }
            } else if (pick < 0 && policy == SCAN) {
                int edge = direction < 0 ? 0 : CYLINDERS - 1;
                total_move += abs(head - edge);
                now += abs(head - edge) * SEEK_MS_PER_CYL;
                head = edge; direction = -direction;
                add_arrivals(r, &next, now);
                pick = choose_direction(r, head, direction, 0);
            } else if (pick < 0 && policy == CSCAN) {
                int edge = direction > 0 ? CYLINDERS - 1 : 0;
                int wrap = direction > 0 ? 0 : CYLINDERS - 1;
                total_move += abs(head - edge) + abs(edge - wrap);
                now += (abs(head - edge) + abs(edge - wrap)) * SEEK_MS_PER_CYL;
                head = wrap;
                add_arrivals(r, &next, now);
                pick = choose_direction(r, head, direction, 1);
            }
        }
        if (pick < 0 && next < REQUESTS && policy >= SCAN) {
            if ((policy == LOOK || policy == CLOOK) && !queued(r, REQUESTS)) {
                now = r[next].arrival;
                add_arrivals(r, &next, now);
            }
            continue;
        }
        if (pick < 0) { free(r); fprintf(stderr, "scheduler made no progress\n"); exit(3); }
        double wait = now - r[pick].arrival;
        if (wait > max_wait) max_wait = wait;
        total_wait += wait;
        int distance = abs(head - r[pick].cyl);
        total_move += distance;
        now += distance * SEEK_MS_PER_CYL + IO_MS;
        head = r[pick].cyl;
        r[pick].state = 2;
        served++;
    }
    struct stats result = {total_move / REQUESTS, max_wait, total_wait / REQUESTS};
    free(r);
    return result;
}
int main(void)
{
    const double intervals[] = {30.0, 10.0, 3.0};
    const char *loads[] = {"light", "medium", "heavy"};
    puts("Synthetic online queue simulation: 10,000 seeded requests, cylinders 0-199, head=53");
    puts("Model: 0.08 ms per cylinder plus 2 ms per request; arrival intervals are assumptions, not hardware benchmarks.");
    puts("load,policy,arrival_ms,average_head_movement_cyl,average_wait_ms,max_wait_ms");
    for (int l = 0; l < 3; l++) for (int p = 0; p < POLICY_COUNT; p++) {
        struct stats s = simulate((enum policy)p, intervals[l]);
        printf("%s,%s,%.1f,%.3f,%.3f,%.3f\n", loads[l], policy_name[p],
               intervals[l], s.avg_move, s.avg_wait, s.max_wait);
    }
    puts("SSTF starvation illustration: while positioned at cylinder 60, 200 requests at cylinder 60 arrive every 1 ms; each read occupies 2 ms. A queued request at 183 remains behind the closer stream for at least 400 ms. If closer work continues without bound, SSTF has no finite wait bound.");
    return 0;
}
```

### `Task06/streaming_sim.c`

```c
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define STARTS 100000
#define VIDEO_COUNT 10000
#define SEGMENT_MIB 1.0
#define DISK_COUNT 4
#define HDD_SEEK_MS 8.0
#define HDD_MIB_PER_SEC 180.0
#define SSD_SEEK_MS 0.15
#define SSD_MIB_PER_SEC 1800.0

static uint32_t rng = 0x4062026U;
static uint32_t next_u32(void) { rng = rng * 1664525U + 1013904223U; return rng; }
static int compare_double(const void *a, const void *b)
{
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}
static void simulate_local_hdd(double wait[], double service)
{
    double available[DISK_COUNT] = {0};
    for (int i = 0; i < STARTS; i++) {
        int disk = 0;
        for (int d = 1; d < DISK_COUNT; d++) if (available[d] < available[disk]) disk = d;
        double arrival = i * 0.5; /* 2,000 start requests/second; time unit is ms */
        double start = fmax(arrival, available[disk]);
        wait[i] = start - arrival + service;
        available[disk] = start + service;
    }
}
static void simulate_ssd_edge_cache(double wait[], double hit_ratio)
{
    double available[DISK_COUNT] = {0};
    for (int i = 0; i < STARTS; i++) {
        int video = (int)(next_u32() % VIDEO_COUNT);
        int hot = video < (int)(VIDEO_COUNT * hit_ratio);
        double arrival = i * 0.5;
        if (hot) {
            wait[i] = 20.0 + SSD_SEEK_MS + SEGMENT_MIB / SSD_MIB_PER_SEC * 1000.0;
        } else {
            int disk = 0;
            for (int d = 1; d < DISK_COUNT; d++) if (available[d] < available[disk]) disk = d;
            double service = HDD_SEEK_MS + SEGMENT_MIB / HDD_MIB_PER_SEC * 1000.0;
            double start = fmax(arrival, available[disk]);
            wait[i] = start - arrival + 20.0 + service;
            available[disk] = start + service;
        }
    }
}
static void report(const char *name, double values[])
{
    double *sorted = malloc(sizeof(double) * STARTS);
    if (!sorted) exit(2);
    for (int i = 0; i < STARTS; i++) sorted[i] = values[i];
    qsort(sorted, STARTS, sizeof *sorted, compare_double);
    double sum = 0;
    for (int i = 0; i < STARTS; i++) sum += sorted[i];
    int reads = 0;
    for (int i = 0; i < STARTS; i++) if (values[i] > 21.0) reads++;
    printf("%s: starts=%d mean_start_latency=%.3f ms p50=%.3f ms p95=%.3f ms p99=%.3f ms max=%.3f ms origin_reads=%d/%d\n",
           name, STARTS, sum / STARTS, sorted[STARTS / 2],
           sorted[(STARTS * 95) / 100], sorted[(STARTS * 99) / 100], sorted[STARTS - 1],
           name[0] == 'L' ? STARTS : reads, STARTS);
    free(sorted);
}
int main(void)
{
    const double bytes_per_video_gib = 1.0;
    const double library_gib = VIDEO_COUNT * bytes_per_video_gib;
    const double network_gbps = 20000.0 * 4.0 / 1000.0;
    const double origin_gbps = network_gbps * 0.05;
    double service = HDD_SEEK_MS + SEGMENT_MIB / HDD_MIB_PER_SEC * 1000.0;
    double *a = calloc(STARTS, sizeof *a), *b = calloc(STARTS, sizeof *b);
    if (!a || !b) return 2;
    printf("Assumed catalog: %d videos x %.1f GiB = %.1f TiB raw; 3 replicas = %.1f TiB before overhead.\n",
           VIDEO_COUNT, bytes_per_video_gib, library_gib / 1024.0, library_gib * 3 / 1024.0);
    printf("Peak scenario: 20,000 viewers x 4 Mbit/s = %.0f Gbit/s; CDN 95%% cache ratio => %.1f Gbit/s origin egress.\n",
           network_gbps, origin_gbps);
    printf("Random-seek segment: %.1f MiB. Model service: HDD %.3f ms, SSD %.3f ms; network edge RTT 20 ms.\n",
           SEGMENT_MIB, service, SSD_SEEK_MS + SEGMENT_MIB / SSD_MIB_PER_SEC * 1000.0);
    printf("4 TB disk at 4 KiB allocation blocks: each %.1f MiB segment spans %d blocks; an offset lookup needs a few extent/index reads before the data run.\n",
           SEGMENT_MIB,
           (int)(SEGMENT_MIB * 1024.0 / 4.0));
    simulate_local_hdd(a, service);
    simulate_ssd_edge_cache(b, 0.95);
    report("Local HDD pool (4 disks, no edge cache)", a);
    report("SSD edge cache plus HDD origin (95% hits)", b);
    puts("Arrival burst is modeled at 2,000 starts/second. All latency values are a deterministic queueing simulation using stated assumptions, not a physical hardware or CDN benchmark. Playback start includes the modeled random segment lookup, not full-video transfer.");
    free(a); free(b);
    return 0;
}
```
