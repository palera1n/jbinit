#include <fakedyld/fakedyld.h>

void get_pinfo(struct paleinfo* pinfo_p) {
    int rmd0 = open(RAW_RAMDISK, O_RDONLY, 0);
    if (rmd0 == -1) {
        LOG("could not get paleinfo!");
        spin();
    }
    off_t off = lseek(rmd0, 0, SEEK_SET);
    if (off == -1) {
        LOG("failed to lseek ramdisk to 0");
        spin();
    }
    uint32_t pinfo_off;
    ssize_t didRead = read(rmd0, &pinfo_off, sizeof(uint32_t));
    if (didRead != (ssize_t)sizeof(uint32_t)) {
        LOG("read %lld bytes does not match expected %llu bytes\n", didRead, sizeof(uint32_t));
        spin();
    }
    off = lseek(rmd0, pinfo_off, SEEK_SET);
    if (off == -1) {
        LOG("failed to lseek ramdisk to %lld", pinfo_off);
        spin();
    }
    didRead = read(rmd0, pinfo_p, sizeof(struct paleinfo));
    if (didRead != (ssize_t)sizeof(struct paleinfo)) {
        LOG("read %lld bytes does not match expected %llu bytes", didRead, sizeof(struct paleinfo));
        spin();
    }
    if (pinfo_p->magic != PALEINFO_MAGIC) {
        LOG("Detected corrupted paleinfo!");
        spin();
    }
    if (pinfo_p->version != PALEINFO_VERSION) {
        LOG("unexpected paleinfo version: %u, expected %u", pinfo_p->version, PALEINFO_VERSION);
        spin();
    }
    close(rmd0);
}
