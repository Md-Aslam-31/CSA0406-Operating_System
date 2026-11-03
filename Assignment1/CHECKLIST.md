# Assignment 1 Verification Checklist

**Student:** Mohamed Aslam H · **Register:** 192572147 · **Department:** CSE (AI) · **Academic year:** 2025 · **Faculty:** Dr. Jayasakthi Velmurugan K

| Requirement | Status | Verification / remaining step |
|---|---|---|
| Assignment and rubric inspected | DONE | Both supplied PDFs reviewed; all 12 tasks and submission requirements captured. |
| Windows/WSL/GCC/Git inspected | DONE | Host build 26200 (ProductName reports Windows 10 Pro); MSYS2 GCC 16.1.0 and Git available. WSL absent. |
| Local WSL 2 + Ubuntu 24.04 | USER ACTION | This shell is not elevated. Run `wsl --install -d Ubuntu-24.04` in Administrator PowerShell and restart if requested. Ubuntu CI has verified the Linux workload. |
| Tasks 1–4 evaluation | DONE | Ranked requirements, OS type/structure alternatives, services and justifications in report. |
| Task 5 system calls and strace | DONE | Explanation and actual Ubuntu `strace -c` output included. |
| Task 6 copy program | CI VERIFY PENDING | Main output was verified in earlier successful CI. Current branch adds boundary/error checks; new workflow run pending. |
| Task 7 process-control program | CI VERIFY PENDING | Main output was verified in earlier successful CI. Current branch adds exec-failure and output-buffering checks; new workflow run pending. |
| Tasks 8–9 protection and I/O | DONE | Applied analysis and 500 MB interrupt calculation in report. |
| Task 10 measurements | DONE | Actual one-run Ubuntu timings and `/usr/bin/time -v` captured; single-trial limitation documented. |
| Tasks 11–12 architecture and design challenge | DONE | Architecture, scaling, virtualization and home/cloud analysis included. |
| References | DONE | Linux man-pages, FreeRTOS, QNX and Ubuntu documentation listed. |
| Per-task repository structure | DONE | `Task-01` through `Task-12` each have task-level writeups. |
| Terminal output and measurements | DONE | Raw captured outputs in `evidence/`; table in `Results/task_measurements.csv`. |
| Genuine terminal screenshots | NOT AVAILABLE | No Linux desktop was available for screenshot capture; raw workflow logs are retained without claiming they are screenshots. |
| DOCX/PDF report | DONE | Word-authored PDF; 8-page render visually checked page by page. |
| Dedicated repository `CSA04-192572147-Assignment1` | USER ACTION | GitHub browser automation could read but not operate the logged-in Chrome window. Repository creation must occur in GitHub UI before pushing the standalone repository. |
| Task-scoped commit history | IN PROGRESS | Separate task folder commits are being created. |
| Push and remote verification | IN PROGRESS | Existing course repository is the authorized reachable remote; dedicated repository push requires its creation. |
