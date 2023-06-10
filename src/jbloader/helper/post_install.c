// 
//  post_install.c
//  src/jbloader/helper/post_install.c
//  
//  Created 02/05/2023
//  jbloader (helper)
//

#include <jbloader.h>

#define DOTFILE_ROOTFUL "/.palecursus_strapped"
#define DOTFILE_ROOTLESS "/var/jb/.palecursus_strapped"

#define LIBRARY "/var/jb/var/mobile/Library"
#define PREFERENCES "/var/jb/var/mobile/Library/Preferences"
#define CACHES "/var/jb/var/mobile/Library/Caches"

char *create_jb_path() { 
    srand(time(0));
    char *valid_set = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char random_str[11] = "jb-XXXXXXXX";

    unsigned int index = 0;
    for (int n = 3;n < 11;n++) {
        index = rand() % 62;          
        random_str[n] = valid_set[index];
    }

    char *ret = malloc(sizeof(char) * (11));
    memcpy(ret, random_str, 11);
    return ret;
}

int create_directories() {
    int ret;
    
    ret = mkpath_np(LIBRARY, 0711);
    if (ret == -1) {
        fprintf(stderr, "%s %s\n", "Failed to create library folder:", strerror(errno));
        return -1;
    }
    chmod(LIBRARY, 0711);
    chown(LIBRARY, 501, 501);

    ret = mkpath_np(PREFERENCES, 0711);
    if (ret == -1) {
        fprintf(stderr, "%s %s\n", "Failed to create preferences folder:", strerror(errno));
        return -1;
    }
    chmod(PREFERENCES, 0755);
    chown(PREFERENCES, 501, 501);

    ret = mkpath_np(CACHES, 0711);
    if (ret == -1) {
        fprintf(stderr, "%s %s\n", "Failed to create caches folder:", strerror(errno));
        return -1;
    }
    chmod(CACHES, 0700);
    chown(CACHES, 501, 501);

    return 0;
}

int post_install(char *pm) {
    if (check_rootful()) {
        int ret = chdir("/");
        if (ret != 0) {
            fprintf(stderr, "%s\n", "Failed to chdir into /");
            return ret;
        }
    } else {
        char *jb_path = create_jb_path();
        char dest[116] = "/private/preboot/";
        char hash[97];

        int ret = get_boot_manifest_hash(hash);
        if (ret != 0) {
            fprintf(stderr, "%s %d\n", "Failed to get boot manifest hash:", ret);
            return ret;
        }

        strncat(dest, hash, 97);
        ret = chdir(dest);
        if (ret != 0) {
            fprintf(stderr, "%s %s\n", "Failed to chdir into:", dest);
            return ret;
        }

        DIR* temp_dir = opendir("var/jb");
        if (temp_dir) closedir(temp_dir);
        else return -1;
        if(rename("var/jb", "var/procursus") != 0) return -1;

        temp_dir = opendir("var");
        if (temp_dir) closedir(temp_dir);
        else return -1;
        if(rename("var", jb_path) != 0) return -1;

        char *path_link = malloc(sizeof(char) * (256));
        sprintf(path_link, "/private/preboot/%s/%s/procursus", hash, jb_path);

        ret = symlink(path_link, "/var/jb");
        if (ret != 0) {
            fprintf(stderr, "%s %s %s%d%s\n", "Failed to create link:", "/var/jb", "(", ret, ")");
            return ret;
        }

        ret = chdir("/var/jb");
        if (ret != 0) {
            fprintf(stderr, "%s %d\n", "Failed to chdir into /var/jb:", ret);
            return ret;
        }
    }

    const char *bin = check_rootful() ? "/usr/bin/sh" : "/var/jb/usr/bin/sh";
    char* args[] = {"sh", "prep_bootstrap.sh", NULL};

    int ret;
    pid_t pid;
    int status;
    
    char* env_rootful[] = {"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:", "NO_PASSWORD_PROMPT=1", NULL};
    char* env_rootless[] ={"PATH=/var/jb/usr/local/sbin:/var/jb/usr/local/bin:/var/jb/usr/sbin:/var/jb/usr/bin:/var/jb/sbin:/var/jb/bin:", "NO_PASSWORD_PROMPT=1", NULL};

    ret = posix_spawnp(&pid, bin, NULL, NULL, args, check_rootful() ? env_rootful : env_rootless);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "prep_bootstrap.sh failed with:", ret);
        return ret;
    }

    waitpid(pid, &status, 0);

    ret = install_deb(pm);
    if (ret != 0) {
        fprintf(stderr, "%s %s %s%d%s\n", "Failed to install:", pm, "(", ret, ")");
        return ret;
    }
    
    const char *dotfile_path = check_rootful() ? DOTFILE_ROOTFUL : DOTFILE_ROOTLESS;
    FILE *dotfile = fopen(dotfile_path, "w+");
    if (dotfile == NULL) {
        fprintf(stderr, "%s\n", "Failed to create dotfiles");
        return -1;
    }
    fclose(dotfile);

    ret = add_sources();
    if (ret != 0) { 
        fprintf(stderr, "%s %d\n", "Failed to add default sources:", ret);
        return ret;
    }

    if (!check_rootful()) {
        ret = create_directories();
        if (ret != 0) {
            fprintf(stderr, "%s %d\n", "Failed to create directories:", ret);
            return ret;
        }
    }


    return 0;
}
