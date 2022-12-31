
#include <stdint.h>
#include "printf.h"

char ios15_rootdev[] = "/dev/disk0s1s1";
char ios16_rootdev[] = "/dev/disk1s1";

asm(
  ".globl __dyld_start\n"
  ".align 4\n"
  "__dyld_start:\n"
  "movn x8, #0xf\n"
  "mov x7, sp\n"
  "and x7, x7, x8\n"
  "mov sp, x7\n"
  "bl _main\n"
  "movz x16, #0x1\n"
  "svc #0x80\n"
);

#define STDOUT_FILENO 1
#define getpid() msyscall(20)
#define exit(err) msyscall(1,err)
#define fork() msyscall(2)
#define puts(str) write(STDOUT_FILENO,str,sizeof(str)-1)
#define fbi(mnt,dir) do { int fbi_ret = mount("bindfs", mnt, MNT_RDONLY, dir); if (fbi_ret != 0) { printf("cannot bind %s onto %s\n", dir, mnt); spin(); } else { printf("bound %s onto %s\n", dir, mnt); } } while(0)

typedef uint32_t kern_return_t;
typedef uint32_t mach_port_t;
typedef uint64_t mach_msg_timeout_t;
// typedef uint64_t size_t;

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2
#define O_CREAT         0x00000200      /* create if nonexistant */

#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

#define PROT_NONE       0x00    /* [MC2] no permissions */
#define PROT_READ       0x01    /* [MC2] pages can be read */
#define PROT_WRITE      0x02    /* [MC2] pages can be written */
#define PROT_EXEC       0x04    /* [MC2] pages can be executed */

#define MAP_FILE        0x0000  /* map from file (default) */
#define MAP_ANON        0x1000  /* allocated from memory, swap space */
#define MAP_ANONYMOUS   MAP_ANON
#define MAP_SHARED      0x0001          /* [MF|SHM] share changes */
#define MAP_PRIVATE     0x0002          /* [MF|SHM] changes are private */


#define	MNT_RDONLY	0x00000001
#define	MNT_LOCAL	  0x00001000
#define MNT_ROOTFS      0x00004000      /* identifies the root filesystem */
#define MNT_UNION       0x00000020
#define MNT_UPDATE      0x00010000      /* not a real mount, just an update */
#define MNT_NOBLOCK     0x00020000      /* don't block unmount if not responding */
#define MNT_RELOAD      0x00040000      /* reload filesystem data */
#define MNT_FORCE       0x00080000      /* force unmount or readonly change */


__attribute__((naked)) kern_return_t thread_switch(mach_port_t new_thread,int option, mach_msg_timeout_t time){
  asm(
    "movn x16, #0x3c\n"
    "svc 0x80\n"
    "ret\n"
  );
}

__attribute__((naked)) uint64_t msyscall(uint64_t syscall, ...){
  asm(
    "mov x16, x0\n"
    "ldp x0, x1, [sp]\n"
    "ldp x2, x3, [sp, 0x10]\n"
    "ldp x4, x5, [sp, 0x20]\n"
    "ldp x6, x7, [sp, 0x30]\n"
    "svc 0x80\n"
    "ret\n"
  );
}

void sleep(int secs){
  thread_switch(0,2,secs*1000);
}

int sys_dup2(int from, int to){
  return msyscall(90,from,to);
}

int stat(void *path, void *ub){
  return msyscall(188,path,ub);
}

int mkdir(void *path, int mode){
  return msyscall(136,path,mode);
}

int chroot(void *path){
  return msyscall(61,path);
}

int mount(char *type, char *path, int flags, void *data){
  return msyscall(167,type,path,flags,data);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, uint64_t offset){
  return (void*)msyscall(197,addr,length,prot,flags,fd,offset);
}

uint64_t read(int fd, void* cbuf, size_t nbyte){
  return msyscall(3,fd,cbuf,nbyte);
}

uint64_t write(int fd, void* cbuf, size_t nbyte){
  return msyscall(4,fd,cbuf,nbyte);
}

int close(int fd){
  return msyscall(6,fd);
}

int open(void *path, int flags, int mode){
  return msyscall(5,path,flags,mode);
}

int execve(char *fname, char *const argv[], char *const envp[]){
  return msyscall(59,fname,argv,envp);
}

