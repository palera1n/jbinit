#include <payload/payload.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pwd.h>
#include <grp.h>

#ifdef HAVE_SYSTEMWIDE_IOSEXEC
int fixup_pwgrp_file(char* sysfile, char* jbfile) {
    if (access(sysfile, F_OK) != 0) {
        fprintf(stderr, "cannot access %s\n", sysfile);
        return -1;
    }
    if (access(jbfile, F_OK) != 0) {
        fprintf(stderr, "cannot access %s\n", jbfile);
        return -1;
    }
    printf("fixing up %s with %s\n", jbfile, sysfile);
    int sys_fd, jb_fd;
    CHECK_ERROR((sys_fd = open(sysfile, O_RDONLY)) == -1, 1, "open %s", sysfile);
    CHECK_ERROR((jb_fd = open(jbfile, O_RDWR | O_APPEND)) == -1, 1, "open %s", jbfile);
    struct stat st;
    CHECK_ERROR(fstat(sys_fd, &st), 1, "fstat");
    char* sys_file_buf = calloc(1, st.st_size+1);
    if (!sys_file_buf) {
        fprintf(stderr, "calloc failed\n");
        spin();
    }
    ssize_t didRead = read(sys_fd, sys_file_buf, st.st_size);
    printf("read %zd bytes from %s\n", didRead, sysfile);

    CHECK_ERROR(fstat(jb_fd, &st), 1, "fstat");
    char* jb_file_buf = calloc(1, st.st_size+2);
    if (!jb_file_buf) {
        fprintf(stderr, "calloc failed\n");
        spin();
    }

    jb_file_buf[0] = '\n';
    jb_file_buf[st.st_size+1] = '\0';
    didRead = read(jb_fd, jb_file_buf+1, st.st_size);
    printf("read %zd bytes from %s\n", didRead, jbfile);
    puts("sysfile: ");
    puts(sys_file_buf);
    puts("jbfile: ");
    puts(jb_file_buf);

    char* entry;
    while ((entry = strsep(&sys_file_buf, "\n")) != NULL) {
        char* entry2 = strdup(entry);
        if (!entry2) {
            fprintf(stderr, "strdup() failed\n");
            spin();
        }
        char* name = strsep(&entry, ":");
        if (!name) {
            free(entry2);
            continue;
        }
        if (name[0] == '#' || name[0] == '\0' || name[0] == ' ') continue;
        char newlinename[260];
        snprintf(newlinename, 260, "\n%s:", name);
        if (strstr(jb_file_buf, newlinename) != NULL) continue;
        printf("missing name %s in %s\n", name, jbfile);
        CHECK_ERROR(lseek(jb_fd, SEEK_END, 0) == -1, 1, "lseek");
        CHECK_ERROR(write(jb_fd, entry2, strlen(entry2)) != strlen(entry2), 1, "write");
        CHECK_ERROR(write(jb_fd, "\n", 1) != 1, 1, "write");
        free(entry2);
    }

    free(jb_file_buf);
    close(sys_fd);
    close(jb_fd);
    return 0;
}

int fixup_databases(void) {
    fixup_pwgrp_file("/etc/passwd", "/var/jb/etc/passwd");
    fixup_pwgrp_file("/etc/master.passwd", "/var/jb/etc/master.passwd");
    fixup_pwgrp_file("/etc/group", "/var/jb/etc/group");
    return 0;
}
#endif
