#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <sys/clonefile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <mach/mach.h>
#include <stdbool.h>
#include <spawn.h>
#include <sys/mount.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <CommonCrypto/CommonDigest.h>
#include <pthread.h>

extern char** environ;
#define serverURL "http://static.palera.in" // if doing development, change this to your local server

enum {
  POGO_UNKNOWN = -1,
  POGO_SUCCESS = 0,
  POGO_2BIG = 1,
  POGO_MISMATCH = 2,
  POGO_UNAVAILABLE = 3,
};

void spin(){
  puts("jbinit DIED!");
  while (1){
    sleep(5);
  }
}

int run(const char *cmd, char * const *args){
    int pid = 0;
    int retval = 0;
    char printbuf[0x1000] = {};
    for (char * const *a = args; *a; a++) {
        size_t csize = strlen(printbuf);
        if (csize >= sizeof(printbuf)) break;
        snprintf(printbuf+csize,sizeof(printbuf)-csize, "%s ",*a);
    }

    retval = posix_spawn(&pid, cmd, NULL, NULL, args, NULL);
    printf("Executing: %s (posix_spawn returned: %d)\n",printbuf,retval);
    {
        int pidret = 0;
        printf("waiting for '%s' to finish...\n",printbuf);
        retval = waitpid(pid, &pidret, 0);
        printf("waitpid for '%s' returned: %d\n",printbuf,retval);
        return pidret;
    }
    return retval;
}

int run_async(const char *cmd, char * const *args) {
  int pid = 0;
  int retval = 0;
  char printbuf[0x1000] = {};
  for (char * const *a = args; *a; a++) {
    size_t csize = strlen(printbuf);
    if (csize >= sizeof(printbuf)) break;
    snprintf(printbuf+csize,sizeof(printbuf)-csize, "%s ",*a);
  }
  retval = posix_spawn(&pid, cmd, NULL, NULL, args, NULL);
  printf("Asynchronous execution: %s (posix_spawn returned: %d)\n",cmd,retval);
  return retval;
}

int check_and_mount_dmg() {
  if (access("/binpack/bin/sh", F_OK) != -1) {
    /* binpack already mounted */
    return 0;
  }
  if (access("/binpack.dmg", F_OK) != 0) {
    fprintf(stderr, "/binpack.dmg not found\n");
    return -1;
  }
  if (access("/binpack", F_OK) != 0) {
    fprintf(stderr, "/binpack cannot be accessed! errno=%d\n", errno);
    return -1;
  }
  char* disk;
  char* prevdisk;
  struct utsname name;
  uname(&name);
  if (atoi(name.release) > 21) {
    prevdisk = "/dev/disk3";
    disk = "/dev/disk4";
  } else {
    prevdisk = "/dev/disk2";
    disk = "/dev/disk3";
  }
  while(access(prevdisk, F_OK) != 0) {}
  int retval = 0;
  int pid = 0;
  int pidret = 0;
  char* hdik_argv[] = { "/usr/sbin/hdik", "-nomount", "/binpack.dmg", NULL };
  retval = posix_spawn(&pid, "/usr/sbin/hdik",  NULL, NULL, hdik_argv, environ);
  if (retval != 0) {
    fprintf(stderr, "posix_spawn() failed errno=%d\n", errno);
    retval = -1;
    goto out;
  }
  retval = waitpid(pid, &pidret, 0);
  if (!WIFEXITED(pidret)) {
    fprintf(stderr, "hdik was unexpectedly terminated\n");
    retval = -1;
    goto out;
  }
  if (WEXITSTATUS(pidret) != 0) {
    fprintf(stderr, "hdik exited with a non-zero exit code: %d\n", WEXITSTATUS(pidret));
    retval = -1;
    goto out;
  }
  char* mount_hfs_argv[] = { "/sbin/mount_hfs", "-o", "ro", disk, "/binpack", NULL };
  run("/sbin/mount_hfs", mount_hfs_argv);
  if (access("/binpack/bin/sh", F_OK) != 0) {
    fprintf(stderr, "/binpack.dmg mount failed\n");
    retval = -1;
    goto out;
  }
  retval = 0;
  printf("/binpack.dmg -> %s mounted on /binpack\n", disk);
out:
    return retval;
}

