# Q7 - First-Principles Memory-Management Plan

`memory_plan.c` times a fixed memory hot set with and without `mlock`, reports p50/p99/max loop delay and memlock limits, and simulates three task streams with and without pinned control pages. It includes a 2 GiB budget and evaluates the extra 500 MiB task. Linux scheduling cannot prove a hard real-time bound; deployment needs target-specific deadline validation. See `../evidence/task07-memory-plan.txt`.
