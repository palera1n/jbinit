#include <jbloader.h>
#include <mach-o/loader.h>

int get_dyld_platform() {
    int dyld_fd = open("/usr/lib/dyld", O_RDONLY);
    if (dyld_fd == -1) {
        printf("cannot open dyld: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    struct stat st;
	int stat_ret = fstat(dyld_fd, &st);
    if (stat_ret) {
        printf("cannot fstat dyld: %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    void* dyld_buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE | MAP_FILE, dyld_fd, 0);
    if (dyld_buf == NULL) {
        printf("failed to map dyld: %d (%s)\n", errno, strerror(errno));
        close(dyld_fd);
        return -1;
    }
    void *after_header = (char *)dyld_buf + 0x20;
    void *before_platform = after_header;
    int platform;

    while (*(uint32_t *)before_platform != 0x32) {
        before_platform += 4;
    }

    if (*(uint8_t *)before_platform == 0x32) {
        uint32_t *platform_ptr = (uint32_t *)before_platform + 2;
        platform = *platform_ptr;
    }
    munmap(dyld_buf, st.st_size);
    close(dyld_fd);

    if (platform > PLATFORM_SEPOS) {
        printf("Unknown platform!\n");
        return -1;
    }

    switch (platform) {
        case PLATFORM_MACOS:
            puts("Platform is macOS");
            break;
        case PLATFORM_IOS:
            puts("Platform is iOS");
            break;
        case PLATFORM_TVOS:
            puts("Platform is tvOS (audioOS)");
            break;
        case PLATFORM_BRIDGEOS:
            puts("Platform is bridgeOS");
            break;
        default:
            printf("unsupported platform %d\n", platform);
            break;
    }

    return platform;
}
