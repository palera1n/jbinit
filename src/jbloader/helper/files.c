// 
//  files.c
//  src/jbloader/helper/files.c
//  
//  Created 03/05/2023
//  jbloader (helper)
//
//  Parts of this source are from untar.c by Tim Kientzle, March 2009 (Public Domain)
//  https://opensource.apple.com/source/libarchive/libarchive-32/libarchive/contrib/untar.c.auto.html
//

#include <jbloader.h>

/*!
 * Fully supports POSIX Tar Headers of 1003.1-1990
 * 
 * @warning Jörg Schilling star header is NOT fully supported
 * char atime[12]; is NOT supported
 * char ctime[12]; is NOT supported
 */

int parsemode(const char* str, mode_t* mode) {
    char* end = NULL;
    *mode = (mode_t)strtol(str, &end, 8);
    if (!end) return 0;
    while(isspace(*end)) end++;
    return *end == '\0' && (unsigned)*mode < 010000;
}

int parseoct(const char *p, size_t n) {
    int i = 0;
    while ((*p <'0' || *p > '7') && n > 0) {++p;--n;}
    while (*p >= '0' && *p <= '7' && n > 0) {i*=8;i+=*p-'0';++p;--n;}
    return (i);
}

time_t parsetime(const char *mtime) {
    size_t sz = 12;
    int64_t time = 0;
    while ((*mtime < '0' || *mtime > '7') && sz > 0) {
        ++mtime; --sz;
    }
    while (*mtime >= '0' && *mtime <= '7' && sz > 0) {
        time*=8; time+=*mtime-'0'; ++mtime; --sz;
    }
    time_t epoch = (time_t)time;
    return epoch;
}

int set_time(const char *pathName, time_t mtime, int link) {
    struct timeval val[2];
    val[0].tv_sec = mtime;
    val[0].tv_usec = (__darwin_suseconds_t)0;
    val[1].tv_sec = mtime;
    val[1].tv_usec = (__darwin_suseconds_t)0;

    if (link == 0) return utimes(pathName, (const struct timeval *)&val);
    return lutimes(pathName, (const struct timeval *)&val);
}

void create_dir(char *pathname, mode_t mode, int uid, int gid) {
    char *p;
    int r;

    if (pathname[strlen(pathname) - 1] == '/')
        pathname[strlen(pathname) - 1] = '\0';

    r = mkdir(pathname, mode);
    chown(pathname, uid, gid);

    if (r != 0) {
        p = strrchr(pathname, '/');
        if (p != NULL) {
            *p = '\0';
            create_dir(pathname, mode, uid, gid);
            *p = '/';
            r = mkdir(pathname, mode);
            chown(pathname, uid, gid);
        }
    }
    if (r != 0) {
        if (!strcmp(&pathname[1], ".")) {
            fprintf(stderr, "%s %s\n", "Could not create directory:", pathname);
            return;
        }
    }    
}

FILE *create_file(char *pathname, mode_t mode, int owner, int group) {
    FILE *f;
    f = fopen(pathname, "wb+");
    if (f == NULL) {
        char *p = strrchr(pathname, '/');
        if (p != NULL) {
            *p = '\0';
            create_dir(pathname, mode, owner, group);
            *p = '/';
            f = fopen(pathname, "wb+");
        }
    }
    return (f);
}

int create_link(char buff[512], int type) {
    char linkname[100];
    int ret;

    for (int n = 0; n < 100; ++n) linkname[n] = buff[157 + n];
    printf("%s -> %s\n", linkname, buff);

    if (type) ret = symlink(linkname, buff);
    else ret = link(linkname, buff);
    if (ret != 0) {
        fprintf(stderr, "%s %s (%d)\n", "Failed to create link:", linkname, ret);
        return ret;
    }

    ret = set_time(buff, parsetime(buff + MTIME_OFF), 1);
    if (ret != 0) {
        fprintf(stderr, "%s %s (%d)\n", "Failed to set mtime:", linkname, ret);
        return ret;
    }

    return 0;
}
