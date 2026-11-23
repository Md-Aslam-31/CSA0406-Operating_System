# A4 - Evaluate containerisation (4 marks)

Linux namespaces give a container a scoped view of process IDs, mount points, network interfaces, IPC, hostnames and users. cgroups account for and constrain CPU, memory and I/O. Images are immutable, content-addressed layer stacks: a writable container layer sits above shared image layers. Containers share the host kernel; they are not small VMs. A container can start in seconds and commonly uses tens to hundreds of MiB for a Python process, whereas a VM boots a guest kernel and uses additional guest memory.

In `TaskB2/Dockerfile`, dependencies are copied before source so code edits do not invalidate the dependency layer. The image declares port 8000, creates a non-root `portal` user and launches Gunicorn bound on all container interfaces. `Dockerfile.multistage` builds wheels in a builder stage and copies them into a slim runtime; its measured image size is compared to `Dockerfile.full`. Compose runs portal and PostgreSQL with a named persistent volume, health check, restart policy and CPU/memory caps. `docker history` shows image layers and `docker stats` shows the cgroup usage/limits.

Containers give fast density and reproducible deployments but share one kernel; a kernel escape or host misconfiguration crosses container boundaries. Use read-only filesystems, no privileged mode, a non-root UID, dropped capabilities, patched/pinned images and network policies. VMs give a stronger separate-kernel boundary at higher boot/memory/disk overhead. For this portal, Kubernetes containers run the stateless web tier, while the database uses a managed data service with backups and encryption (or a stateful VM for the private-site alternative).

## VM alternative and complexity

Running web and DB in separate VMs increases independent kernel isolation, but consumes two guest OS footprints and adds minutes to cold boot/provisioning in this lab design versus seconds for a container. Creating n VMs takes O(n) guest images and OS patch streams; n containers from one image use O(1) image publishing plus O(n) runtime scheduling records. The security boundary is stronger between VM kernels, while container isolation depends on the shared host kernel and correctly configured namespaces/cgroups. Measure actual memory from `/proc`/`docker stats`; do not present a base-image download size as live RSS.

