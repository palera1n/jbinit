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
      run(args[0], args);
    }
    free(pp);
  }
  closedir(d);
  return 0;
}

int jbloader_sysstatuscheck(int argc, char *argv[])
{
  if (checkrain_option_enabled(pinfo.flags, palerain_option_jbinit_log_to_file))
  {
    int fd_log = open("/cores/jbinit.log", O_WRONLY | O_APPEND | O_SYNC, 0644);
    if (fd_log != -1)
    {
      dup2(fd_log, STDOUT_FILENO);
      dup2(fd_log, STDERR_FILENO);
      fputs("======== jbloader (sysstatuscheck) log start =========", stderr);
    }
    else
      fputs("cannot open /cores/jbinit.log for logging", stderr);
  }
  puts(
    "#==================================\n"
    "# palera1n loader (sysstatuscheck) \n"
    "#      (c) palera1n develope r     \n"
    "#==================================="
  );
  enable_non_default_system_apps();
  if (!checkrain_option_enabled(jbloader_flags, jbloader_userspace_rebooted)) {
    remount(pinfo.rootdev);
  }
  if (!checkrain_option_enabled(info.flags, checkrain_option_safemode) && 
    !checkrain_option_enabled(info.flags, checkrain_option_force_revert) 
    ) load_etc_rc_d();
  execv("/usr/libexec/sysstatuscheck", (char*[]){"/usr/libexec/sysstatuscheck", NULL });
  fprintf(stderr, "execve %s failed: %d (%s)\n", "/usr/libexec/sysstatuscheck", errno, strerror(errno));
  spin();
  return 0;
}
