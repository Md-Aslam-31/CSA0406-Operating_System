#!/usr/bin/env python3
"""Capacity lower-bound and annual EC2 compute-only sensitivity model."""
import csv
import math
import sys
from pathlib import Path

source = Path(sys.argv[1] if len(sys.argv) > 1 else "evidence/benchmark.csv")
rates = {}
with source.open(newline="", encoding="utf-8") as f:
    for row in csv.DictReader(f):
        if row["requests_per_second"]:
            rates[row["deployment"]] = float(row["requests_per_second"])

students, event_seconds, flash_factor, target_util = 150_000, 7_200, 5, 0.70
rate = rates.get("container", 0)
if rate <= 0:
    raise SystemExit("Missing positive measured container requests_per_second in benchmark.csv")
replica_scenarios = {}
for per_student in (1, 5, 10):
    avg = students * per_student / event_seconds
    peak = avg * flash_factor
    required = math.ceil(peak / (rate * target_util))
    replica_scenarios[per_student] = required
    print(f"requests_per_student={per_student}; average_rps={avg:.2f}; modeled_5x_peak_rps={peak:.2f}; required_at_70pct={required}")
required = replica_scenarios[10]
baseline = 3
nodes_fixed = max(baseline, required)
burst_hours_per_year = 2 * 4
usd_per_hour = 0.0835
fixed_peak_fleet_usd = nodes_fixed * 8760 * usd_per_hour
burst_compute_usd = max(0, required - baseline) * burst_hours_per_year * usd_per_hour
cloud_only_usd = baseline * 8760 * usd_per_hour + burst_compute_usd
hybrid_incremental_usd = burst_compute_usd

print("Input: 150,000 students / 2 hours; 1/5/10 HTTP requests per student; modeled 5x peak; 70% target utilization")
print(f"measured_container_rps={rate:.2f}; conservative_10_request_replica_count=ceil(peak/(measured_rps*0.70))={required}")
print(f"three-replica_safety_floor={baseline}; fixed_instances={nodes_fixed}")
print(f"fixed_peak_fleet_cloud_compute_only_usd_year={fixed_peak_fleet_usd:.2f}")
print(f"cloud_only_3_vm_baseline_plus_{burst_hours_per_year}h_burst_compute_only_usd_year={cloud_only_usd:.2f}")
print(f"hybrid_incremental_cloud_burst_compute_only_usd_year={hybrid_incremental_usd:.2f}; existing campus baseline cost excluded")
print("Excludes database, cluster control plane, load balancer, disks, backups, egress, taxes, support and campus-host cost. Synthetic password-hash throughput is an optimistic lower bound for database-backed result lookup; load-test the real query before production sizing.")
