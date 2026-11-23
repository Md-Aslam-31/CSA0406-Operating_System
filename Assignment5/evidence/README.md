# Assignment 5 evidence

`run_linux_evidence.sh` records all terminal output to text files and generates performance charts. It saves operating-system observations, the shell-log exercise, cron line, systemd/Nginx/ufw state, image builds and sizes, Docker limits/history, bare-metal/container/VM benchmark output, kind deployment/pod-recovery/scale output, and capacity arithmetic. `verification.txt` records whether each expected behavior passed. When a hosted Ubuntu feature is unavailable, the output identifies the skip and keeps the related exact local commands in the task README; it does not invent a successful run.

The GitHub Actions run artifact is the Linux evidence bundle. The hosted runner is ephemeral; image sizes, timing, and `/proc` values describe that runner and are not a benchmark of the student's laptop or production hardware. Repeat with `bash scripts/run_linux_evidence.sh` on a prepared Ubuntu host.
