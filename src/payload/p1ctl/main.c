#include <paleinfo.h>
#include <payload/payload.h>
#include <libjailbreak/libjailbreak.h>
#include <stdio.h>
#include <xpc/private.h>

static int help_cmd(int argc, char* argv[]);
static int kbase_cmd(int argc, char* argv[]);
int bootstrap_main(int argc, char* argv[]);
int reboot_userspace_main(int argc, char* argv[]);
int reload_main(int argc, char* argv[]);
int tweakloader_main(int argc, char* argv[]);
static int usage();

struct subcommand {
    const char* name;
    const char* help;
    const char* usage;
    const char* long_help;
    int (*cmd_main)(int argc, char* argv[]);
};

static struct subcommand commands[] = {
    {"help", "\t\tPrints usage information", "[subcommand]", NULL, help_cmd},
    {"palera1n_flags", "\tPrint palera1n flags", "[-s]", "With no options specified, this command prints a hexadecimal representation of the flags.\nOptions:\n\n\t-s\tPrint a string description of all the flags instead", palera1n_flags_main},
    {"kbase", "\t\tPrint kernel base", NULL, "Get a hexadecimal representation of the kernel base", kbase_cmd},
    {"kslide", "\t\tPrint kernel slide", NULL, "Get a hexadecimal representation of the kernel slide", kbase_cmd},
    {"bootstrap", "\tDeploy bootstrap", "[-s|-S <password>] <bootstrap path>", "The <bootstrap path> argument should be a path to a zstd-compressed tar archive bootstrap matching the current jailbreak type\nOptions:\n\n\t-s\t\tWhen this option is specified, the terminal password will not be set\n\t-S <password>\tThis option allows supplying the terminal password without responding to prompts", bootstrap_main},
    {"revert-install", "\tRemove bootstrap (Rootless)", NULL, "Remove the installed bootstrap. This operation is only supported on rootless.", obliterate_main},
    {"reboot-userspace", "Reboot userspace", NULL, "Unmount /Developer and reboot userspace", reboot_userspace_main},
    {"reload", "\t\tReload launchd jailbreak state", NULL, "Reload launchd's jailbreak state, such as the JB_ROOT_PATH variable", reload_main},
    {"tweakloader", "\tSet TweakLoader path", "<tweakloader path>", "Sets the tweak injection library that will be loaded by systemhook.dylib", tweakloader_main},
    {NULL, NULL, NULL}
};

int tweakloader_main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "No new tweakloader path specified.\n");
        return -1;
    }

    xpc_object_t xdict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_object_t xreply;
    xpc_dictionary_set_uint64(xdict, "cmd", LAUNCHD_CMD_SET_TWEAKLOADER_PATH);
    xpc_dictionary_set_string(xdict, "path", argv[1]);
    int ret = jailbreak_send_launchd_message(xdict, &xreply);
    if (xreply) {
        print_jailbreakd_reply(xreply);
    } else {
        fprintf(stderr, "failed to send launchd message: %d (%s)\n", ret, strerror(ret));
    }
    xpc_release(xdict);
    return ret;
}

int reload_main(int argc, char* argv[]) {
    xpc_object_t xdict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_object_t xreply;
    xpc_dictionary_set_uint64(xdict, "cmd", LAUNCHD_CMD_RELOAD_JB_ENV);
    int ret = jailbreak_send_launchd_message(xdict, &xreply);
    if (xreply) {
        print_jailbreakd_reply(xreply);
    } else {
        fprintf(stderr, "failed to send launchd message: %d (%s)\n", ret, strerror(ret));
    }
    xpc_release(xdict);
    return ret;
}

#define RB2_USERREBOOT (0x2000000000000000llu)

int reboot_userspace_main(int argc, char* argv[]) {
    xpc_object_t xdict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(xdict, "cmd", JBD_CMD_PERFORM_REBOOT3);
    xpc_dictionary_set_uint64(xdict, "howto", RB2_USERREBOOT);
    xpc_object_t xreply = jailbreak_send_jailbreakd_message_with_reply_sync(xdict);
    if (xpc_get_type(xreply) != XPC_TYPE_ERROR) {
        print_jailbreakd_reply(xreply);
    } else {
        char* desc = xpc_copy_description(xreply);
        fprintf(stderr, "failed to send jailbreakd message: %s\n", desc);
        free(desc);
    }
    xpc_release(xreply);
    xpc_release(xdict);
    return 0;
}


static int help_cmd(int argc, char* argv[]) {
    if (argc < 2) {
        usage();
        return 0;
    }
    for (uint8_t i = 0; commands[i].name != NULL; i++) {
        if (!strcmp(argv[1], commands[i].name)) {
            fprintf(stderr, "Usage: %s %s %s\n", getprogname(), commands[i].name, commands[i].usage ? commands[i].usage : "");
            if (commands[i].long_help != NULL) {
                fprintf(stderr, "\n%s\n", commands[i].long_help);
            }
            return 0;
        }
    }
    return 0;
}

static int kbase_cmd(int argc, char* argv[]) {
    P1CTL_UPCALL_JBD_WITH_ERR_CHECK(xreply, JBD_CMD_GET_PINFO_KERNEL_INFO);
    int retval = 0;
    uint64_t kbase;
    if ((kbase = xpc_dictionary_get_uint64(xreply, argv[0]))) {
        printf("0x%llx\n", kbase);
        retval = 0;
    } else {
        uint64_t error = xpc_dictionary_get_int64(xreply, "error");
        fprintf(stderr, "get %s failed: %d (%s)\n", argv[0], (int)error, xpc_strerror((int)error));
        retval = -1;
    }
    xpc_release(xreply);
    return retval;
}

static int usage() {
    fprintf(stderr,
        "Usage: p1ctl <subcommand> [subcommand options] [subcommand argument]\n"
        "p1ctl is a tool to interact with palera1n internal interfaces\n\nSubcommands:\n"
    );
    for (uint8_t i = 0; commands[i].name != NULL; i++) {
        fprintf(stderr, "\t%s\t%s\n", commands[i].name, commands[i].help);
    }
    fprintf(stderr, "\nUse `p1ctl help <subcommand>` to get help on a subcommand.\n");
    return 0;
}


int p1ctl_main(int argc, char* argv[]) {
    if (argc < 2) {
        usage();
        return 1;
    }
    for (uint8_t i = 0; commands[i].name != NULL; i++) {
        if (!strcmp(argv[1], commands[i].name)) {
            return commands[i].cmd_main(argc - 1, &argv[1]);
            break;
        }
    }
    usage();
    return 1;
}
