#include <jbloader.h>
#include <getopt.h>
#include <mach-o/arch.h>

static int helper_usage() {
    fprintf(stderr,
        "Usage: helper [-pkbPrRi] <optional-arguments>\n"
        "helper is for use with the palera1n loader only.\n"
        "\t-p, --print-pflags\t\tprints paleinfo flags\n"
        "\t-k, --print-kflags\t\tprints kerninfo flags\n"
        "\t-b, --print-bmhash\t\tprints boot manifest hash\n"
        "\t-P, --set-password\t\tset 501 (uid) password\n"
        "\t-r, --reboot\t\t\treboot device\n"
        "\t-R, --revert-install\t\trevert palera1n install\n"
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
    {"revert", required_argument, 0, 'R'},
    {"install", required_argument, 0, 'i'},
    {NULL, 0, NULL, 0}
};

int helper_main(int argc, char *argv[]) {
    const NXArchInfo *arch = NXGetLocalArchInfo();
    int option_index = 0;
    int opt;

    if (strcmp("arm64", arch->name)) {
        fprintf(stderr, "Architecture type '%s' is not supported.\n", arch->name);
        return EPERM;
    }

    if (argc < 2) {
        helper_usage();
        return EINVAL;
    }
    
    if (getuid() != 0 && getgid() != 0) {
        fprintf(stderr, "helper must be ran as 'root'.\n");
        return EACCES;
    }

    while((opt = getopt_long(argc, argv, "pkbi:P:rR", long_opt, NULL)) != -1) {
        switch (opt) {
            case 0: if (long_opt[option_index].flag != 0) break; if (optarg) break;
            case 'p': get_pflags(); break;
            case 'k': get_kflags(); break;
            case 'b': get_bmhash(); break;
            case 'P': setpw(optarg); break;
            case 'r': reboot(0); break;
            case 'R': helper_usage(); break;
            case 'i': install_bootstrap(optarg, "test_dir"); break;
            default: helper_usage(); break;
        }
    }

    return 0;
}