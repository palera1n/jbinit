#include <jbloader.h>

int get_boot_manifest_hash(char hash[97])
{
  const UInt8 *bytes;
  CFIndex length;
  io_registry_entry_t chosen = IORegistryEntryFromPath(0, "IODeviceTree:/chosen");
  assert(chosen);
  CFDataRef manifestHash = (CFDataRef)IORegistryEntryCreateCFProperty(chosen, CFSTR("boot-manifest-hash"), kCFAllocatorDefault, 0);
  if (manifestHash == NULL || CFGetTypeID(manifestHash) != CFDataGetTypeID())
  {
    return 1;
  }
  length = CFDataGetLength(manifestHash);
  bytes = CFDataGetBytePtr(manifestHash);
  CFRelease(manifestHash);
  for (int i = 0; i < length; i++)
  {
    snprintf(&hash[i * 2], 3, "%02X", bytes[i]);
  }
  return 0;
}
