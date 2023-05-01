#include <jbloader.h>

int get_boot_manifest_hash(char hash[97])
{
  const UInt8 *bytes;
  CFIndex length;
  io_registry_entry_t chosen = IORegistryEntryFromPath(0, "IODeviceTree:/chosen");
  if (!MACH_PORT_VALID(chosen)) return 1;
  CFDataRef manifestHash = (CFDataRef)IORegistryEntryCreateCFProperty(chosen, CFSTR("boot-manifest-hash"), kCFAllocatorDefault, 0);
  IOObjectRelease(chosen);
  if (manifestHash == NULL || CFGetTypeID(manifestHash) != CFDataGetTypeID())
  {
    if (manifestHash != NULL) CFRelease(manifestHash);
    return 1;
  }
  length = CFDataGetLength(manifestHash);
  bytes = CFDataGetBytePtr(manifestHash);
  for (int i = 0; i < length; i++)
  {
    snprintf(&hash[i * 2], 3, "%02X", bytes[i]);
  }
  CFRelease(manifestHash);
  return 0;
}

int print_boot_manifest_hash_main(int argc, char* argv[]) {
  char hash[97];
  int ret = get_boot_manifest_hash(hash);
  if (ret != 0) {
    fprintf(stderr, "could not get boot manifest hash\n");
    return ret;
  }
  printf("%s\n", hash);
  return 0;
}
