#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <mach/mach.h>
#include <sys/mount.h>
#include <mount_args.h>
#include <pthread.h>
#include <mach/error.h>
#include <payload/payload.h>

#define HDI_MAGIC 0x1beeffeed

struct HDIImageCreateBlock64
{
  uint64_t magic;
  const void *props;
  uint64_t props_size;
  char padding[0x100 - 24];
};

#define DIRECT_MOUNT_DMG
#ifndef DIRECT_MOUNT_DMG
struct mount_dmg_options {
  const char* source;
  const char* fstype;
  const char* mnt;
  const int mntopts;
  bool is_overlay;
  int ret;
  bool mount_attempted;
  pthread_cond_t condition;
};

struct mount_dmg_timeout_options {
  pthread_t thread;
  pthread_cond_t condition;
};

#else
#define mount_dmg_internal mount_dmg
#endif

int mount_dmg_internal(const char *device, const char *fstype, const char *mnt, const int mntopts, bool is_overlay)
{
  CFDictionaryKeyCallBacks key_callback = kCFTypeDictionaryKeyCallBacks;
  CFDictionaryValueCallBacks value_callback = kCFTypeDictionaryValueCallBacks;
  CFAllocatorRef allocator = kCFAllocatorDefault;
  CFMutableDictionaryRef hdix = IOServiceMatching("IOHDIXController");
  io_service_t hdix_service = IOServiceGetMatchingService(kIOMasterPortDefault, hdix);
  io_connect_t connect;
  assert(hdix_service != 0);
  kern_return_t open_hdix = IOServiceOpen(hdix_service, mach_task_self(), 0, &connect);
  assert(open_hdix == KERN_SUCCESS);
  fprintf(stderr, "IOServiceOpen: %d\n", open_hdix);
  CFMutableDictionaryRef props = CFDictionaryCreateMutable(allocator, 0, &key_callback, &value_callback);
  CFUUIDRef uuid = CFUUIDCreate(allocator);
  CFStringRef uuid_string = CFUUIDCreateString(0, uuid);
  size_t device_path_len = strlen(device);
  CFDataRef path_bytes = CFDataCreateWithBytesNoCopy(allocator, (unsigned char *)device, device_path_len, kCFAllocatorNull);
  assert(path_bytes != 0);
  CFDictionarySetValue(props, CFSTR("hdik-unique-identifier"), uuid_string);
  CFDictionarySetValue(props, CFSTR("image-path"), path_bytes);
  CFDictionarySetValue(props, CFSTR("autodiskmount"), kCFBooleanFalse);
  CFDictionarySetValue(props, CFSTR("removable"), kCFBooleanTrue);
  CFMutableDictionaryRef image_secrets = NULL;
  if (is_overlay)
  {
    image_secrets = CFDictionaryCreateMutable(allocator, 0, &key_callback, &value_callback);
    CFDictionarySetValue(image_secrets, CFSTR("checkra1n-overlay"), kCFBooleanTrue);
    CFDictionarySetValue(props, CFSTR("image-secrets"), image_secrets);
  }
  CFDataRef hdi_props = CFPropertyListCreateData(allocator, props, kCFPropertyListXMLFormat_v1_0, 0, 0);
  assert(hdi_props != 0);
  struct HDIImageCreateBlock64 hdi_stru;
  memset(&hdi_stru, 0, sizeof(hdi_stru));
  hdi_stru.magic = HDI_MAGIC;
  hdi_stru.props = (char *)CFDataGetBytePtr(hdi_props);
  hdi_stru.props_size = CFDataGetLength(hdi_props);
  volatile unsigned long four_L = 4L;
  uint32_t val;
  size_t val_size = sizeof(val);
  kern_return_t stru_ret = IOConnectCallStructMethod(connect, 0, &hdi_stru, sizeof(hdi_stru), &val, &val_size);
  if (stru_ret != 0)
  {
    fprintf(stderr, "IOConnectCallStructMethod(connect, 0, &hdi_stru, sizeof(hdi_stru), &val, &val_size) returned %x %s\n", stru_ret, mach_error_string(stru_ret));
    return 1;
  }
  assert(four_L == 4);
  CFMutableDictionaryRef pmatch = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(pmatch, CFSTR("hdik-unique-identifier"), uuid_string);
  CFMutableDictionaryRef matching = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(matching, CFSTR("IOPropertyMatch"), pmatch);
  hdix_service = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
  if (hdix_service == 0)
  {
    fprintf(stderr, "successfully attached, but didn't find top entry in IO registry\n");
    return 1;
  }
  io_iterator_t iter;
  kern_return_t iterator_ret = IORegistryEntryCreateIterator(hdix_service, kIOServicePlane, kIORegistryIterateRecursively, &iter);
  if (iterator_ret != KERN_SUCCESS)
  {
    fprintf(stderr, "IORegistryEntryCreateIterator(hdix_service, kIOServicePlane, 1, &iter) returned %x %s\n", iterator_ret, mach_error_string(iterator_ret));
    return 1;
  };
  uint8_t not_mount_ret = 0;
  while (1)
  {
    io_object_t next = IOIteratorNext(iter);
    if ((int)next == 0)
      break;
    CFStringRef bsd_name = (CFStringRef)IORegistryEntryCreateCFProperty(next & 0xffffffff, CFSTR("BSD Name"), 0, 0);
    IOObjectRelease(next);
    char buf[1024];
    if (bsd_name == 0) {
      continue;
    }
    char cstring = CFStringGetCString(bsd_name, buf, sizeof(buf), kCFStringEncodingUTF8);
    assert(cstring != '\0');
    puts(buf);
    char diskdev_name_buf[512];
    bzero(&diskdev_name_buf, sizeof(diskdev_name_buf));
    snprintf(diskdev_name_buf, sizeof(diskdev_name_buf), "/dev/%s", buf);
    char *dev2 = strdup(diskdev_name_buf);
    hfs_mount_args_t args = { dev2 };
    printf("calling mount(fstype=%s, mnt=%s, mntopts=%d, data=%p)\n", fstype, mnt, mntopts, &args);
    int mount_ret = mount(fstype, mnt, mntopts, &args);
    if (mount_ret == 0)
    {
      not_mount_ret = 1;
    }
    CFRelease(bsd_name);
    break;
  }
#if 1
  if (image_secrets) CFRelease(image_secrets);
  CFRelease(pmatch);
  CFRelease(uuid_string);
  CFRelease(uuid);
  CFRelease(path_bytes);
  CFRelease(hdi_props);
  IOObjectRelease(hdix_service);
  IOObjectRelease(iter);
#endif
  if ((not_mount_ret & 1) == 0)
  {
    fprintf(stderr, "successfully attached, but mounting failed (potentially due to entry not found): %d (%s)\n", errno, strerror(errno));
    return 1;
  }
  return 0;
}