void _putchar(char character){
  static size_t chrcnt = 0;
  static char buf[0x100];
  buf[chrcnt++] = character;
  if (character == '\n' || chrcnt == sizeof(buf)){
    write(STDOUT_FILENO,buf,chrcnt);
    chrcnt = 0;
  }
}


void spin(){
  puts("jbinit DIED!\n");
  while (1){
    sleep(5);
  }
}

void memcpy(void *dst, void *src, size_t n){
  uint8_t *s =(uint8_t *)src;
  uint8_t *d =(uint8_t *)dst;
  for (size_t i = 0; i<n; i++) *d++ = *s++;
}

void memset(void *dst, int c, size_t n){
  uint8_t *d =(uint8_t *)dst;
  for (size_t i = 0; i<n; i++) *d++ = c;
}

int main(){
  int fd_console = open("/dev/console",O_RDWR,0);
  sys_dup2(fd_console,0);
  sys_dup2(fd_console,1);
  sys_dup2(fd_console,2);
  char statbuf[0x400];

  puts("================ Hello from jbinit ================ \n");

  // I HAVE NO IDEA WHY THIS IS NEEDED DONT REMOVE
  int fd_jbloader = 0;
  fd_jbloader = open("/sbin/launchd",O_RDONLY,0);
  if (fd_jbloader == -1) {
    spin();
  }
  size_t jbloader_size = msyscall(199,fd_jbloader,0,SEEK_END);
  msyscall(199,fd_jbloader,0,SEEK_SET);
  void *jbloader_data = mmap(NULL, (jbloader_size & ~0x3fff) + 0x4000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
  if (jbloader_data == (void*)-1) {
    spin();
  }
  int didread = read(fd_jbloader,jbloader_data,jbloader_size);
  close(fd_jbloader);

  puts("Checking for roots\n");
  char* rootdev;
  {
    while (stat(ios15_rootdev, statbuf) && stat(ios16_rootdev, statbuf) ) {
      puts("waiting for roots...\n");
      sleep(1);
    }
  }
  if (stat(ios15_rootdev, statbuf)) {
    rootdev = ios16_rootdev;
  } else rootdev = ios15_rootdev;
  printf("Got rootfs %s\n", rootdev);

  {
    char buf[0x100];
    struct apfs_mountarg {
      char *path;
      uint64_t _null;
      uint64_t mountAsRaw;
      uint32_t _pad;
      char snapshot[0x100];
    } arg = {
      rootdev,
      0,
      0, //1 mount without snapshot, 0 mount snapshot
      0,
    };
    int err = 0;
retry_rootfs_mount:
    puts("mounting rootfs\n");
    err = mount("apfs","/fs/orig",MNT_RDONLY, &arg);
    if (!err) {
      puts("mount rootfs OK\n");
    }else{
      printf("mount rootfs FAILED with err=%d!\n",err);
      sleep(1);
      // spin();
    }


    if (stat("/fs/orig/private/", statbuf)) {
      printf("stat /fs/orig/private/ FAILED with err=%d!\n",err);
      sleep(1);
      goto retry_rootfs_mount;
    }else{
      puts("stat /fs/orig/private/ OK\n");
    }
  }

  puts("mounting devfs\n");
  {
    char *path = "devfs";
    int err = mount("devfs","/dev/",0, path);
    if (!err) {
      puts("mount devfs OK\n");
    }else{
      printf("mount devfs FAILED with err=%d!\n",err);
      spin();
    }
  }

  {
    fbi("/Applications", "/fs/orig/Applications");
    fbi("/bin","/fs/orig/bin");
    fbi("/private", "/fs/orig/private");
    fbi("/Library", "/fs/orig/Library");
    fbi("/System", "/fs/orig/System");
    fbi("/usr", "/fs/orig/usr");
    fbi("/sbin", "/fs/orig/sbin");
  }

  {
    char **argv = (char **)jbloader_data;
    char **envp = argv+2;
    char *strbuf = (char*)(envp+2);
    argv[0] = strbuf;
    argv[1] = NULL;
    memcpy(strbuf,"/jbin/jbloader",sizeof("/jbin/jbloader"));
    strbuf += sizeof("/jbin/jbloader");
    int err = execve(argv[0],argv,NULL);
    if (err) {
      printf("execve FAILED with err=%d!\n",err);
      spin();
    }
  }
  puts("FATAL: shouldn't get here!\n");
  spin();

  return 0;
}
