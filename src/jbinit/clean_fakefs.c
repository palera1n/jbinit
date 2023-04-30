#include <jbinit.h>
#include <common.h>

void clean_fakefs(char* rootdev) {
    struct stat statbuf;
    struct apfs_mountarg arg = {
        rootdev, 0, 1, 0
    };
retry_rootfs_mount:
    LOG("Test-mounting rootfs %s\n", rootdev);
    int err = mount("apfs", "/mnt", 0, &arg);
    if (!err) {
      LOG("test mount rootfs OK\n");
    } else {
      LOG("test mount rootfs %s FAILED with err=%d!\n", rootdev, err);
      sleep(1);
      goto retry_rootfs_mount;
      // spin();
    }
    if ((err = stat("/mnt/private/", &statbuf))) {
      LOG("stat %s FAILED with err=%d!\n", "/mnt/private/", err);
      sleep(1);
      goto retry_rootfs_mount;
    } else {
      LOG("stat %s OK\n", "/mnt/private/");
    }
    int dirfd = open("/mnt", O_RDONLY, 0);
    if (dirfd < 0) {
        LOG("could not open test rootfs\n");
        spin();
    }
    bool found_orig_fs = false;
    struct attrlist alist = { 0 };
    char abuf[2048];
    alist.commonattr = ATTR_BULK_REQUIRED;
	int count = fs_snapshot_list(dirfd, &alist, &abuf[0], sizeof (abuf), 0);
    if (count < 0) {
        LOG("could not list snapshot on test rootfs\n");
        spin();
    } else if (count == 0) {
        goto list_done;
    } else {
        char *p = &abuf[0];
		for (int i = 0; i < count; i++) {
			char *field = p;
			uint32_t len = *(uint32_t *)field;
			field += sizeof (uint32_t);
			attribute_set_t attrs = *(attribute_set_t *)field;
			field += sizeof (attribute_set_t);

			if (attrs.commonattr & ATTR_CMN_NAME) {
				attrreference_t ar = *(attrreference_t *)field;
				char *name = field + ar.attr_dataoffset;
				field += sizeof (attrreference_t);
				if (strcmp("orig-fs", name) == 0) {
                    found_orig_fs = true;
                    goto list_done;
                }
			}
			p += len;
		}
    }
list_done:
    if (!found_orig_fs) {
        LOG("could not find orig-fs snapshot, this function can only be used with a new enough fakefs\n");
        spin();
    }
    LOG("found orig-fs\n");
    int ret = fs_snapshot_revert(dirfd, "orig-fs", 0);
    if (ret != 0) {
        LOG("could not revert orig-fs\n");
        spin();
    }
    LOG("reverted to orig-fs\n");
    ret = unmount("/mnt", MNT_FORCE);
    if (ret != 0) {
        LOG("could not unmount test rootfs\n");
        spin();
    }
}
