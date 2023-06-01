#include <jbloader.h>

extern const mach_port_t kIOMasterPortDefault __API_AVAILABLE(ios(15.0));

int mount_dmg(const char *device, const char *fstype, const char *mnt, const int mntopts, bool is_overlay)
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
  if (is_overlay)
  {
    CFMutableDictionaryRef image_secrets = CFDictionaryCreateMutable(allocator, 0, &key_callback, &value_callback);
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
    char buf[1024];
    if (bsd_name == 0)
      continue;
    char cstring = CFStringGetCString(bsd_name, buf, sizeof(buf), kCFStringEncodingUTF8);
    assert(cstring != '\0');
    puts(buf);
    char diskdev_name_buf[512];
    bzero(&diskdev_name_buf, sizeof(diskdev_name_buf));
    snprintf(diskdev_name_buf, sizeof(diskdev_name_buf), "/dev/%s", buf);
    char *dev2 = strdup(diskdev_name_buf);
    fprintf(stderr, "calling mount(fstype=%s, mnt=%s, mntopts=%d, data=%s)\n", fstype, mnt, mntopts, dev2);
    int mount_ret = mount(fstype, mnt, mntopts, &dev2);
    if (mount_ret == 0)
    {
      not_mount_ret = 1;
    }
  }
  if ((not_mount_ret & 1) == 0)
  {
    fprintf(stderr, "successfully attached, but mounting failed (potentially due to entry not found): %d (%s)\n", errno, strerror(errno));
    return 1;
  }
  return 0;
}
