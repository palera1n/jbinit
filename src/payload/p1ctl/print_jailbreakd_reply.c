#include <xpc/xpc.h>
#include <xpc/private.h>
#include <stdio.h>

#define PRINT_XPC(obj) do { char* desc=xpc_copy_description(obj);puts(desc);free(desc); } while (0)

int print_jailbreakd_reply(xpc_object_t xreply) {
    int retval = 0;
    int error = (int)xpc_dictionary_get_int64(xreply, "error");
    const char* _Nullable errorDescription = xpc_dictionary_get_string(xreply, "errorDescription");
    if (error || errorDescription) {
        if (error && !errorDescription) {
            fprintf(stderr, "Error: %d (%s)\n", error, xpc_strerror(error));
        } else if (!error && errorDescription) {
            fprintf(stderr, "Error: %s\n", errorDescription);
        } else if (error && errorDescription) {
            fprintf(stderr, "Error: %s: %d (%s)\n", errorDescription, error, xpc_strerror(error));
        }
        retval = -1;
    }

    const char* _Nullable message;
    if ((message = xpc_dictionary_get_string(xreply, "message"))) {
        fprintf(stderr, "%s\n", message);
    }

    return retval;
}
