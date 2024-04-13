#include <fakedyld/fakedyld.h>
#include <mount_args.h>

#define TEST_MOUNT_PREFIX "/fs/fake"

void clean_fakefs(char* rootdev) {
    char dev_rootdev[32];
    snprintf(dev_rootdev,32,"/dev/%s",rootdev);
    struct stat64 statbuf;
    apfs_mount_args_t arg = {
        dev_rootdev, 0, 1, 0
    };
retry_rootfs_mount:
    LOG("Test-mounting rootfs %s", dev_rootdev);
    int err = mount("apfs", TEST_MOUNT_PREFIX, 0, &arg);
    if (!err) {
      LOG("test mount rootfs OK");
    } else {
      LOG("test mount rootfs %s FAILED with err=%d!", dev_rootdev, errno);
      sleep(1);
      goto retry_rootfs_mount;
    }
    if ((err = stat64(TEST_MOUNT_PREFIX "/private/", &statbuf))) {
      LOG("stat %s FAILED with err=%d!", TEST_MOUNT_PREFIX "/private/", err);
      sleep(1);
      goto retry_rootfs_mount;
    } else {
      LOG("stat %s OK", TEST_MOUNT_PREFIX "/private/");
    }
    if (stat64(TEST_MOUNT_PREFIX "/usr/lib/nekojb", &statbuf) == 0) {
        LOG("Detected nekoJB fakefs");
    }
    int dirfd = open(TEST_MOUNT_PREFIX, O_RDONLY, 0);
    if (dirfd < 0) {
        panic("could not open test rootfs");
    }
    bool found_orig_fs = false;
    struct attrlist alist = { 0 };
    char abuf[2048];
    alist.commonattr = ATTR_BULK_REQUIRED;
    int count = fs_snapshot_list(dirfd, &alist, &abuf[0], sizeof (abuf), 0);
    if (count < 0) {
        panic("could not list snapshot on test rootfs");
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
        panic("could not find orig-fs snapshot, this function can only be used with a new enough fakefs");
    }
    LOG("found orig-fs");
    int ret = fs_snapshot_revert(dirfd, "orig-fs", 0);
    if (ret != 0) {
        panic("could not revert orig-fs");
    }
    LOG("reverted to orig-fs");
    ret = unmount(TEST_MOUNT_PREFIX, MNT_FORCE);
    if (ret != 0) {
        panic("could not unmount test rootfs");
    }
}
