# CSA0406 Operating Systems - Assignment 5 (CO5)

**Student:** Mohamed Aslam H  
**Register number:** 192572147  
**Department:** CSE (AI)  
**Academic year:** 2025  
**Faculty:** Dr. Jayasakthi Velmurugan K  
**Nature:** Individual

This assignment is kept in its own `Assignment5` folder in the course repository, consistent with the repository used for Assignments 1-4. The submission includes the nine rubric tasks, executable sample portal, shell scripts, Docker/Compose/Kubernetes configuration, workload measurements, evidence, and the PDF and editable Word reports. [Open the report PDF](REPORT.pdf), [Word report](REPORT.docx), or [successful Linux run](https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/45236369527).

## Run

Use Ubuntu 24.04 (VM or Linux host) with GCC, Python 3/pip, Docker Engine with Compose v2, kubectl and kind. `matplotlib` is needed for charts; some B1 commands require sudo. The CI records the Ubuntu and GCC versions using `/etc/os-release` and `gcc --version`. This assignment has no C sources, so there is no GCC compile command; Python, shell, Docker and YAML configurations are run directly. The complete repeatable run is:

```bash
cd Assignment5
bash scripts/run_linux_evidence.sh
```

The script stores command output in `evidence/`, graphs in `plots/`, and runs the same Flask app directly on Linux, in Docker and in a QEMU/KVM guest. It also builds a local kind cluster, checks pod replacement and scales the deployment under load. It records the actual virtualization mode. B1 service verification requires a systemd Linux host; if a host lacks systemd the run stops and reports the missing requirement instead of claiming success.

For a short interactive run:

```bash
docker compose -f TaskB2/compose.yaml up --build -d
curl http://localhost:8000/health
docker compose -f TaskB2/compose.yaml ps
docker stats --no-stream
docker history result-portal:compose
docker compose -f TaskB2/compose.yaml down
```

## Evidence and notes

- `evidence/README.md` describes which measurements ran in the Ubuntu workflow and how to read them.
- B1's original worksheet code is preserved outside its TODOs; both status-count and busiest-client outputs match the expected output, including its 43,200-byte total.
- VM performance is labeled with the actual KVM or software-emulation mode; don't treat emulated timings as bare-metal KVM results.
- The hosted Ubuntu runner booted the QEMU TCG guest, but the guest health endpoint was unavailable within the 360-second limit. The report labels this VM benchmark unavailable; run the same VM script on a Linux host with working KVM for measured VM performance.
- The production cost example uses published AWS us-east-1 Linux EC2 rates and states the capacity assumptions; it excludes database, storage, load balancer, data transfer and tax.

## Repository

https://github.com/Md-Aslam-31/CSA0406-Operating_System/tree/main/Assignment5
