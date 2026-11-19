# Q3 - Segmentation

Build with `gcc -std=c11 -Wall -Wextra -pthread segment_lab.c -o segment_lab`; run `./segment_lab` on Linux. It checks eight segment/offset pairs and bounds, prints `/proc/self/maps`, triggers a protected-page fault in a child, and reports a 20-segment external-fragmentation simulation. See `../evidence/task03-segmentation.txt`.
