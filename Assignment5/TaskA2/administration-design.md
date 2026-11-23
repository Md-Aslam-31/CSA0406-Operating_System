# A2 - Design system administration (5 marks)

## Accounts, services, logs

Create named human accounts (no shared `student` or `admin` login), groups `portal-ops`, `portal-readonly` and `db-ops`, MFA-backed SSH keys for operators, and a non-login `result-portal` account for the web service. A separate `postgres` account owns database files. Keep secrets in a protected secret store, not in Git or shell history. Review group membership monthly and log every sudo action.

The portal systemd unit is in `TaskB1/result-portal.service`: restart on failure, explicit working directory, dedicated account, umask 0027, private temporary directory and filesystem protections. Nginx terminates TLS and forwards only to `127.0.0.1:8000`. Forward `journalctl` and Nginx access/error logs to the central log service; alert on repeated 5xx, failed health checks, disk over 80%, swap activity and certificate expiry. Preserve student identifiers only where operationally needed and retain logs for the campus-approved period.

## Network and automation

Assign a stable private address using DHCP reservation or infrastructure IPAM; publish `results.example.edu` through managed DNS to a public load balancer, not the node. Nginx is the only public web entry. Default-deny inbound firewall, allow TCP 22 only from the campus VPN/bastion, and TCP 80/443 at the edge; redirect 80 to HTTPS. Disable password SSH, prohibit root SSH, use keys and rate limiting. Database port 5432 remains on a private subnet and accepts only portal nodes. Rotate logs, renew TLS certificates, take encrypted database-consistent backups, test restore, and run disk/health alerts automatically. `TaskB1/cron.txt` shows the hourly cron entry; systemd timers are safer for production because they retain missed schedules and central logs.

## Ansible alternative and complexity

`TaskA2/ansible/site.yml` is an idempotent desired-state playbook. Install its collection requirements, copy `inventory.example.ini` to a private inventory and replace the reserved example address, then run `ansible-playbook -i inventory.ini TaskA2/ansible/site.yml`. A hand-written bootstrap on one node has little setup overhead: roughly 15-30 minutes for this small fixed set of files/packages. Doing it on 20 nodes serially is O(n*k) operator actions for n hosts and k setup steps, often 5-10 engineer-hours including checks; results can drift. Ansible has an upfront inventory/role/key setup (illustratively 1-2 hours), then one playbook run fans out to 20 nodes with bounded parallelism, so operator effort is O(k + n/p) wall-clock batches for parallelism p, while total work remains O(n*k). Idempotency and version-controlled variables reduce configuration drift and simplify repeat runs; a broken role can propagate quickly, so validate in staging and use limited batches.

**Decision:** use Ansible for repeatable OS/Nginx/service/firewall changes and shell scripts only for a host-local one-off check. The exact minute estimates are planning estimates, not measurements from 20 physical hosts; verify with the university's inventory and approval process.

