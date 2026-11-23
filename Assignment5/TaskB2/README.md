# B2 - Containerise the portal

`Dockerfile` completes the worksheet's three TODOs: sets `/app`, copies application source, runs as `portal`, documents port 8000 and starts Gunicorn on `0.0.0.0:8000`. It uses `python:3.12-slim` and installs dependency layer before app source. `compose.yaml` starts the portal, PostgreSQL and the LMS extension; database files live in named volume `result-db`, the health check gates app start, and each app/database container has CPU and memory caps. The included SQL is synthetic demonstration data only.

From the Assignment5 root:

```bash
docker compose -f TaskB2/compose.yaml up --build -d
curl http://localhost:8000/health
curl http://localhost:8000/results/10001
docker compose -f TaskB2/compose.yaml ps
docker stats --no-stream
docker history result-portal:compose
docker image inspect result-portal:compose --format '{{.Size}}'
docker exec "$(docker compose -f TaskB2/compose.yaml ps -q portal)" whoami
docker compose -f TaskB2/compose.yaml down
```

Compare full Python base, slim and multi-stage builds with the evidence runner. The minimal app's Python base/dependencies dominate; a multi-stage wheel builder does not guarantee a smaller image when runtime packages are pure Python, so report the measured sizes instead of assuming a saving. Docker layer history identifies which step contributes data. `--cpus=0.5 --memory=512m` maps to cgroup CPU/memory controls; `docker stats` gives live usage and hard limits.

