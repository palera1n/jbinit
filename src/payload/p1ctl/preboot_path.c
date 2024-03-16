#include <payload/payload.h>
#include <xpc/xpc.h>
#include <xpc/connection.h>
#include <xpc/private.h>
#include <libjailbreak/libjailbreak.h>

int preboot_path_main(int __unused argc, char* __unused argv[]) {
    int retval = 0;
    P1CTL_UPCALL_JBD_WITH_ERR_CHECK(xreply, JBD_CMD_GET_PINFO_FLAGS);
    const char* path = NULL;
    if ((path = xpc_dictionary_get_string(xreply, "path"))) {
        puts(path);
    } else {
        uint64_t error = xpc_dictionary_get_int64(xreply, "error");
        if (error) {
            fprintf(stderr, "get preboot path failed: %d (%s)\n", (int)error, xpc_strerror((int)error));
            return -1;
        }
        retval = -1;
    }
    xpc_release(xreply);
    return retval;
}
