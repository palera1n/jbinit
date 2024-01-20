#include <stdio.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <xpc/connection.h>
#include <libjailbreak/libjailbreak.h>
#include <CoreFoundation/CoreFoundation.h>
#include <payload/payload.h>
#include <paleinfo.h>
#include <errno.h>
#include <spawn.h>
#include <pthread.h>
#include <spawn_private.h>
#include <sys/utsname.h>
#include <sys/spawn_internal.h>
#include <sys/kern_memorystatus.h>
#include <CoreFoundation/CoreFoundation.h>
#include <removefile.h>
#include <copyfile.h>

#include <sys/stat.h>

void overwrite_file(xpc_object_t xrequest, xpc_object_t xreply, struct paleinfo* pinfo) {
    size_t data_size;
    const char* path = xpc_dictionary_get_string(xrequest, "path");
    uint64_t mode = xpc_dictionary_get_uint64(xrequest, "mode");
    const uint8_t* data = xpc_dictionary_get_data(xrequest, "data", &data_size);
    if (!path) {
        xpc_dictionary_set_string(xreply, "errorDescription", "Missing file path");
        xpc_dictionary_set_int64(xreply, "error", EINVAL);
        return;
    }

    const char* allowlist[] = {
        "/var/jb/etc/apt/sources.list.d",
        "/etc/apt/sources.list.d",
        "/.installed_",
        "/var/jb/.installed_",
        NULL
    };

    bool is_allowlisted = false;
    for (uint16_t i = 0; allowlist[i] != NULL; i++) {
        if (!strncmp(allowlist[i], path, strlen(allowlist[i]))) {
            is_allowlisted = true;
        }
    }
    
    if (!is_allowlisted) {
        xpc_dictionary_set_string(xreply, "errorDescription", "requested file not in allowlist");
        xpc_dictionary_set_int64(xreply, "error", EPERM);
        return;
    }

    if (!data) {
        xpc_dictionary_set_string(xreply, "errorDescription", "Missing file data");
        xpc_dictionary_set_int64(xreply, "error", EINVAL);
        return;
    }

    int ret;
    struct stat st;
    if ((ret = lstat(path, &st))) {
        if (errno != ENOENT) {
            xpc_dictionary_set_string(xreply, "errorDescription", "failed to stat destination file");
            xpc_dictionary_set_int64(xreply, "error", errno);
            return;
        }
    }

    if (!S_ISREG(st.st_mode)) {
        xpc_dictionary_set_string(xreply, "errorDescription", "destination file exists and is not a regular file");
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }

    int fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, (mode_t)mode);
    if (fd == -1) {
        xpc_dictionary_set_string(xreply, "errorDescription", "failed to open destination file");
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }

    errno = 0;
    ssize_t didWrite = write(fd, data, data_size);
    if (didWrite == -1) {
        xpc_dictionary_set_string(xreply, "errorDescription", "Failed to write destination file");
        xpc_dictionary_set_int64(xreply, "error", errno);
        return;
    }

    xpc_dictionary_set_string(xreply, "message", "Success");
    xpc_dictionary_set_int64(xreply, "bytes-written", didWrite);

    close(fd);

    return;
}
