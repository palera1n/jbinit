#include <jbloader.h>
#include <getopt.h>

static int help_cmd(int argc, char* argv[]);

struct subcommand {
    const char* name;
    const char* help;
    const char* usage;
    const char* long_help;
    int (*cmd_main)(int argc, char* argv[]);
};

static struct subcommand commands[] = {
    {"print", "\tprints kerninfo and paleinfo", "", "The print command prints a textual representation of kerninfo and paleinfo, including the flags", print_info},
    {"help", "\tprints usage information", "[subcommand]", NULL, help_cmd},
    {"palera1n_flags", "print or set palera1n flags", "[new flags]", "The palera1n_flags subcommand accept numerical input in [new flags]\nWith no arguments, it prints a hexadecimal representation of the flags.", palera1n_flags_main},
    {"checkra1n_flags", "print or set checkra1n flags", "[new_flags]", "The checkra1n_flags subcommand accept numerical input in [new flags]\nWith no arguments, it prints a hexadecimal representation of the flags.", checkra1n_flags_main },
    {"print_bmhash", "print the boot manifest hash", "", "The print_bmhash command prints the boot manifest hash in uppercase.", print_boot_manifest_hash_main },
    {NULL, NULL, NULL}
};

static int usage() {
    fprintf(stderr,
        "Usage: p1ctl <subcommand> [subcommand options] [subcommand argument]\n"
        "p1ctl prints or edits palera1n data\n\nSubcommands:\n"
    );
    for (uint8_t i = 0; commands[i].name != NULL; i++) {
        fprintf(stderr, "\t%s\t%s\n", commands[i].name, commands[i].help);
    }
    return 0;
}

static int help_cmd(int argc, char* argv[]) {
    if (argc < 2) {
        usage();
        return 0;
    }
    for (uint8_t i = 0; commands[i].name != NULL; i++) {
        if (!strcmp(argv[1], commands[i].name)) {
            fprintf(stderr, "Usage: %s %s %s\n", getprogname(), commands[i].name, commands[i].usage);
            if (commands[i].long_help != NULL) {
                fprintf(stderr, "\n%s\n", commands[i].long_help);
            }
            return 0;
        }
    }
    return 0;
}


int p1ctl_main(int argc, char *argv[]) {
    if (argc < 2) {
        usage();
        return 1;
    }
    if (init_info()) return -1;
    for (uint8_t i = 0; commands[i].name != NULL; i++) {
        if (!strcmp(argv[1], commands[i].name)) {
            return commands[i].cmd_main(argc - 1, &argv[1]);
            break;
        }
    }
    usage();
    return 1;
}
