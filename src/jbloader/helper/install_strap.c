// 
//  install_strap.c
//  src/jbloader/helper/install_strap.c
//  
//  Created 30/04/2023
//  jbloader (helper)
//
//  Parts of this source are from untar.c by Tim Kientzle, March 2009 (Public Domain)
//  https://opensource.apple.com/source/libarchive/libarchive-32/libarchive/contrib/untar.c.auto.html
//

#include <jbloader.h>

#define ZST 1
#define TAR 2
#define UNKNOWN 3

static int archive_end(const char *p) {
    for (int n = 511; n >= 0; --n)
        if (p[n] != '\0') return (0);
    return (1);
}

static int verify_checksum(const char *p) {
    int n, u = 0;
    for (n = 0; n < 512; ++n) {
        if (n < 148 || n > 155)
            u += ((unsigned char *)p)[n];
        else
            u += 0x20;
    }
    return (u == parseoct(p + 148, 8));
}

static int filter(char *path) {
    const char *filename = basename(path);
    fprintf(stdout, "%s %s\n", "basepath:", filename);

    if (!strcmp(".DS_Store", filename) || !strcmp("__MACOSX", filename)) {
        fprintf(stdout, "%s %s\n", "Skipping file:", path);
        return -1;
    }
    
    return 0;
}

static void untar(FILE *a, const char *path) {
    char pathName[100]; 
    char buff[512];
    FILE *f = NULL;
    size_t bytes_read;
    int filesize;
    time_t mtime;
    mode_t mode;

    fprintf(stdout, "%s %s\n", "Extracting bootstrap from:", path);
    for (;;) {
        bytes_read = fread(buff, 1, 512, a);
        
        if (bytes_read < 512) {
            fprintf(stderr, "%s %s %s %d\n", "Short read on", path, ": expected 512, got", (int)bytes_read);
            return;
        }
        if (archive_end(buff)) {
            fprintf(stdout, "%s %s\n", "End of", path);
            return;
        }
        if (!verify_checksum(buff)) {
            fprintf(stderr, "%s\n", "Checksum failure");
            return;
        }

        if (filter(buff) != 0) break;
        filesize = parseoct(buff + 124, 12);
        switch (buff[TYPEFLAG_OFF]) {
            case LNKTYPE: create_link(buff, HARD); break;
            case SYMTYPE: create_link(buff, SYM); break;
            case CHRTYPE: break;
            case BLKTYPE: break;
            case FIFOTYPE: break;
            case DIRTYPE: MODE; create_dir(buff, mode, UID, GID); filesize = 0; break;
            default:
                printf("%s\n", buff);
                mtime = parsetime(buff + MTIME_OFF);
                parsemode(buff + OCTAL_OFF, &mode);
                for (int n = 0; n < 100; ++n) pathName[n] = buff[0 + n];
                f = create_file(buff, mode, UID, GID);
                chown(pathName, UID, GID);
                break;
        }

        while (filesize > 0) {
            bytes_read = fread(buff, 1, 512, a);
            if (bytes_read < 512) {
                fprintf(stderr, "%s %s %s %d\n", "Short read on", path, ": Expected 512, got", (int)bytes_read);
                return;
            }
            if (filesize < 512)
                bytes_read = filesize;
            if (f != NULL) {
                if (fwrite(buff, 1, bytes_read, f) != bytes_read) {
                    fprintf(stderr, "%s %s\n", "Failed write to:", buff);
                    fclose(f);
                    f = NULL;
                }
            }
            filesize -= bytes_read;
        }       

        if (f != NULL) {
            fclose(f);
            chmod(pathName, mode);
            set_time(pathName, mtime, 0);
            f = NULL;
        }
    }
}

int install_bootstrap(const char *tar, char *pm) {
    int type;
    int ret = mount_check("/private/preboot");
    if (ret != 0) return -1;
    
    char *tar_path = realpath(tar, NULL);
    char *pm_path = realpath(pm, NULL);
    FILE *t;
    
    if (tar_path == NULL) {
        fprintf(stderr, "%s %s\n", "Unable to find tar real path:", tar);
        return -1;
    }

    if (pm_path == NULL) {
        fprintf(stderr, "%s %s\n", "Unable to find package manager real path:", pm);
        return -1;
    }

    int tar_len = strlen(tar);
    const char *ext = &tar[tar_len-3];
    if (!strcmp(ext, "zst")) type = ZST;
    else if (!strcmp(ext, "tar")) type = TAR;
    else {
        fprintf(stderr, "%s %s\n", "Unknown tar file supplied.", tar);
        return -1;
    }
    
    if (check_rootful() == 1) {
        if (check_forcerevert()) {
            fprintf(stderr, "%s\n", "Please re-jailbreak after a force-revert.");
            return -1;
        }
        
        chdir("/");
    } else {
        FILE *varjb = fopen("/var/jb", "rb");
        if (varjb != NULL) {
            fprintf(stdout, "%s\n", "Found /var/jb, removing before install.");
            fclose(varjb);
            if (rmdir("/var/jb") != 0) {
                fprintf(stderr, "%s\n", "Failed to remove old /var/jb.");
                return -1;
            }
        } else {
            fclose(varjb);
        }

        char dest[116] = "/private/preboot/";
        char hash[97];
        ret = get_boot_manifest_hash(hash);

        if (ret != 0) {
            fprintf(stderr, "%s %d\n", "Failed to get boot manifest hash:", ret);
            return ret;
        }

        strncat(dest, hash, 97);
        chdir(dest);
    }

    t = fopen(tar_path, "rb");
    if (t == NULL) {
        fprintf(stderr, "%s %s\n", "Unable to open:", tar_path);
        return -1;
    } else {       
        if (type == ZST) {
            fclose(t);

            ret = decompress(tar_path);
            if (ret != 0) {
                fprintf(stderr, "%s %d\n", "Failed to decompress zst:", ret);
                return ret;
            }

            t = fopen("/var/mobile/Library/palera1n/temp/bootstrap.tar", "rb");
            if (t == NULL) {
                fprintf(stderr, "%s %s\n", "Unable to open:", tar_path);
                return -1;
            }

            untar(t, "/var/mobile/Library/palera1n/temp/bootstrap.tar");
        } else {
            untar(t, tar_path);
        }
        fclose(t);
    }

    ret = post_install(pm_path);
    if (ret != 0) {
        fprintf(stderr, "%s %d\n", "Post install failed:", ret);
        return ret;
    }
    
    return 0;
}
