#include <jbloader.h>

void print_flag_text(uint32_t flags, const char* prefix, const char* (strflags)(checkrain_option_t opt)) {
    for (uint8_t bit = 0; bit < 32; bit++) {
        if (checkrain_option_enabled(flags, (1 << bit))) {
            char printbuf[0x30];
            const char* opt_str = strflags(1 << bit);
            if (opt_str == NULL) {
                snprintf(printbuf, 0x30, "%s_option_unknown_%" PRIu8, prefix, bit);
            }
            printf("%s ", opt_str == NULL ? printbuf : opt_str);
        }
    }
}

int palera1n_flags_main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("0x%x\n", pinfo.flags);
        return 0;
    }
    errno = 0;
    char* endptr;
    uint32_t new_flags = strtoul(argv[1], &endptr, 0);
    if (errno || endptr == argv[1]) {
        fprintf(stderr, "Invalid number entered: %d (%s)", errno, strerror(errno));
        return 1;
    }
    pinfo.flags = new_flags;
    int rd_fd = open("/dev/rmd0", O_RDWR);
    if (rd_fd == -1) {
        fprintf(stderr, "failed to open ramdisk file: %d (%s)\n", errno, strerror(errno));
        return errno;
    }
    uint32_t rd_end = 0;
    read(rd_fd, &rd_end, 4);
    lseek(rd_fd, (long)rd_end + 0x1000L, SEEK_SET);
    write(rd_fd, &pinfo, sizeof(struct paleinfo));
    close(rd_fd);
    return 0;
}

int checkra1n_flags_main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("0x%x\n", info.flags);
        return 0;
    }
    errno = 0;
    char* endptr;
    uint32_t new_flags = strtoul(argv[1], &endptr, 0);
    if (errno || endptr == argv[1]) {
        fprintf(stderr, "Invalid number entered: %d (%s)\n", errno, strerror(errno));
        return 1;
    }
    info.flags = new_flags;
    int rd_fd = open("/dev/rmd0", O_RDWR);
    if (rd_fd == -1) {
        fprintf(stderr, "failed to open ramdisk file: %d (%s)\n", errno, strerror(errno));
        return errno;
    }
    uint32_t rd_end = 0;
    read(rd_fd, &rd_end, 4);
    lseek(rd_fd, (long)rd_end, SEEK_SET);
    write(rd_fd, &info, sizeof(struct kerninfo));
    close(rd_fd);
    return 0;
}

