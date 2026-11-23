#!/usr/bin/env python3
"""Render workload throughput/startup and container image comparison figures."""
import csv
import re
from pathlib import Path

import matplotlib.pyplot as plt

base = Path(__file__).resolve().parent.parent
with (base / "evidence/benchmark.csv").open(newline="", encoding="utf-8") as stream:
    measurements = list(csv.DictReader(stream))
usable = [row for row in measurements if row["requests_per_second"]]
if not usable:
    raise SystemExit("No benchmark request rates found")

fig, axes = plt.subplots(1, 2, figsize=(10, 4.2))
labels = [row["deployment"] for row in usable]
rates = [float(row["requests_per_second"]) for row in usable]
startup = [float(row["startup_seconds"]) for row in usable]
axes[0].bar(labels, rates, color=["#175cd3", "#12b76a", "#f79009"][: len(labels)])
axes[0].set_ylabel("Requests per second (higher is better)")
axes[0].set_title("Same /simulate/login route, ab -n 2000 -c 25")
axes[0].grid(axis="y", alpha=.25)
axes[1].bar(labels, startup, color=["#175cd3", "#12b76a", "#f79009"][: len(labels)])
axes[1].set_ylabel("Seconds to first successful health check")
axes[1].set_title("Measured startup path")
axes[1].grid(axis="y", alpha=.25)
fig.tight_layout()
fig.savefig(base / "plots/deployment-comparison.png", dpi=180)

text = (base / "evidence/b2-images.txt").read_text(encoding="utf-8")
variants, sizes = [], []
for match in re.finditer(r'image=\["?result-portal:([^\]" ]+)"?\] bytes=(\d+)', text):
    variants.append(match.group(1))
    sizes.append(int(match.group(2)) / 1024 / 1024)
if not variants:
    raise SystemExit("No Docker image sizes found in evidence/b2-images.txt")
fig, ax = plt.subplots(figsize=(8, 4.2))
ax.bar(variants, sizes, color="#7f56d9")
ax.set_ylabel("Image size (MiB)")
ax.set_title("Measured portal image variants")
ax.grid(axis="y", alpha=.25)
fig.tight_layout()
fig.savefig(base / "plots/image-size.png", dpi=180)
