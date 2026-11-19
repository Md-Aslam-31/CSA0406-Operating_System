# CSA0406 Operating Systems - Assignment 3 (CO3)

**Student:** Mohamed Aslam H | **Register:** 192572147 | **Department:** CSE (AI) | **Academic year:** 2025 | **Faculty:** Dr. Jayasakthi Velmurugan K

Assignment 3 is separate from Assignments 1 and 2 and is stored in this folder of the course repository. The complete report is `REPORT.md` (also provided as PDF and editable DOCX). `Task01` through `Task07` contain source code and task notes; `evidence/` has Linux output; `plots/` has generated charts.

## Build and run

The verified run uses Ubuntu 24.04.5 LTS and GCC 13.3.0. Exact compile commands and environment are captured in `evidence/build-environment.txt`. From this folder run:

```sh
bash scripts/run_linux_evidence.sh
```

This builds and executes all C programs, tests the provided TODO programs, records system and performance output, and generates page-fault plots. The workflow uploads the evidence artifact. Linux is required for ELF libraries, `dlopen`, `/proc`, `mlock`, `fork`, `getrusage`, `lsblk` and `fdisk`.

## Repository

https://github.com/Md-Aslam-31/CSA0406-Operating_System/tree/main/Assignment3

## Verified run

All seven tasks passed on Ubuntu 24.04.5 LTS with GCC 13.3.0. [View GitHub Actions run #44586278245](https://github.com/Md-Aslam-31/CSA0406-Operating_System/actions/runs/44586278245). The retained raw evidence and plots are included here; the report PDF and editable DOCX are in this folder.
