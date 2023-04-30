#include <jbinit.h>

int fs_snapshot_list(int dirfd, struct attrlist *alist, void *attrbuf, size_t bufsize, uint32_t flags) {
	if (flags != 0) {
		// errno = EINVAL;
		return -1;
	}

	return getattrlistbulk(dirfd, alist, attrbuf, bufsize, FSOPT_LIST_SNAPSHOT);
}

int fs_snapshot_create(int dirfd, const char *name, uint32_t flags) {
	return fs_snapshot(SNAPSHOT_OP_CREATE, dirfd, name, NULL, NULL, flags);
}

int fs_snapshot_delete(int dirfd, const char *name, uint32_t flags) {
	return fs_snapshot(SNAPSHOT_OP_DELETE, dirfd, name, NULL, NULL, flags);
}

int fs_snapshot_rename(int dirfd, const char *old, const char *new, uint32_t flags) {
	return fs_snapshot(SNAPSHOT_OP_RENAME, dirfd, old, new, NULL, flags);
}

int fs_snapshot_revert(int dirfd, const char *name, uint32_t flags) {
	return fs_snapshot(SNAPSHOT_OP_REVERT, dirfd, name, NULL, NULL, flags);
}

int fs_snapshot_root(int dirfd, const char *name, uint32_t flags) {
	return fs_snapshot(SNAPSHOT_OP_ROOT, dirfd, name, NULL, NULL, flags);
}

int fs_snapshot_mount(int dirfd, const char *dir, const char *snapshot, uint32_t flags) {
	return fs_snapshot(SNAPSHOT_OP_MOUNT, dirfd, snapshot, dir, NULL, flags);
}
