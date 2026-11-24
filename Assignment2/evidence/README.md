# Captured Linux evidence

These raw outputs were produced by successful GitHub Actions run [#44579401828](https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/44579401828) on Ubuntu 24.04.5 LTS with GCC 13.3.0. Rebuild and regenerate them with `bash scripts/run_linux_evidence.sh` from the Assignment2 folder. Timing results are specific to this runner and workload.


## Complete OnlineGDB run screenshots

The C source files remain in `Task01` through `Task06`; the screenshots document real OnlineGDB compile/run sessions. Terminal output that exceeds one screen is split into beginning and ending captures, and the full raw stdout is retained in this evidence folder.

- `onlinegdb/process-demo-full.jpg` and `onlinegdb/process_demo-output-first.jpg` — expanded terminal views showing child creation, `ps` and `/proc` snapshots, and exit statuses.
- `onlinegdb/scheduling-full.jpg` and `onlinegdb/scheduling-output-start.jpg` — expanded scheduler results across all policies, with Gantt sequences and timing metrics.
- `onlinegdb/thread_pool-full.jpg` — thread-pool source/run and timing output.
- `onlinegdb/ipc_benchmark-full.jpg` — OnlineGDB IPC run; its 100 MiB shared-memory case ended with SIGBUS/exit 135. The full successful Ubuntu measurements are in `task04-ipc.csv`.
- `onlinegdb/bounded_buffer-full.jpg` — correctness, repeated stress, and lock-scope measurements.
- `onlinegdb/sync_primitives_bench-full.jpg` — synchronization benchmark.
- `onlinegdb/banker-full.jpg` — safe/unsafe-state and request checks.
