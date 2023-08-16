#include <stdio.h>
#include <spawn.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <mount_args.h>

int attach_dmg(const char *source, char* device_path, size_t device_path_len);

_Noreturn void spin(int fd_console) {
    dprintf(fd_console, "jbinit DIED!\n");
    while(1) {
        sleep(3600);
    }
}

int main(void) {
    spin(1);
    int fd_console = open("/dev/console", O_RDWR);
    if (fd_console == -1) {
        spin(fd_console);
    }
    dprintf(fd_console, "mounting /cores");
    /*if (getppid() != 1) {
        fprintf(stderr, "this is a plooshra1n internal utility, do not use\n");
        return -1;
    }*/
    char device_path[50];
    int ret = attach_dmg("ram://2048", device_path, 50);
    if (ret) spin(fd_console);
    pid_t pid;
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/console", O_RDWR, 0);
    posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/console", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/console", O_WRONLY, 0);
    ret = posix_spawn(&pid, "/sbin/newfs_hfs", &actions, NULL, (char*[]){ "/sbin/newfs_hfs", "-s", "-v", "nebula is a furry", device_path, NULL } , NULL);
    posix_spawn_file_actions_destroy(&actions);
    if (ret) {
        dprintf(fd_console, "faild to spawn /sbin/newfs_hfs: %d\n", errno);
        spin(fd_console);
    }
    int status;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status)) {
        dprintf(fd_console, "/sbin/newfs_hfs failed\n");
        spin(fd_console);
    }
    struct hfs_mount_args cores_mountarg = { device_path, 0, 0, 0, 0, { 0, 0 }, HFSFSMNT_EXTENDED_ARGS, 0, 0, 1 };
    ret = mount("hfs", "/cores", 0, &cores_mountarg);
    if (ret) {
        dprintf(fd_console, "mount failed: %d\n", errno);
        spin(fd_console);
    }
    return 0;
}
