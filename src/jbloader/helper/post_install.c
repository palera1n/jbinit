// 
//  post_install.c
//  src/jbloader/helper/post_install.c
//  
//  Created 02/05/2023
//  jbloader (helper)
//

#include <jbloader.h>


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

int post_install(char *pm) {
    if (check_rootful() == 1) {
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

        char *path_link = malloc(sizeof(char) * (256)); // does not need to be 256, maybe 136?
        sprintf(path_link, "/private/preboot/%s/%s/procursus", hash, jb_path);
        printf("full: %s\n", path_link);

        ret = symlink(path_link, "/var/jb");
        if (ret != 0) {
            fprintf(stderr, "%s %s (%d)\n", "Failed to create link:", "/var/jb", ret);
            return ret;
        }

        ret = chdir("/var/jb");
        if (ret != 0) {
            fprintf(stderr, "%s (%d)\n", "Failed to chdir into /var/jb:", ret);
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
        fprintf(stderr, "%s %d", "prep_bootstrap.sh failed with:", ret);
        return ret;
    }

    waitpid(pid, &status, 0);

    ret = install_deb(pm);
    if (ret != 0) {
        fprintf(stderr, "%s %s %s%d%s", "Failed to install:", pm, "(", ret, ")");
        return ret;
    }
    
    add_sources();
    return 0;
}
