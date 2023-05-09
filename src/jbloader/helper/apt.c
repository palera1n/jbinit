// 
//  apt.c
//  src/jbloader/helper/apt.c
//  
//  Created 07/05/2023
//  jbloader (helper)
//

#include <jbloader.h>

#define APT_BIN_ROOTFUL "/usr/bin/apt-get"
#define APT_BIN_ROOTLESS "/var/jb/usr/bin/apt-get"
#define DPKG_BIN_ROOTFUL "/usr/bin/dpkg"
#define DPKG_BIN_ROOTLESS "/var/jb/usr/bin/dpkg"

#define SOURCES_PATH_ROOTFUL "/etc/apt/sources.list.d/palera1n.sources"
#define SOURCES_PATH_ROOTLESS "/var/jb/etc/apt/sources.list.d/palera1n.sources"
#define PROCURSUS_PATH "/etc/apt/sources.list.d/procursus.sources"
#define ZEBRA_PATH "/var/mobile/Library/Application Support/xyz.willy.Zebra/sources.list"

#define ELLEKIT_SILEO "Types: deb\nURIs: https://ellekit.space/\nSuites: ./\nComponents:\n\n"
#define ELLEKIT_ZEBRA "deb https://ellekit.space/ ./\n"

#define PALECURSUS_SILEO_1800 "Types: deb\nURIs: https://strap.palera.in/\nSuites: iphoneos-arm64/1800\nComponents: main\n\n"
#define PALECURSUS_SILEO_1900 "Types: deb\nURIs: https://strap.palera.in/\nSuites: iphoneos-arm64/1900\nComponents: main\n\n"
#define PALECURSUS_ZEBRA_1800 "deb https://strap.palera.in/ iphoneos-arm64/1800 main\n"
#define PALECURSUS_ZEBRA_1900 "deb https://strap.palera.in/ iphoneos-arm64/1900 main\n"

#define PALERA1N_ZEBRA "deb https://repo.palera.in/ ./\n"
#define PALERA1N_SILEO "Types: deb\nURIs: https://repo.palera.in/\nSuites: ./\nComponents:\n\n"

#define ZEBRA_ZEBRA "deb https://getzbra.com/repo ./\n"

#define PROCURSUS_ZEBRA_1800 "deb https://apt.procurs.us/ 1800 main\n"
#define PROCURSUS_ZEBRA_1900 "deb https://apt.procurs.us/ 1900 main\n"

int apt(char* args[]) {
    int ret, status;
    pid_t pid;

    const char *apt = check_rootful() ? APT_BIN_ROOTFUL : APT_BIN_ROOTLESS;
    if (access(apt, F_OK) != 0) {
        fprintf(stderr, "Unable to access apt: %d (%s)\n", errno, strerror(errno));
        return -1;
    }

    ret = posix_spawnp(&pid, apt, NULL, NULL, args, NULL);
    if (ret != 0) {
        fprintf(stderr, "%s %d \n", "apt failed with error:", ret);
        return ret;
    }

    waitpid(pid, &status, 0);
    return 0;
}

int dpkg_check_install(char* package) {
    int ret, status;
    pid_t pid;

    char* args[] = {"dpkg", "-s", package, NULL};
    const char *apt = check_rootful() ? DPKG_BIN_ROOTFUL : DPKG_BIN_ROOTLESS;
    if (access(apt, F_OK) != 0) {
        fprintf(stderr, "Unable to access dpkg: %d (%s)\n", errno, strerror(errno));
        return -1;
    }

    ret = posix_spawnp(&pid, apt, NULL, NULL, args, NULL);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "dpkg failed with error:", ret);
        return ret;
    }

    waitpid(pid, &status, 0);
    return 0;
}
            
