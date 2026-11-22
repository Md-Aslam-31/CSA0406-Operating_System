from pathlib import Path
import csv
import re
import matplotlib.pyplot as plt

base = Path(__file__).resolve().parent.parent
lines = (base / "evidence/task04-simulation.csv").read_text().splitlines()
header_index = next(i for i, line in enumerate(lines) if line.startswith("load,policy,"))
rows = list(csv.DictReader(lines[header_index:]))
fig, axes = plt.subplots(1, 3, figsize=(10, 3.7), sharey=True)
for ax, load in zip(axes, ["light", "medium", "heavy"]):
    group = [r for r in rows if r["load"] == load]
    ax.bar([r["policy"] for r in group], [float(r["max_wait_ms"]) for r in group], color="#2673b8")
    ax.set_title(load.title() + " load")
    ax.tick_params(axis="x", rotation=45)
    ax.grid(axis="y", alpha=.25)
axes[0].set_ylabel("Maximum queue wait (ms)")
fig.suptitle("Disk scheduling: maximum wait in seeded 10,000-request model")
fig.tight_layout()
fig.savefig(base / "plots/scheduler_wait.png", dpi=170)

text = (base / "evidence/task06-simulation.txt").read_text()
names = ["Local HDD pool (4 disks, no edge cache)", "SSD edge cache plus HDD origin (95% hits)"]
values = []
for name in names:
    m = re.search(re.escape(name) + r":.*?p95=([0-9.]+) ms", text)
    if m:
        values.append(float(m.group(1)))
fig, ax = plt.subplots(figsize=(7, 4))
ax.bar(["Four HDDs\n(no edge cache)", "SSD/CDN cache\n(95% modeled hits)"], values, color=["#b54708", "#18805a"])
ax.set_ylabel("95th percentile startup lookup latency (ms)")
ax.set_title("Streaming start requests in stated queue model")
ax.grid(axis="y", alpha=.25)
fig.tight_layout()
fig.savefig(base / "plots/streaming_latency.png", dpi=170)
