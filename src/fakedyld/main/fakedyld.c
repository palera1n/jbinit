/*
 * jbinit 3
 * made by nick chan
*/

#include <fakedyld/fakedyld.h>
#include <mount_args.h>

int main(int argc, char* argv[], char* envp[], char* apple[]) {
    int console_fd = open("/dev/console", O_RDWR | O_CLOEXEC | O_SYNC, 0);
    if (console_fd == -1) exit(-1);
    set_fd_console(console_fd);
    printf("argc: %d\n", argc);
    for (int i = 0; argv[i] != NULL; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    for (int i = 0; envp[i] != NULL; i++) {
        printf("envp[%d]: %s\n", i, envp[i]);
    }
    for (int i = 0; apple[i] != NULL; i++) {
        printf("apple[%d]: %s\n", i, apple[i]);
    }
    int ret = 0;
    struct paleinfo pinfo;
    get_pinfo(&pinfo);
    printf(
        "kbase: 0x%llx\n"
        "kslide: 0x%llx\n"
        "flags: 0x%llx\n"
        "rootdev: %s\n"
    ,pinfo.kbase,pinfo.kslide,pinfo.flags,pinfo.rootdev
    );

    pinfo_check(&pinfo);
    struct systeminfo sysinfo;
    systeminfo(&sysinfo);
    LOG("Parsed kernel version: Darwin %d.%d.%d xnu: %d",
        sysinfo.osrelease.darwinMajor,
        sysinfo.osrelease.darwinMinor,
        sysinfo.osrelease.darwinRevision,
        sysinfo.xnuMajor
    );
    LOG("boot-args: %s", sysinfo.bootargs);
    LOG("Kernel version (raw): %s", sysinfo.kversion);
    memory_file_handle_t payload;
    memory_file_handle_t payload15_dylib;
#if 0
    if (pinfo.flags & palerain_option_bind_mount) {
        read_file("/payload.dylib", &payload15_dylib);
        read_file("/payload", &payload);
    }
#endif
    if (sysinfo.xnuMajor < 7938) mountroot(&pinfo, &sysinfo);
    prepare_rootfs(&sysinfo, &pinfo);
    memory_file_handle_t dyld_handle;
    read_file("/usr/lib/dyld", &dyld_handle);
    check_dyld(&dyld_handle);
    int platform = get_platform(&dyld_handle);
    init_cores(&sysinfo, platform);
    patch_dyld(&dyld_handle, platform);
    write_file("/cores/usr/lib/dyld", &dyld_handle);
    // set_fd_console(1);
    // close(console_fd);
    /*
        argv0/execve_buffer -> =============================
                                      "/sbin/launchd"
                     envp0 ->  =============================
                                "DYLD_INSERT_LIBRARIES..."
                     envp0 ->  =============================
                                   "JB_PINFO_FLAGS=0x123"
                     envp0 ->  =============================
                                   "JB_ROOT_PATH=/var/jb"
            argv, &argv[0] ->  =============================
                                        argv0 (ptr)
                   &argv[1] -> =============================
                                        nullptr
             envp, &envp[0] -> =============================
                                        envp0 (ptr)
                  &envp[1] ->  =============================
                                         nullptr
                               =============================

    */
    /*if (sysinfo.osrelease.darwinMajor > 19) {*/
    #define INSERT_DYLIB "DYLD_INSERT_LIBRARIES=/cores/payload.dylib"
    
        char* JBRootPathEnv;
        if (pinfo.flags & palerain_option_rootful) {
            JBRootPathEnv = "JB_ROOT_PATH=/";
        } else {
            JBRootPathEnv = "JB_ROOT_PATH=/var/jb"; /* will fixup to preboot path in sysstatuscheck stage */
        }

        char pinfo_buffer[50];
        snprintf(pinfo_buffer, 50, "JB_PINFO_FLAGS=0x%llx", pinfo.flags);
        char* execve_buffer = mmap(NULL, 0x4000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        if (execve_buffer == MAP_FAILED) {
            printf("mmap for execve failed\n");
            spin();
        }
        char* launchd_argv0 = execve_buffer;
        char* launchd_envp0 = (execve_buffer + sizeof("/sbin/launchd"));
        char* launchd_envp1 = (launchd_envp0 + sizeof(INSERT_DYLIB));
        char* launchd_envp2 = (launchd_envp1 + strlen(pinfo_buffer) + 1);
        memcpy(launchd_argv0, "/sbin/launchd", sizeof("/sbin/launchd"));
        memcpy(launchd_envp0, INSERT_DYLIB, sizeof(INSERT_DYLIB));
        memcpy(launchd_envp1, pinfo_buffer, strlen(pinfo_buffer) + 1);
        memcpy(launchd_envp2, JBRootPathEnv, strlen(JBRootPathEnv) + 1);
        char** launchd_argv = (char**)((char*)launchd_envp2 + strlen(JBRootPathEnv) + 1);
        char** launchd_envp = (char**)((char*)launchd_argv + (2*sizeof(char*)));
        launchd_argv[0] = launchd_argv0;
        launchd_argv[1] = NULL;
        launchd_envp[0] = launchd_envp0;
        launchd_envp[1] = launchd_envp1;
        launchd_envp[2] = launchd_envp2;
        launchd_envp[3] = NULL;
        LOG("(launchd) argv0: %s, envp0: %s, argv[0]: %s, envp[0]: %s, envp[1]: %s, envp[2]: %s", launchd_argv0, launchd_envp0, launchd_argv[0], launchd_envp[0], launchd_envp[1], launchd_envp[2]);
        ret = execve(launchd_argv0, launchd_argv, launchd_envp);
    /*}*/
    LOG("execve failed with error=%d", ret);
    spin();
    __builtin_unreachable();
}

void spin() {
    LOG("jbinit DIED!");
    while(true) {
        sleep(5);
    }
}
