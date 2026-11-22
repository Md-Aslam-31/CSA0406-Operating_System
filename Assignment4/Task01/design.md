# Task 1 - File-system design review

## Problems in the proposal

1. A flat namespace allows users to choose the same filename; every lookup may scan/index among 12,000 entries, and the naming rule gives no owner context.
2. A directory entry can become a hotspot and is difficult to delegate or browse. Search is O(n) in a simple linear directory, even when a filesystem index improves lookup.
3. Contiguous allocation has excellent sequential reads and direct block calculation, but growth may need a larger adjacent extent or relocation; free space can be externally fragmented.
4. No authentication or permission checks expose private records to any student who guesses a name or reads the server volume.
5. There is no explicit backup, versioning, retention or tested restore plan. One accidental deletion or server failure can lose everything.
6. Group work requires uncontrolled copying, creating stale duplicates and unclear ownership.

## Improved hierarchy and naming

```text
/srv/student-files/
  19/25/72/192572147/
    profile.json
    coursework/CSA0406/assignment-04/
    uploads/2026/11/
  /groups/{course-id}/{project-id}/
```

The numeric register number is the stable student key; validate it at account creation and never use a user-chosen name as the sole identity. Store original filenames as metadata under a server-generated object ID, reject path separators and normalize Unicode. Keep per-user quotas, timestamps, owner/group, MIME type, size, checksum, retention class and version ID. Each personal directory is owned by the student and service account with mode 0700; service admins use audited break-glass access. Course staff get only course-scoped read/write ACLs. Group directories have a course group ACL; a shared project directory or ACL grants members access without copying files. Use an acyclic directory tree and symbolic links only as convenience references, never as access control.

Back up metadata and data to a separate failure domain, keep immutable/versioned copies, encrypt backups, test restores quarterly and define retention/deletion. Use an extent-based filesystem with journaling and checksums where supported. Snapshot before upgrades and monitor capacity/inode use.

### Privacy and sharing (under 100 words)

Every personal directory is owned by its student and has private default permissions; the service checks identity and ACLs on every open. Staff receive course-scoped access only when required and access is audited. A project group gets a dedicated shared directory with an explicit membership ACL, so collaborators edit the same current files without exposing personal folders. Backups preserve permissions and are encrypted. Filenames do not grant access, and changing a link cannot bypass server-side authorization.

### Alternative: cloud drive

Use institution-managed tenant accounts, single sign-on/MFA, group drives, default-private personal spaces, retention/versioning, audit logs, region and legal-hold controls. A directory/search index or provider API finds an object by key; opening adds network latency and authentication, but provider caching/CDN can make repeated access fast across campuses. Storage cost is the sum of live objects, versions, trash/retention and replicated provider storage; collaboration links usually reference one object, while offline sync or explicit copies consume additional space. The college loses direct control of disks and filesystem internals, and must govern provider administrators, region, encryption keys, export and contract terms. Measure search p95 and open p95 from campus and home before migration.
