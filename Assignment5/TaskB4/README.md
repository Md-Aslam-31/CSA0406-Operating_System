# B4 - Final engineering decision

The CI extension deploys a second `lms` service with its own Deployment, Service, two replicas, 200m CPU / 128Mi requests and 400m CPU / 256Mi limits. It shares the same stateless demo image but has a separate service identity and `APP_NAME`; production LMS and portal should use separate code, credentials and data services. B4's capacity script reads measured direct/container/VM request rates, computes average student login rate, a documented flash-crowd multiplier, utilization headroom and required app replicas. It also tabulates a clearly scoped, illustrative USD compute estimate.

**Engineering choice:** reuse the university's current host capacity for routine load, containerize both stateless services, and add public-cloud capacity before each result announcement. No new campus servers are purchased. Database is backed up, access-controlled and region-approved; only minimum necessary data crosses the hybrid link. Run a pre-release capacity test because CI results on a synthetic route can overstate real database-backed student-login capacity.