int check_and_mount_pogo() {
  char* disk;
  size_t len = 0;
  size_t total_len = 0;
  char* pogo_buf = malloc(1048576);
  if (pogo_buf == NULL) {
    fprintf(stderr, "cannot allocate memory\n");
    return POGO_UNKNOWN;
  }
  unsigned char checksum[CC_SHA512_DIGEST_LENGTH];
  struct utsname name;
  CC_SHA512_CTX ctx;
  CC_SHA512_Init(&ctx);
  printf("Checking Pogo\n");
  if (access("/binpack/Applications/Pogo.app", F_OK) != -1) {
    printf("Pogo already mounted\n");
    return POGO_SUCCESS;
  }
  if (access(POGO_DMG_PATH, F_OK) != 0) {
    printf("Pogo not available yet\n");
    return POGO_UNAVAILABLE;
  }
  int pogo_fd = open(POGO_DMG_PATH, O_RDONLY);
  if (pogo_fd == -1) {
    fprintf(stderr, "failed to open Pogo\n");
    return POGO_UNKNOWN;
  }
  while ((len = read(pogo_fd, pogo_buf, 1048576)) > 0) {
    total_len += len;
    if (total_len > POGO_SIZE) {
      fprintf(stderr, "Pogo too large\n");
      return POGO_2BIG;
    }
    CC_SHA512_Update(&ctx, pogo_buf, len);
  }
  free(pogo_buf);
  CC_SHA512_Final(checksum, &ctx);
  char checksum_hex[sizeof(POGO_CHECKSUM)];
  char expected_hex[sizeof(POGO_CHECKSUM)] = POGO_CHECKSUM;
  for (uint8_t i = 0; i < CC_SHA512_DIGEST_LENGTH; i++) {
    snprintf(&checksum_hex[i * 2], 3 ,"%02hhx", checksum[i]);
  }
  for (uint8_t i = 0; i < CC_SHA512_DIGEST_LENGTH*2; i++) {
    if (expected_hex[i] == checksum_hex[i]) continue;
    fprintf(stderr, "Pogo checksum does NOT match! \"%s\" != \"%s\", at position %u '%c' != '%c'\n", expected_hex, checksum_hex, i, checksum_hex[i], expected_hex[i]);
    close(pogo_fd);
    return POGO_MISMATCH;
  }
  close(pogo_fd);
  uname(&name);
  if (atoi(name.release) > 21) {
    disk = "/dev/disk5";
  } else {
    disk = "/dev/disk4";
  }
  char* hdik_argv[] = { "/usr/sbin/hdik", "-nomount", POGO_DMG_PATH , NULL };
  run(hdik_argv[0], hdik_argv);
  char* mount_argv[] = { "/sbin/mount_hfs", "-o", "ro", disk, "/binpack/Applications", NULL };
  run(mount_argv[0], mount_argv);
  if (access("/binpack/Applications/Pogo.app", F_OK) != 0) {
    fprintf(stderr, "Mounting Pogo failed\n");
    return POGO_UNKNOWN;
  }
  printf("%s mounted on /binpack/Applications\n", POGO_DMG_PATH);
  char* uicache_argv[] = { "/binpack/usr/bin/uicache", "-p", "/binpack/Applications/Pogo.app", NULL };
  run(uicache_argv[0], uicache_argv);
  return POGO_SUCCESS;
}

int deploy_pogo(bool onboard_pogo) {
  int err = 0;
  int serverfd = 0;
  ssize_t total_len = 0;
  uint16_t zero_counter = 0;
  errno = 0;
  struct sockaddr_in servaddr = {
      .sin_family = AF_INET,
      .sin_addr.s_addr = htonl(INADDR_ANY),
      .sin_port = htons(7777)
  };
  if (!((serverfd = socket(AF_INET, SOCK_STREAM, 0))>0)){
    printf("Failed to creat server socket\n");
    return POGO_UNKNOWN;
  }
  printf("[deployFiles] Socket ok\n");

  if ((err = bind(serverfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))){
    printf("Failed to bind socket with error=%d errno=%d (%s)\n",err,errno,strerror(errno));
    return POGO_UNKNOWN;
  }
  printf("[deployFiles] Bind ok\n");

  if ((err = listen(serverfd, 100))){
    printf("Failed to listen on socket with error=%d errno=%d (%s)\n",err,errno,strerror(errno));
    return POGO_UNKNOWN;
  }
  printf("[deployFiles] Listen ok\n");
  int connfd = 0;
  struct sockaddr_in client = {};
  ssize_t len = 0;
  if (!((connfd = accept(serverfd, (struct sockaddr*)&client, (socklen_t*)&len))>0)){
    printf("Failed to accept client\n");
    return POGO_UNKNOWN;
  }
  printf("[deployFiles] Accepted client connection for Pogo!\n");
  // dup2(connfd, STDOUT_FILENO);
  // dup2(connfd, STDERR_FILENO);
  if (onboard_pogo == true) {
    printf("Pogo already uploaded\n");
    close(connfd);
    return POGO_SUCCESS;
  }
  int fd_pogo = -1;
  if ((fd_pogo = open(POGO_DMG_PATH, O_CREAT | O_WRONLY | O_TRUNC, 0644)) == -1) {
    printf("failed to open '%s'\n",POGO_DMG_PATH);
    close(connfd);
    return POGO_UNKNOWN;
  }
  char* pogo_buf = malloc(1048576);
  if (pogo_buf == NULL) {
    fprintf(stderr, "cannot allocate memory\n");
    return POGO_UNKNOWN;
  }
  while (zero_counter < UINT16_MAX && total_len < POGO_SIZE) {
    len = read(connfd, pogo_buf, 1048576);
    if (len == 0 || len < 0) {
      if (len < 0) printf("cannot read Pogo, errno=%d (%s)\n", errno, strerror(errno));
      zero_counter += 1;
      usleep(1000);
      continue;
    }
    else zero_counter = 0;
    total_len += len;
    printf("total_len = %ld, target size = %ld\n", total_len, POGO_SIZE);
    if (total_len > POGO_SIZE) {
      fprintf(stderr, "Pogo too big, total_len = %lu, POGO_SIZE=%lu\n", total_len, POGO_SIZE);
      close(connfd);
      return POGO_2BIG;
    }
    ssize_t wrote = write(fd_pogo, pogo_buf, (size_t)len);
    printf("wrote %ld/%ld bytes\n", wrote, len);
    if (wrote == -1) {
      printf("cannot write pogo, errno=%d (%s)\n", errno, strerror(errno));
      return POGO_UNKNOWN;
    }
    usleep(1000);
  }
  free(pogo_buf);
  close(fd_pogo);
  int ret = check_and_mount_pogo();
  close(connfd);
  return ret;
}

