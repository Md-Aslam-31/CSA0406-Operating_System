# A3 - Evaluate virtualisation (5 marks)

## Hypervisors

A type-1 hypervisor runs on hardware or as a privileged host-kernel hypervisor layer: KVM uses Linux's kernel virtualization support, while ESXi, Xen and Hyper-V provide their own host/management architecture. A type-2 hypervisor such as VirtualBox or VMware Workstation is an application on a general-purpose host OS; it schedules guest vCPUs through the host. Full virtualization presents virtual hardware so an unmodified guest can run; hardware virtualization extensions (Intel VT-x or AMD-V) trap/switch privileged operations. Paravirtual drivers (for example virtio) let guests use efficient hypervisor-aware I/O without changing application code.

## Benefits and costs

VMs isolate kernels and guest OS failure domains, consolidate several services per host, support snapshots for controlled test/rollback and can live-migrate when storage/networking and hypervisor compatibility permit. Costs include memory for every guest kernel and cache, virtual-device and I/O overhead, image/backup storage, and the need to patch both host and guest. A database should be its own VM or managed database boundary rather than sharing a failure domain with the web worker if stronger isolation and independent recovery justify the RAM.

The practical comparison in TaskB3 boots the same sample Flask application on the Linux host, in Docker and in a QEMU guest. It records whether KVM acceleration exists; software-emulated TCG measurements are explicitly not treated as hardware-assisted production performance.

## Type-2 alternative

Type-2 is useful for student laptops, compatibility and quick local labs. A CPU-bound guest pays extra host scheduling/context-switch cost, although VT-x/AMD-V makes the cost much lower than instruction emulation. I/O-bound work also crosses virtual device and host filesystem/network paths and can suffer contention. Memory includes host OS + hypervisor + each guest OS, while KVM shares Linux host page cache/kernel services differently. For production, use KVM/type-1 under a managed cluster (or cloud VM) for predictable host control, migrations and monitoring; do not use a desktop VirtualBox host as the result portal's single production server.

Complexity comparison: one type-2 VM setup is O(1) operator workflow and easy to clone, but managing n independent desktop hosts is O(n) updates. A type-1 cluster has higher initial provisioning and network/storage design, but central APIs make n-host scheduling and migration tractable; VM count does not eliminate per-guest patch work.

