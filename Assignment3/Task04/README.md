# Q4 - Virtual Memory and Demand Paging

`virtual_memory.c` calculates page-fault EAT, touches memory sequentially/randomly and records `getrusage` faults, inspects `/proc/self/smaps_rollup` around fork/COW, and safely tests a 256 MiB address-space limit against a 512 MiB allocation. The evidence script also captures `/usr/bin/time -v`. See `../evidence/task04-*` and `../plots/page_faults.png`.
