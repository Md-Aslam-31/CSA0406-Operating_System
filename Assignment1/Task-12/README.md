# Assignment 1 — Task 12

**Mohamed Aslam H · 192572147 · CSE (AI) · 2025**

## Task 12 — Day-4 design challenge: MODIFY

**A. Impact:** Home readings add hostile/unreliable WAN paths, heterogeneous devices, remote updates, identity and consent concerns. Cloud records add distributed consistency, availability-zone failure, data residency/retention and partner access control. Monitor RTOS type remains suitable; home-device security, mobile/network services and database topology are affected.

**B. Original design:** Local hospital monitors still meet their intended deadline if alarms remain locally generated. A tablet or home device using a remote cloud round trip cannot guarantee a 10-ms alarm due to variable internet delay and outages. The clustered database concept remains useful but a single hospital-local cluster alone does not provide multi-hospital cloud availability. Security must extend to device identity, transport, tenant separation, audit and revocation.

**C. Adaptation:** Modify rather than replace. Home units use embedded RTOS or constrained mobile/embedded OS with local threshold detection, buffering and authenticated signed updates. Send readings over mutually authenticated TLS to an ingestion gateway; queue, validate, timestamp and deduplicate records. Host the shared record service across independent cloud failure domains with encrypted backups, regional recovery, tenant/role-based access, audit and explicit consistency semantics. Hospital bedside alarms remain local. Partner systems use versioned APIs and scoped identity federation; allow offline workflows and later reconciliation.

**D. Defence:** This preserves deterministic local behavior and existing desktop/imaging tiers while adding distributed protections where the changed trust boundary requires them. The trade-off is greater identity, monitoring, governance and operational complexity, plus cloud/network cost and possible replication lag. WAN outages must reduce remote visibility, not disable local alarm response. Validate remote alert latency as a service objective, not a hard local-control guarantee; perform threat modeling, penetration testing, disaster recovery and clinical usability testing before deployment.

See the [complete report](../REPORT.md) for the full submission and references.
