#include <jbloader.h>

void safemode_alert(CFNotificationCenterRef center, void *observer,
                    CFStringRef name, const void *object, CFDictionaryRef userInfo)
{
  int ret;
  CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(dict, kCFUserNotificationAlertHeaderKey, CFSTR("Entered Safe Mode"));
  if (checkrain_options_enabled(info.flags, checkrain_option_failure))
  {
    CFDictionarySetValue(dict, kCFUserNotificationAlertMessageKey, CFSTR("jbloader entered safe mode due to an error"));
  }
  else
  {
    CFDictionarySetValue(dict, kCFUserNotificationAlertMessageKey, CFSTR("jbloader entered safe mode due to an user request"));
  }
  CFUserNotificationCreate(kCFAllocatorDefault, 0, 0, &ret, dict);
  if (ret != 0)
  {
    fprintf(stderr, "CFUserNotificationCreate() returned %d %s\n", ret, mach_error_string(ret));
  }
  printf("Safe mode notification alert sent\n");
  set_safemode_spin(false);
  return;
}
