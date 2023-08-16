#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <paleinfo.h>
#include <errno.h>

int get_pinfo(struct paleinfo* pinfo_p) {
    int ret;
    ssize_t didRead;
    uint32_t ramdisk_size;

    int fd = open("/dev/rmd0", O_RDONLY);
    if (fd == -1) return -1;

    didRead = read(didRead, &ramdisk_size, 4);
    if (didRead == -1) return -1;
    
    ret = lseek(fd, (off_t)ramdisk_size, SEEK_SET);
    if (ret == -1) return -1;

    didRead = read(fd, pinfo_p, sizeof(struct paleinfo));

    if (didRead == -1) return -1;
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

    didRead = read(didRead, &ramdisk_size, 4);
    if (didRead == -1) return -1;
    
    ret = lseek(fd, (off_t)ramdisk_size, SEEK_SET);
    if (ret == -1) return -1;

    return (int)write(fd, pinfo_p, sizeof(struct paleinfo));
}
