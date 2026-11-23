# A5 - Design cloud deployment and orchestration (4 marks)

IaaS gives the university VMs, disks and networks to administer; PaaS supplies a managed runtime/database/queue while the team owns application behavior; SaaS is a finished service such as hosted learning software, with little infrastructure control. For results, keep identity and authoritative records in the university-controlled database. Use a managed PostgreSQL primary with private endpoints, backups and encryption; put stateless portal replicas in a managed Kubernetes cluster across availability zones, behind TLS load balancer/WAF. Cache static files and read-only result metadata only after checking invalidation and privacy. Autoscale workers on CPU and request/queue metrics; reserve warm minimum replicas before result release.

Pods are the scheduling unit; a Deployment maintains desired replicas; a Service gives stable discovery/load balancing; requests guide placement and HPA utilization while limits cap container usage. Readiness probes prevent unready pods entering endpoints. A failed pod is replaced by its ReplicaSet controller; a node loss requires another healthy node and capacity. HPA changes the replica target from observed metrics. The provided `TaskA5/deployment.yaml` completes the worksheet's three replicas, label selector and CPU/memory request/limits; `hpa.yaml` shows the autoscaling policy. For a private lab cluster, scale replicas manually or use metrics-server; cloud cluster-autoscaler/node autoscaling is separately needed when the nodes are full.

### SIMATS-style pseudocode: pre-release capacity response

```text
ALGORITHM 1: PREWARM RESULT PORTAL
BEGIN
    INPUT: predictedPeakRPS, measuredSafeRPSPerPod, targetUtilization
    SET requiredPods <- CEILING(predictedPeakRPS / (measuredSafeRPSPerPod * targetUtilization))
    IF requiredPods < 3 THEN
        SET requiredPods <- 3
    END IF
    SET portalDeployment.replicas <- requiredPods
    WAIT until all requiredPods pass readinessProbe
    IF readyPods < requiredPods THEN
        SEND capacityAlert to portal-ops
    END IF
    OUTPUT: readyPods, predictedPeakRPS, capacityAlertStatus
END
```

For n replicas, controller reconciliation and health checking are O(n); the arithmetic calculation is O(1). This is a simple scheduled warm-up rule, not the built-in HPA algorithm; real HPA uses observed metrics and stabilization policies.

## Fixed VM alternative

Pre-size a fixed VM fleet to result-day peak and it responds immediately, without waiting for an HPA metric and image pull. But it wastes those CPU/RAM-hours on the other 364 days and must cover worst-case peak, often with safety margin. Autoscaling leaves a small always-on baseline and adds replicas as demand climbs, reducing idle resource hours; its risks are cold image pulls, slow database connections and metric lag. Mitigate with scheduled pre-scaling before the published result time, warmed images/connections and a load rehearsal. Complexity for fixed sizing is O(1) at runtime but repeated manual re-sizing; autoscaling adds policy/metric setup (O(k) signals) and automated per-replica control, but more failure modes.

**Design choice:** hybrid. Run routine traffic and a minimum resilient service on existing campus hosts; use a managed cloud burst tier, load balancer and managed DB/read replica for the short result-day flash crowd. Keep student data encrypted, access-audited and region-approved. Public cloud alone simplifies elasticity but has recurring operating/data-transfer charges and moves custody to provider contracts; private-only cannot absorb the specified spike without newly purchased capacity. The user's constraint says no new servers, so measure available private capacity and reserve cloud capacity ahead of the release.