int upgrade_packages() {
    int sileo = dpkg_check_install("org.coolstar.sileo");
    int zebra = dpkg_check_install("xyz.willy.Zebra");
    int nightly = dpkg_check_install("org.coolstar.sileonightly");

    if (sileo && zebra && nightly) {
        fprintf(stderr, "%s\n", "No package manager found, unable to continue");
        return -1;
    }

    apt((char*[]){"apt-get", "update", "--allow-insecure-repositories", NULL});
    apt((char*[]){"apt-get", "--fix-broken",  "install", "-y", "--allow-unauthenticated", NULL});

    if (check_rootful()) {
        fprintf(stderr, "%s\n", "Installing keyring for strap.palera.in");
        apt((char*[]){"apt-get", "install", "nebula-keyring", "-y", "--allow-unauthenticated", NULL});
    }

    apt((char*[]){"apt-get", "upgrade", "-y", "--allow-unauthenticated", NULL});
    return 0;
}

int rootful_cleanup() {
    if (!check_rootful()) {
        fprintf(stderr, "%s\n", "Used for rootful only.");
        return -1;
    }

    FILE *procursus = fopen(PROCURSUS_PATH, "rb");
    if (procursus != NULL) {
        fprintf(stdout, "%s\n", "Removing procursus.sources.");
        fclose(procursus);
        remove(PROCURSUS_PATH);
    } else {
        fclose(procursus);
    }
    
    return 0;
}

int add_sources_sileo() {
    FILE *sources;
    const char *sources_file = check_rootful() ? SOURCES_PATH_ROOTFUL : SOURCES_PATH_ROOTLESS;
    int CF = (int)((floor)(kCFCoreFoundationVersionNumber / 100) * 100);

    sources = fopen(sources_file, "w+");
    fputs(PALERA1N_SILEO, sources);

    if (CF == 1900) {
        if (check_rootful()) fputs(PALECURSUS_SILEO_1900, sources);
        else fputs(ELLEKIT_SILEO, sources);
    } else if (CF == 1800) {
        if (check_rootful()) fputs(PALECURSUS_SILEO_1800, sources);
        else fputs(ELLEKIT_SILEO, sources);
    } else {
        fprintf(stderr, "%s %d\n", "Unknown CoreFoundation Version:", CF);
        return -1;
    }

    int ret = fclose(sources);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to close sources file:", ret);
        return ret;
    }
    
    return 0;
}

int add_sources_zebra() {
    FILE *sources;
    const char *sources_file = ZEBRA_PATH;
    int CF = (int)((floor)(kCFCoreFoundationVersionNumber / 100) * 100);

    sources = fopen(sources_file, "rb");
    if (sources != NULL) {
        fprintf(stdout, "%s\n", "Removing zebra.list.");
        fclose(sources);
        remove(sources_file);
    } else {
        fclose(sources);
    }

    sources = fopen(sources_file, "w+");
    fputs(ZEBRA_ZEBRA, sources);
    fputs(PALERA1N_ZEBRA, sources);

    if (CF == 1900) {
        if (check_rootful()) fputs(PALECURSUS_ZEBRA_1900, sources);
        else {fputs(ELLEKIT_ZEBRA, sources); fputs(PROCURSUS_ZEBRA_1900, sources);}
    } else if (CF == 1800) {
        if (check_rootful()) fputs(PALECURSUS_ZEBRA_1800, sources);
        else {fputs(ELLEKIT_ZEBRA, sources); fputs(PROCURSUS_ZEBRA_1800, sources);}
    } else {
        fprintf(stderr, "%s %d\n", "Unknown CoreFoundation Version:", CF);
        return -1;
    }

    int ret = fclose(sources);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to close sources file:", ret);
        return ret;
    }

    return 0;
}

int add_sources() {
    int sileo = dpkg_check_install("org.coolstar.sileo");
    int zebra = dpkg_check_install("xyz.willy.Zebra");
    int nightly = dpkg_check_install("org.coolstar.sileonightly");

    int ret;
    if (check_rootful()) rootful_cleanup();

    if (!sileo || !nightly) {
        ret = add_sources_sileo();
        if (ret != 0) {
            fprintf(stderr, "%s %d\n", "Failed to add default sources to Sileo:", ret);
            return ret;
        }
    }

    if (!zebra) {
        ret = add_sources_zebra();
        if (ret != 0) {
            fprintf(stderr, "%s %d\n", "Failed to add default sources to Zebra:", ret);
            return ret;
        }
    }

    ret = upgrade_packages();
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to update packages via apt:", ret);
        return ret;
    }

    return 0;
}