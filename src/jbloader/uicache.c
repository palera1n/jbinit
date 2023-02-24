#include <jbloader.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <stdbool.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct objc_object NSMutableDictionary;
typedef struct objc_object NSString;
typedef struct objc_object NSNumber;
typedef struct objc_object* ObjectType;
typedef struct objc_object* KeyType;

struct __NSConstantStringImpl {
  int *isa;
  int flags;
  char *str;
  long length;
};

extern int __CFConstantStringClassReference[];
static struct __NSConstantStringImpl springboard_plist __attribute__ ((section ("__DATA, __cfstring"))) = {__CFConstantStringClassReference,0x000007c8,"/var/mobile/Library/Preferences/com.apple.springboard.plist",(long)strlen("/var/mobile/Library/Preferences/com.apple.springboard.plist")};
static struct __NSConstantStringImpl SBShowNonDefaultSystemApps __attribute__ ((section ("__DATA, __cfstring"))) = {__CFConstantStringClassReference,0x000007c8,"SBShowNonDefaultSystemApps",(long)strlen("SBShowNonDefaultSystemApps")};

int uicache_apps()
{
  if (checkrain_option_enabled(pinfo.flags, palerain_option_rootful))
  {
    if (access("/usr/bin/uicache", F_OK) == 0)
    {
      {
        char *uicache_argv[] = {
            "/usr/bin/uicache",
            "-a",
            NULL};
        run_async(uicache_argv[0], uicache_argv);
        return 0;
      };
    }
    return 0;
  }
  else
  {
    if (access("/var/jb/usr/bin/uicache", F_OK) == 0)
    {
      {
        char *uicache_argv[] = {
            "/var/jb/usr/bin/uicache",
            "-a",
            NULL};
        run_async(uicache_argv[0], uicache_argv);
        return 0;
      };
    }
    return 0;
  }
}

void *prep_jb_ui(void *__unused _)
{
  uicache_apps();
  return NULL;
}

int enable_non_default_system_apps() {
  // NSMutableDictionary* md = [[NSMutableDictionary alloc] initWithContentsOfFile:@"/var/mobile/Library/Preferences/com.apple.springboard.plist"];
  NSMutableDictionary* md = ((NSMutableDictionary * _Nullable (*)(id, SEL, NSString * _Nonnull))(void *)objc_msgSend)((id)((NSMutableDictionary *(*)(id, SEL))(void *)objc_msgSend)((id)objc_getClass("NSMutableDictionary"), sel_registerName("alloc")),sel_registerName("initWithContentsOfFile:"),(NSString *)&springboard_plist);

  // if ([md objectForKey:@"SBShowNonDefaultSystemApps"] == nil)
  if (((id  _Nullable (*)(id, SEL, KeyType _Nonnull))(void *)objc_msgSend)((id)md, sel_registerName("objectForKey:"), (id _Nonnull)(NSString *)&SBShowNonDefaultSystemApps) == NULL)
  {
    // add SBShowNonDefaultSystemApps key
    // [md setObject:[NSNumber numberWithBool:YES] forKey:@"SBShowNonDefaultSystemApps"];
    ((void (*)(id, SEL, ObjectType _Nonnull, id))(void *)objc_msgSend)((id)md, sel_registerName("setObject:forKey:"), (id _Nonnull)((NSNumber * _Nonnull (*)(id, SEL, BOOL))(void *)objc_msgSend)((id)objc_getClass("NSNumber"), sel_registerName("numberWithBool:"), true), (id)(NSString *)&SBShowNonDefaultSystemApps);

    // [md writeToFile:@"/var/mobile/Library/Preferences/com.apple.springboard.plist" atomically:YES];
    ((BOOL (*)(id, SEL, NSString * _Nonnull, BOOL))(void *)objc_msgSend)((id)md, sel_registerName("writeToFile:atomically:"), (NSString *)&springboard_plist, true);

    chown("/var/mobile/Library/Preferences/com.apple.springboard.plist", 501, 501);
    printf("Enabled non-default system apps\n");
  } else {
    printf("Non-default system apps are already enabled\n");
  }
  return 0;
}

int uicache_loader()
{
  char *loader_uicache_argv[] = {
    "/cores/binpack/usr/bin/uicache",
    "-p",
    "/cores/binpack/Applications/palera1nLoader.app",
    NULL};
  run(loader_uicache_argv[0], loader_uicache_argv);
  return 0;
}
