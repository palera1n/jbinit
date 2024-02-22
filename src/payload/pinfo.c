#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <paleinfo.h>
#include <errno.h>
#include <payload/payload.h>

int get_pinfo(struct paleinfo* pinfo_p) {
    int ret;
    ssize_t didRead;
    uint32_t ramdisk_size;

    int fd = open("/dev/rmd0", O_RDONLY);
    if (unlikely(fd == -1)) return -1;

    didRead = read(fd, &ramdisk_size, 4);
    if (unlikely(didRead == -1)) return -1;

    ret = (int)lseek(fd, (off_t)ramdisk_size, SEEK_SET);
    if (unlikely(ret == -1)) return -1;

    didRead = read(fd, pinfo_p, sizeof(struct paleinfo));

    if (unlikely(didRead == -1)) return -1;
    else if (didRead != sizeof(struct paleinfo)) {
        errno = EAGAIN;
        return -1;
    }
    return 0;
}

int set_pinfo(struct paleinfo* pinfo_p) {
    int ret;
    ssize_t didRead;
    uint32_t ramdisk_size;

    int fd = open("/dev/rmd0", O_RDWR);
    if (fd == -1) return -1;

    didRead = read(fd, &ramdisk_size, 4);
    if (unlikely(didRead == -1)) return -1;

    ret = (int)lseek(fd, (off_t)ramdisk_size, SEEK_SET);
    if (unlikely(ret == -1)) return -1;

    ret = (int)write_fdout(fd, pinfo_p, sizeof(struct paleinfo));
    close(fd);
    return ret;
}
