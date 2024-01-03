#include <payload/payload.h>
#include <xpc/xpc.h>
#include <xpc/connection.h>
#include <libjailbreak/libjailbreak.h>
#include <unistd.h>

#define PRINT_XPC(obj) do { char* desc=xpc_copy_description(obj);puts(desc);free(desc); } while (0)

int bootstrap_main(int argc, char* argv[]) {
    int ch;
    bool no_password = false;
    char* password = NULL;
    char bootstrap[PATH_MAX];
    while ((ch = getopt(argc, argv, "sS:")) != -1) {
        switch(ch) {
            case 's':
                no_password = true;
                break;
            case 'S':
                password = optarg;
                break;
            default:
                return -1;
        }
    }

    argc -= optind;
    argv += optind;

    if (no_password && password) {
        fprintf(stderr, "The -s and -S options are not allowed at the same time\n");
        return -1;
    }

    if (argc < 1) {
        fprintf(stderr, "No bootstrap file specified\n");
        return -1;
    }

    char* status = realpath(argv[0], bootstrap);
    if (!status) {
        fprintf(stderr, "Could not resolve path '%s', caused by: '%s'\n", argv[0], bootstrap);
        return -1;
    }

    printf("Will install bootstrap: %s\n", bootstrap);

    if (!no_password && !password) {
        printf("You need to set a new terminal password to use command line tools like \"sudo\".\n\n");
        char* confirm = NULL;
        do {
            if (confirm) {
                printf("Passwords does not match or is empty, try again\n");
            }
            password = getpass("New Password: ");
            confirm = getpass("Retype New Password: ");
        } while (strcmp(confirm, password) || confirm[0] == '\0');
    }

    xpc_object_t xdict = xpc_dictionary_create(NULL, NULL, 0);

    xpc_dictionary_set_string(xdict, "path", bootstrap);
    xpc_dictionary_set_uint64(xdict, "cmd", JBD_CMD_DEPLOY_BOOTSTRAP);
    xpc_dictionary_set_string(xdict, "bootstrapper-name", "p1ctl");
    xpc_dictionary_set_string(xdict, "bootstrapper-version", "3.0");
    xpc_dictionary_set_bool(xdict, "no-password", no_password);

    if (password) xpc_dictionary_set_string(xdict, "password", password);
    xpc_object_t xreply = jailbreak_send_jailbreakd_message_with_reply_sync(xdict);
    xpc_release(xdict);
    if (xpc_get_type(xreply) == XPC_TYPE_ERROR) {
        char* desc = xpc_copy_description(xreply);
        fprintf(stderr, "jailbreakd upcall failed: %s\n", desc);
        fprintf(stderr, "%s\n", desc);
        free(desc);
        xpc_release(xreply);
        return -1;
    }

    int retval = print_jailbreakd_reply(xreply);
    xpc_release(xreply);
    return retval;
}
