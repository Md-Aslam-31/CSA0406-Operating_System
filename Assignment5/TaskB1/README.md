# B1 - Administer Linux

Run `bash scripts/run_linux_evidence.sh` from Assignment5 on Ubuntu 24.04 to capture `ps`, `top`, `free`, `vmstat`, `iostat`, `df`, `ss`, the hourly sample-log report, disk alert, Nginx/systemd/ufw state and service health. The workflow uses a synthetic portal and synthetic access log only; it contains no student records. `setup-service.sh` installs the service and Nginx config and starts them using systemd. For a real campus system, restrict SSH source addresses to the VPN/bastion and choose a faculty-approved retention policy.

The cron line is in `cron.txt`; `setup-service.sh` creates the service-owned log file. On Ubuntu install cron and the service, then run `sudo crontab -u result-portal TaskB1/cron.txt` and `sudo systemctl enable --now cron`. Under CI, the script runs with a 0% threshold as a safe boundary test, so it proves the warning branch without filling the runner disk. The live command output is recorded in `evidence/b1-*.txt`.

The submitted `log_summary.sh` retains the worksheet's non-TODO lines and replaces only the TODOs. Because awk splits the quoted request at spaces, its status code is field 7 and byte count field 8; the original total expression is correct. Both scripts print the supplied expected totals and busiest client.

### Central logging alternative

A journald forwarder or central ELK/OpenSearch stack indexes logs from all 10 nodes. An operator can filter by request ID, status and time range in one query instead of SSHing to hosts and repeating ten local greps (O(n) host visits versus one indexed query, typically O(log N) retrieval after ingestion). The collector, index and replicas consume extra memory and disk; estimate storage as daily bytes x retention days x index/replication factor, and apply role-based access because URLs may contain sensitive identifiers. Setup requires TLS, clocks, retention, access policy, capacity monitoring and failure alarms. Keep a small local fallback spool so the portal remains diagnosable if the central sink is unavailable.

