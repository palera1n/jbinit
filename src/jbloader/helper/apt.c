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
#define PROCURSUS_ZEBRA_2000 "deb https://apt.procurs.us/ 2000 main\n" // future proofing

#define PROCURSUS_PATH "/etc/apt/sources.list.d/procursus.sources"
#define PROCURSUS_PREFS_PATH "/etc/apt/preferences.d/procursus"

int apt(char* args[]) {
    int ret, status;
    pid_t pid;

    const char *apt = check_rootful() ? APT_BIN_ROOTFUL : APT_BIN_ROOTLESS;
    if (access(apt, F_OK) != 0) {
        fprintf(stderr, "%s %d %s%s%s\n", "Unable to access apt:", errno, "(", strerror(errno), ")");
        return EACCES;
    }

    ret = posix_spawnp(&pid, apt, NULL, NULL, args, NULL);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "apt failed with error:", ret);
        return ret;
    }

    waitpid(pid, &status, 0);
    return 0;
}
            
int upgrade_packages() {
    int installed = pm_installed();
    if (installed == 0) {
        fprintf(stderr, "%s\n", "No package manager found, unable to continue.");
        return -1;
    }

    apt((char*[]){"apt-get", "update", "--allow-insecure-repositories", NULL});
    apt((char*[]){"apt-get", "--fix-broken",  "install", "-y", "--allow-unauthenticated", NULL});
    apt((char*[]){"apt-get", "upgrade", "-y", "--allow-unauthenticated", NULL});
    apt((char*[]){"apt-get", "install", "nebula-keyring", "-y", "--allow-unauthenticated", NULL});

    return 0;
}

int rootful_cleanup() {
    if (!check_rootful()) {
        fprintf(stderr, "%s\n", "Used for rootful only.");
        return -1;
    }

    FILE *source = fopen(PROCURSUS_PATH, "rb");
    if (source != NULL) {
        fprintf(stdout, "%s\n", "Removing procursus.sources.");
        fclose(source);
        remove(PROCURSUS_PATH);
    } else {
        fclose(source);
    }

    FILE *prefs = fopen(PROCURSUS_PATH, "rb");
    if (prefs != NULL) {
        fprintf(stdout, "%s\n", "Removing procursus preferences.");
        fclose(prefs);
        remove(PROCURSUS_PATH);
    } else {
        fclose(prefs);
    }
    
    return 0;
}

int add_sources_apt() {
    const char *sources_file = check_rootful() ? SOURCES_PATH_ROOTFUL : SOURCES_PATH_ROOTLESS;
    int CF = (int)((floor)(kCFCoreFoundationVersionNumber / 100) * 100);

    FILE *apt_sources = fopen(sources_file, "rb");
    if (apt_sources != NULL) {
        fprintf(stdout, "%s\n", "Removing zebra.list.");
        fclose(apt_sources);
        remove(sources_file);
    } else {
        fclose(apt_sources);
    }

    apt_sources = fopen(sources_file, "w+");
    fputs(PALERA1N_SILEO, apt_sources);

    if (CF == 1800) {
        if (check_rootful()) fputs(PALECURSUS_SILEO_1800, apt_sources);
        else fputs(ELLEKIT_SILEO, apt_sources);
    } else if (CF == 1900) {
        if (check_rootful()) fputs(PALECURSUS_SILEO_1900, apt_sources);
        else fputs(ELLEKIT_SILEO, apt_sources);
    } else if (CF == 2000) {
        if (!check_rootful()) fputs(ELLEKIT_SILEO, apt_sources);
    } else {
        fprintf(stderr, "%s %d\n", "Unknown CoreFoundation Version:", CF);
        return -1;
    }

    int ret = fclose(apt_sources);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to close apt sources file:", ret);
        return ret;
    }

    return 0;
}

int add_sources_zebra() {
    FILE *zebra_sources = fopen(ZEBRA_PATH, "rb");
    if (zebra_sources != NULL) {
        fprintf(stdout, "%s\n", "Removing zebra.list.");
        fclose(zebra_sources);
        remove(ZEBRA_PATH);
    } else {
        fclose(zebra_sources);
    }

    int CF = (int)((floor)(kCFCoreFoundationVersionNumber / 100) * 100);
    zebra_sources = fopen(ZEBRA_PATH, "w+");
    fputs(ZEBRA_ZEBRA, zebra_sources);
    fputs(PALERA1N_ZEBRA, zebra_sources);

    if (CF == 1800) {
        if (check_rootful()) fputs(PALECURSUS_ZEBRA_1800, zebra_sources);
        else {fputs(ELLEKIT_ZEBRA, zebra_sources); fputs(PROCURSUS_ZEBRA_1800, zebra_sources);}
    } else if (CF == 1900) {
        if (check_rootful()) fputs(PALECURSUS_ZEBRA_1900, zebra_sources);
        else {fputs(ELLEKIT_ZEBRA, zebra_sources); fputs(PROCURSUS_ZEBRA_1900, zebra_sources);}
    } else if (CF == 2000) {
        if (!check_rootful()) {fputs(ELLEKIT_ZEBRA, zebra_sources); fputs(PROCURSUS_ZEBRA_1900, zebra_sources);}
    } else {
        fprintf(stderr, "%s %d\n", "Unknown CoreFoundation Version:", CF);
        return -1;
    }

    int ret = fclose(zebra_sources);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to close Zebra sources file:", ret);
        return ret;
    }

    return 0;
}

int add_sources() {
    int installed = pm_installed();
    int ret;
    if (check_rootful()) rootful_cleanup();

    ret = add_sources_apt();
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Failed to add default sources to apt:", ret);
        return ret;
    }
    
    if (installed == 1 || installed == 3) {
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
