#include <stdio.h>
#include <mach-o/loader.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* copied from plooshfinder */
bool macho_check(const void *buf) {
    uint32_t *buf_ptr = (uint32_t *) buf;
    uint32_t magic = buf_ptr[0];

    if (magic == 0xfeedfacf || magic == 0xbebafeca) {
        return true;
    }
    
    return false;
}

/* this function retuns 0 on failure */
uint32_t macho_get_platform(const void *buf) {
    if (!macho_check(buf)) {
        errno = EINVAL;
        return 0;
    }

    const struct load_command *after_header = buf + sizeof(struct mach_header_64);
    const struct mach_header_64 *header = buf;

    for (int i = 0; i < header->ncmds; i++) {
        if (after_header->cmd == LC_BUILD_VERSION) {
            const struct build_version_command *cmd = (const struct build_version_command *) after_header;

            if (cmd->platform > 20) {
                errno = EINVAL;
                return 0;
            }

            return cmd->platform; 
        }

        after_header = (const struct load_command *) ((const char *) after_header + after_header->cmdsize);
    }

    errno = ENOTSUP;
    return 0;
}

int get_platform() {
    struct stat st;
    int ret = stat("/usr/lib/dyld", &st);
    if (ret) return ret;
    int fd_dyld = open("/usr/lib/dyld", O_RDONLY);
    if (fd_dyld == -1) return -1;
    void* dyld_buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd_dyld, 0);
    if (dyld_buf == MAP_FAILED) return -1;
    uint32_t platform = macho_get_platform(dyld_buf);
    if (!platform) return -1;
    munmap(dyld_buf, st.st_size);
    return (int)platform;
}
