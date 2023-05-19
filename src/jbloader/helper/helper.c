// 
//  helper.c
//  src/jbloader/helper/helper.c
//  
//  Created 30/04/2023
//  jbloader (helper)
//

#include <jbloader.h>
#include <getopt.h>
#include <mach-o/arch.h>

static int helper_usage() {
    fprintf(stderr,
        "Usage: helper [-pkbi:P:rRd:tfsS] <optional-arguments>\n"
        "helper is for use with the palera1n loader only.\n"
        "\t-p, --print-pflags\t\tprints paleinfo flags\n"
        "\t-k, --print-kflags\t\tprints kerninfo flags\n"
        "\t-b, --print-bmhash\t\tprints boot manifest hash\n"
        "\t-P, --set-password\t\tset mobile password\n"
        "\t-r, --reboot\t\t\treboot device\n"
        "\t-m, --safemode\t\t\tenter/exit safemode\n"
        "\t-R, --revert-install\t\trevert palera1n install\n"
        "\t-s, --string-pflags\t\tprints strings of pflags\n"
        "\t-S, --string-kflags\t\tprints strings of kflags\n"
        "\t-d, --install-deb\t\tinstall a deb file\n"
        "\t-i, --install\t\t\tcompletes palera1n install\n"
    );
    return 0;
}

static struct option long_opt[] = {
    {"print-pflags", no_argument, 0, 'p'},
    {"print-kflags", no_argument, 0, 'k'},
    {"print-bmhash", no_argument, 0, 'b'},
    {"set-password", required_argument, 0, 'P'},
    {"reboot", no_argument, 0, 'r'},
    {"revert-install", no_argument, 0, 'R'},
    {"install", required_argument, 0, 'i'},
    {"jailbreak-type", no_argument, 0, 't'},
    {"force-revert-check", no_argument, 0, 'f'},
    {"string-pflags", no_argument, 0, 's'},
    {"string-kflags", no_argument, 0, 'S'},
    {"install-deb", required_argument, 0, 'd'},
    {"safemode", required_argument, 0, 'm'},
    {NULL, 0, NULL, 0}
};

int helper_main(int argc, char *argv[]) {
    const NXArchInfo *arch = NXGetLocalArchInfo();
    char *package_manager;
    int option_index = 0;
    int opt;

    if (strcmp("arm64", arch->name)) {
        fprintf(stderr, "%s %s %s\n", "Architecture type", arch->name, "is not supported.");
        return EPERM;
    }

    if (argc < 2) {
        helper_usage();
        return EINVAL;
    }
    
    if (getuid() != 0 && getgid() != 0) {
        fprintf(stderr, "%s\n", "Insufficient permissions.");
        return EACCES;
    }

    while((opt = getopt_long(argc, argv, "pkbi:P:rRd:tfsS", long_opt, NULL)) != -1) {
        switch (opt) {
            case 0: if (long_opt[option_index].flag != 0) break; if (optarg) break;
            case 'p': get_pflags(); break;
            case 'k': get_kflags(); break;
            case 'b': get_bmhash(); break;
            case 'P': setpw(optarg); break;
            case 'r': reboot(0); break;
            case 'R': revert_install(); break;
            case 't': fprintf(stdout, "%d\n",check_rootful()); break;
            case 'f': fprintf(stdout, "%d\n",check_forcerevert()); break;
            case 's': print_pflags_str(); break;
            case 'S': print_kflags_str(); break;
            case 'd': install_deb(realpath(optarg, NULL)); break;
            case 'i': install_bootstrap(optarg, argv[3]); break;
            case 'm': safemode(atoi(optarg)); break;
            default: helper_usage(); break;
        }
    }

    return 0;
}