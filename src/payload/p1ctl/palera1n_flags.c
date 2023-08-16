#include <payload/payload.h>
#include <xpc/xpc.h>
#include <xpc/connection.h>
#include <libjailbreak/libjailbreak.h>
#include <xpc/private.h>

int palera1n_flags_main(int argc, char* argv[]) {
    int ch;
    bool stringify = false;
    while ((ch = getopt(argc, argv, "s")) != -1) {
        switch(ch) {
            case 's':
                stringify = true;
                break;
            default:
                return -1;
        }
    }

    P1CTL_UPCALL_JBD_WITH_ERR_CHECK(xreply, JBD_CMD_GET_PINFO_FLAGS);
    uint64_t error = xpc_dictionary_get_int64(xreply, "error");
    if (error) {
        fprintf(stderr, "get pinfo failed: %d (%s)\n", (int)error, xpc_strerror((int)error));
        return -1;
    }

    uint64_t pflags = xpc_dictionary_get_uint64(xreply, "flags");
    xpc_release(xreply);
    if (!stringify) {
        printf("0x%llx\n", pflags);
    } else {
        for (uint64_t i = 0; i < 63; i++) {
            if (pflags & (UINT64_C(1) << i)) {
                printf("%s\n", jailbreak_str_pinfo_flag(pflags & (UINT64_C(1) << i)));
            }
        }
    }
    return 0;
}
