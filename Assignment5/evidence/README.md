# Assignment 5 evidence

`run_linux_evidence.sh` records all terminal output to text files and generates performance charts. It saves operating-system observations, the shell-log exercise, cron line, systemd/Nginx/ufw state, image builds and sizes, Docker limits/history, bare-metal/container/VM benchmark output, kind deployment/pod-recovery/scale output, and capacity arithmetic. `verification.txt` records whether each expected behavior passed. When a hosted Ubuntu feature is unavailable, the output identifies the skip and keeps the related exact local commands in the task README; it does not invent a successful run.

The GitHub Actions run artifact is the Linux evidence bundle. The hosted runner is ephemeral; image sizes, timing, and `/proc` values describe that runner and are not a benchmark of the student's laptop or production hardware. Repeat with `bash scripts/run_linux_evidence.sh` on a prepared Ubuntu host.


## OnlineGDB screenshot

- onlinegdb/taskb4-capacity-calculation.jpg: standalone OnlineGDB Python calculation based on measured container throughput; the full script and CSV evidence are stored separately.

## OnlineGDB output screenshots

- `onlinegdb/capacity_estimate-full.jpg` and `capacity_estimate-output-start.jpg` — complete OnlineGDB capacity run against the benchmark sample committed at capture time (303.12 requests/s). The later Linux rerun #45451421902 refreshed the committed CSV and `b4-capacity-cost.txt` to 270.20 requests/s; those latest files drive the current report analysis.
- `onlinegdb/taskb4-capacity-calculation.jpg` — preliminary standalone capacity arithmetic, superseded by the full script run.
- Shell/service/container/Kubernetes output remains in the matching `b1-*`, `b2-*`, and `b3-*` evidence files; those require an Ubuntu/Docker/kind environment and are not runnable in OnlineGDB.
