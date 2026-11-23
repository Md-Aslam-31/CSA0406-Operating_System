#!/usr/bin/env python3
"""Small standard-library GET load generator; reports completed request rate."""
import concurrent.futures
import sys
import threading
import time
import urllib.request

url = sys.argv[1]
duration = float(sys.argv[2]) if len(sys.argv) > 2 else 15.0
workers = int(sys.argv[3]) if len(sys.argv) > 3 else 25
stop = threading.Event()
ok = 0
errors = 0
lock = threading.Lock()


def worker():
    global ok, errors
    while not stop.is_set():
        try:
            with urllib.request.urlopen(url, timeout=3) as response:
                response.read()
                with lock:
                    if response.status == 200:
                        ok += 1
                    else:
                        errors += 1
        except Exception:
            with lock:
                errors += 1


start = time.perf_counter()
with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
    futures = [pool.submit(worker) for _ in range(workers)]
    time.sleep(duration)
    stop.set()
    for future in futures:
        future.result()
elapsed = time.perf_counter() - start
print(f"url={url} duration_s={elapsed:.3f} concurrency={workers} ok={ok} errors={errors} requests_per_second={ok/elapsed:.2f}")
if errors:
    raise SystemExit(1)
