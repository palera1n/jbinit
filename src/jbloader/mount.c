#include <jbloader.h>

int load_etc_rc_d()
{
  DIR *d = NULL;
  struct dirent *dir = NULL;
  if (checkrain_option_enabled(pinfo.flags, palerain_option_rootful))
  {
    d = opendir("/etc/rc.d/");
  }
  else
  {
    d = opendir("/var/jb/etc/rc.d/");
  }
  if (!d)
  {
    printf("Failed to open dir with err=%d (%s)\n", errno, strerror(errno));
    return 0;
  }
  while ((dir = readdir(d)))
  { // remove all subdirs and files
    if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
    {
      continue;
    }
    char *pp = NULL;
    if (checkrain_option_enabled(pinfo.flags, palerain_option_rootful))
    {
      asprintf(&pp, "/etc/rc.d/%s", dir->d_name);
    }
    else
    {
      asprintf(&pp, "/var/jb/etc/rc.d/%s", dir->d_name);
    }
    {
      char *args[] = {
          pp,
          NULL};
      run_async(args[0], args);
    }
    free(pp);
  }
  closedir(d);
  return 0;
}

int mount_main(int argc, char *argv[])
{
  if (checkrain_option_enabled(pinfo.flags, palerain_option_jbinit_log_to_file))
  {
    int fd_log = open("/cores/jbinit.log", O_WRONLY | O_APPEND | O_SYNC, 0644);
    if (fd_log != -1)
    {
      dup2(fd_log, STDOUT_FILENO);
      dup2(fd_log, STDERR_FILENO);
      fputs("======== jbloader (mount) log start =========", stderr);
    }
    else
      fputs("cannot open /cores/jbinit.log for logging", stderr);
  }
  if (!userspace_rebooted) {
    char* mount_argv[] = {
      "/sbin/mount",
      "-P",
      "2",
      NULL
    };
    int mount_ret = run(mount_argv[0], mount_argv);
    if (!WIFEXITED(mount_ret)) {
      int termsig = 0;
      if (WIFSIGNALED(mount_ret)) {
        termsig = WTERMSIG(mount_ret);
        fprintf(stderr, "/sbin/mount -P 2 exited due to signal %d\n", termsig);
      } else {
        fprintf(stderr, "/sbin/mount -P 2 exited abnormally\n");
      }
      spin();
    }
    if (WEXITSTATUS(mount_ret) != 0) {
      fprintf(stderr, "/sbin/mount -P 2 exited with code %d\n", WEXITSTATUS(mount_ret));
      spin();
    }
    remount(pinfo.rootdev);
  }
  if (!checkrain_option_enabled(info.flags, checkrain_option_safemode) && 
    !checkrain_option_enabled(info.flags, checkrain_option_force_revert) 
    ) load_etc_rc_d();
  return 0;
}