#ifndef DIRECT_MOUNT_DMG
void* mount_dmg_thread(void* vopts) {
  struct mount_dmg_options* opts = (struct mount_dmg_options*)vopts;
  opts->ret = mount_dmg_internal(opts->source, opts->fstype, opts->mnt, opts->mntopts, opts->is_overlay);
  opts->mount_attempted = true;
  pthread_cond_broadcast(&opts->condition);
  return &opts->ret;
}

void* mount_dmg_timeout_thread(void* vopts) {
  struct mount_dmg_timeout_options* opts = (struct mount_dmg_timeout_options*)vopts;
  sleep(4);
  fprintf(stderr, "mount_dmg thread timed out\n");
  pthread_cancel(opts->thread);
  pthread_cond_broadcast(&opts->condition);
  return NULL;
}

int mount_dmg(const char *source, const char *fstype, const char *mnt, const int mntopts, bool is_overlay) {
  struct mount_dmg_options options = {
    source, fstype, mnt, mntopts, is_overlay, 0, 0
  };
  while (!options.mount_attempted && !options.ret) {
    errno = 0;
    pthread_t dmg_thread, timeout_thread;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    CHECK_ERROR(errno = pthread_mutex_init(&mutex, NULL), 1, "pthread_mutex_init");;
    CHECK_ERROR(errno = pthread_cond_init(&condition, NULL), 1, "pthread_cond_init");
    struct mount_dmg_timeout_options timeout_options = {
      dmg_thread, condition
    };
    options.condition = condition;
    CHECK_ERROR(errno = pthread_create(&dmg_thread, NULL, mount_dmg_thread, &options), 1, "pthread_create");
    CHECK_ERROR(errno = pthread_create(&timeout_thread, NULL, mount_dmg_timeout_thread, &timeout_options), 1, "pthread_create");
    CHECK_ERROR(errno = pthread_mutex_lock(&mutex), 1, "pthread_mutex_lock");
    CHECK_ERROR(errno = pthread_cond_wait(&condition, &mutex), 1, "pthread_cond_wait");
    CHECK_ERROR(errno = pthread_mutex_unlock(&mutex), 1, "pthread_mutex_lock");
    CHECK_ERROR(errno = pthread_cancel(timeout_thread), 1, "pthread_cancel");
    CHECK_ERROR(errno = pthread_cancel(dmg_thread), 1, "pthread_cancel");
    CHECK_ERROR(errno = pthread_join(timeout_thread, NULL), 1, "pthread_join");
    // CHECK_ERROR(errno = pthread_join(dmg_thread, NULL), 1, "pthread_join");
    CHECK_ERROR(errno = pthread_mutex_destroy(&mutex), 1, "pthread_mutex_destroy");
    CHECK_ERROR(errno = pthread_cond_destroy(&condition), 1, "pthread_cond_destroy");
  }
  return options.ret;
}
#endif
