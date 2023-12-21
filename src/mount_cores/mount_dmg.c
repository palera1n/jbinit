#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <mach/mach.h>
#include <mach/error.h>

#define HDI_MAGIC 0x1beeffeed
struct HDIImageCreateBlock64
{
  uint64_t magic;
  const void *props;
  uint64_t props_size;
  char padding[0x100 - 24];
};
#ifdef MOUNT_CORES
int attach_dmg(const char *source, char* device_path, size_t device_path_len)
#else
int attach_dmg(const char *source, bool is_overlay, char* device_path, size_t device_path_len)
#endif
{
  CFMutableDictionaryRef hdix = IOServiceMatching("IOHDIXController");
  io_service_t hdix_service = IOServiceGetMatchingService(kIOMasterPortDefault, hdix);
  io_connect_t connect;
  assert(hdix_service != 0);
  kern_return_t open_hdix = IOServiceOpen(hdix_service, mach_task_self(), 0, &connect);
  assert(open_hdix == KERN_SUCCESS);
  fprintf(stderr, "IOServiceOpen: %d\n", open_hdix);
  CFMutableDictionaryRef props = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFUUIDRef uuid = CFUUIDCreate(kCFAllocatorDefault);
  CFStringRef uuid_string = CFUUIDCreateString(0, uuid);
  size_t source_len = strlen(source);
  CFDataRef path_bytes = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (unsigned char *)source, source_len, kCFAllocatorNull);
  assert(path_bytes != 0);
  CFDictionarySetValue(props, CFSTR("hdik-unique-identifier"), uuid_string);
  CFDictionarySetValue(props, CFSTR("image-path"), path_bytes);
  CFDictionarySetValue(props, CFSTR("autodiskmount"), kCFBooleanFalse);
  CFDictionarySetValue(props, CFSTR("removable"), kCFBooleanTrue);
#ifndef MOUNT_CORES
  CFMutableDictionaryRef image_secrets = NULL;
  if (is_overlay)
  {
    image_secrets = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(image_secrets, CFSTR("checkra1n-overlay"), kCFBooleanTrue);
    CFDictionarySetValue(props, CFSTR("image-secrets"), image_secrets);
  }
#endif
  CFDataRef hdi_props = CFPropertyListCreateData(kCFAllocatorDefault, props, kCFPropertyListXMLFormat_v1_0, 0, 0);
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
  bool found_device = false;
  io_object_t next = IOIteratorNext(iter);
  while ((next = IOIteratorNext(iter)) != 0)
  {
    CFStringRef bsd_name = (CFStringRef)IORegistryEntryCreateCFProperty(next & 0xffffffff, CFSTR("BSD Name"), 0, 0);
    char buf[1024];
    if (bsd_name == 0)
      continue;
    char cstring = CFStringGetCString(bsd_name, buf, sizeof(buf), kCFStringEncodingUTF8);
    assert(cstring != '\0');
    snprintf(device_path, device_path_len, "/dev/%s", buf);
    puts(device_path);
    CFRelease(bsd_name);
    found_device = true;
    break;
  }
#ifndef MOUNT_CORES
  if (image_secrets) CFRelease(image_secrets);
#endif
  CFRelease(uuid_string);
  CFRelease(uuid);
  CFRelease(path_bytes);
  CFRelease(hdi_props);
  IOObjectRelease(hdix_service);
  IOObjectRelease(iter);
  if (!found_device) {
    fprintf(stderr, "cannot find device path in IO Registry\n");
    return 1;
  }
  return 0;
}
