#include <fakedyld/fakedyld.h>

void get_pinfo(struct paleinfo* pinfo_p) {
    int rmd0 = open(RAW_RAMDISK, O_RDONLY, 0);
    if (rmd0 == -1) {
        panic("could not get paleinfo!");
    }
    off_t off = lseek(rmd0, 0, SEEK_SET);
    if (off == -1) {
        panic("failed to lseek ramdisk to 0");
    }
    uint32_t pinfo_off;
    ssize_t didRead = read(rmd0, &pinfo_off, sizeof(uint32_t));
    if (didRead != (ssize_t)sizeof(uint32_t)) {
        panic("read %lld bytes does not match expected %llu bytes\n", didRead, sizeof(uint32_t));
    }
    off = lseek(rmd0, pinfo_off, SEEK_SET);
    if (off == -1) {
        panic("failed to lseek ramdisk to %lld", pinfo_off);
    }
    didRead = read(rmd0, pinfo_p, sizeof(struct paleinfo));
    if (didRead != (ssize_t)sizeof(struct paleinfo)) {
        panic("read %lld bytes does not match expected %llu bytes", didRead, sizeof(struct paleinfo));
    }
    if (pinfo_p->magic != PALEINFO_MAGIC) {
        panic("Detected corrupted paleinfo!");
    }
    if (pinfo_p->version != PALEINFO_VERSION) {
        panic("unexpected paleinfo version: %u, expected %u", pinfo_p->version, PALEINFO_VERSION);
    }
    close(rmd0);
}