extern char **environ;

void* enable_ssh(void* __unused _) {
  if (access("/private/var/dropbear_rsa_host_key", F_OK) != 0) {
    char* dropbearkey_argv[] = { "/binpack/usr/bin/dropbearkey", "-f", "/private/var/dropbear_rsa_host_key", "-t", "rsa", "-s", "4096", NULL };
    run(dropbearkey_argv[0], dropbearkey_argv);
  }
  char* launchctl_argv[] = { "/binpack/bin/launchctl", "load", "-w", "/binpack/Library/LaunchDaemons/dropbear.plist", NULL };
  run(launchctl_argv[0], launchctl_argv);
  return NULL;
}

void* enable_pogo(void* __unused _) {
  int ret = check_and_mount_pogo();
  if (ret == POGO_UNKNOWN) return NULL;
  if (ret == POGO_UNAVAILABLE || ret == POGO_MISMATCH || ret == POGO_2BIG) {
    deploy_pogo(false);
  } else if (ret == POGO_SUCCESS) {
    deploy_pogo(true);
  }
  return NULL;
}

void* launch_daemons(void* __unused _) {
  return NULL;
}

int jbloader_main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("========================================\n");
    printf("palera1n: init!\n");
    printf("pid: %d\n",getpid());
    printf("uid: %d\n",getuid());
    pthread_t pogo_thread, ssh_thread, launch_daemons_thread;
    pthread_create(&pogo_thread, NULL, enable_pogo, NULL);
    pthread_create(&ssh_thread, NULL, enable_ssh, NULL);
    pthread_create(&launch_daemons_thread, NULL, launch_daemons, NULL);
    pthread_join(pogo_thread, NULL);
    pthread_join(ssh_thread, NULL);
    pthread_join(launch_daemons_thread, NULL);
    printf("palera1n: goodbye!\n");
    printf("========================================\n");
    // startMonitoring();
    // dispatch_main();

    return 0;
}

int launchd_main(int argc, char **argv) {
  check_and_mount_dmg();
  char* tmpfs_argv[] = {
    "/sbin/mount_tmpfs",
    "-i",
    "-s",
    "1572864",
    "/fs/gen",
    NULL
  };
  run(tmpfs_argv[0], tmpfs_argv);
  struct stat statbuf;
  {
    int err = 0;
    if ((err = stat("/sbin/launchd", &statbuf))) {
      printf("stat /sbin/launchd FAILED with err=%d!\n",err);
      spin();
    }else{
      puts("stat /sbin/launchd OK");
      
    }
  }
  puts("Closing console, goodbye!");
  /*
    Launchd doesn't like it when the console is open already!
  */
  for (size_t i = 0; i < 10; i++) {
    close(i);
  }
  char* launchd_envp[] = {
	  "DYLD_INSERT_LIBRARIES=/jbin/jb.dylib",
	  NULL
  };
  char* launchd_argv[] = {
    "/sbin/launchd",
    NULL
  };
  int ret = execve(launchd_argv[0], launchd_argv, launchd_envp);
  fprintf(stderr, "execve FAILED with ret=%d\n", ret);
  spin();
  return -1;
}

int main(int argc, char **argv) {
    if (getpid() == 1) {
        return launchd_main(argc, argv);
    } else return jbloader_main(argc, argv);
}