# Q6 - Disk Management

`disk_analysis.c` calculates six-disk RAID usable capacities and independent annual data-loss probabilities under a stated 2% per-disk failure assumption. The evidence script captures `lsblk`, `fdisk -l`, `findmnt`, swap status and a file-read throughput sample on the runner filesystem. The runner does not provide a comparable HDD and SSD pair, so no false device-class comparison is claimed. See `../evidence/task06-disk.txt`.
