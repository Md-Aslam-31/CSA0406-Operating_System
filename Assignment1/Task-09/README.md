# Assignment 1 — Task 09

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 9 — Memory hierarchy and I/O

Typical order-of-magnitude values vary by hardware: CPU registers ~sub-ns to a few ns; L1 cache ~1 ns, L2 several ns, L3 tens of ns; DRAM ~50–150 ns; NVMe SSD ~tens–hundreds of µs; HDD ~5–15 ms; network storage ~sub-ms to tens of ms depending on network/load. Capacity generally increases as speed decreases. These are illustrative ranges, not benchmark claims.

| Data | Placement and rationale |
|---|---|
| Active monitor values/alarm state | Registers/cache/DRAM in local controller; bounded durable event log in flash. Never block alarm on remote storage. |
| Frequently accessed patient index/records | Database DRAM buffer/cache backed by replicated durable SSD/storage; cache requires invalidation/consistency controls. |
| MRI scan in progress | DRAM buffers and high-throughput local NVMe/scratch; archive validated images to managed durable storage. |
| Long-term records/images | Encrypted replicated storage with offline/immutable backup and tested restore. |

Programmed I/O ties CPU to every transfer and suits simple low-rate controls. Interrupt-driven I/O avoids busy waiting and suits moderate event rates. DMA transfers blocks with less CPU intervention and suits high-volume MRI acquisition; use bounded buffers, completion interrupts, checksums and IOMMU protection. For a 500 MB image at 4 bytes/word, interrupt-per-word implies 125,000,000 interrupts. At even 1 µs handler overhead each, the interrupt overhead alone is 125 seconds; at 5 µs, 625 seconds. DMA instead needs a small number of block completions, plus setup/interrupt overhead. Interrupt-per-word also requires buffering/processing each word; DMA needs one or more bounded large buffers and descriptors. Caching speeds repeated lookups by avoiding slower storage, but stale clinical data must be prevented with transactional invalidation/consistency rules.

See the [complete report](../REPORT.md) for the full submission and references.
