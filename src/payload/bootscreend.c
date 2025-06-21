#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOTypes.h>
#include <dlfcn.h>
#include <IOMobileFramebuffer/IOMobileframebuffer.h>
#include <IOSurface/IOSurface.h>
#include <IOSurface/IOSurfaceRef.h>
#include <sys/stat.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/param.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <sys/sysctl.h>
#include <libjailbreak/libjailbreak.h>

#define WHITE 0xffffffff
#define BLACK 0x00000000
static void *base = NULL;
IOSurfaceRef buffer;
static int bytesPerRow = 0;
static int height = 0;
static int width = 0;
//#define BSD_LOG_TO_FILE

static int bootscreend_draw_gradient(void);

#if !defined(TESTMAIN)
static int bsd_printf(const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int retval = vfprintf(stderr, fmt, va);
    va_end(va);
#if defined(BSD_LOG_TO_FILE)
    int fd = open("/cores/bootscreend.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd != -1) {
        va_start(va, fmt);
        vdprintf(fd, fmt, va);
        va_end(va);
        close(fd);
    }
#endif
    return retval;
}
#else
#define bsd_printf(...) printf(__VA_ARGS__);
#endif

static int init_display(void) {
    IOMobileFramebufferReturn retval = 0;
    if (base) return 0;
    IOMobileFramebufferRef display = NULL;
    retval = IOMobileFramebufferGetMainDisplay(&display);
    if (retval) {
        bsd_printf("IOMobileFramebufferGetMainDisplay: %s\n", mach_error_string(retval));
        bsd_printf("trying IOMobileFramebufferGetSecondaryDisplay instead\n");
        retval = IOMobileFramebufferGetSecondaryDisplay(&display);
        if (retval) {
            bsd_printf("IOMobileFramebufferGetSecondaryDisplay: %s\n", mach_error_string(retval));
            return -1;
        }
    }
    IOMobileFramebufferDisplaySize size;
    IOMobileFramebufferGetDisplaySize(display, &size);
    IOMobileFramebufferGetLayerDefaultSurface(display, 0, &buffer);
    bsd_printf("got display %p\n", display);
    width = size.width;
    height = size.height;
    bsd_printf("width: %d, height: %d\n", width, height);

    // create buffer
    CFMutableDictionaryRef properties = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    CFDictionarySetValue(properties, CFSTR("IOSurfaceIsGlobal"), kCFBooleanFalse);
    CFDictionarySetValue(properties, CFSTR("IOSurfaceWidth"), CFNumberCreate(NULL, kCFNumberIntType, &width));
    CFDictionarySetValue(properties, CFSTR("IOSurfaceHeight"), CFNumberCreate(NULL, kCFNumberIntType, &height));
    CFDictionarySetValue(properties, CFSTR("IOSurfacePixelFormat"), CFNumberCreate(NULL, kCFNumberIntType, &(int){ 0x42475241 }));
    CFDictionarySetValue(properties, CFSTR("IOSurfaceBytesPerElement"), CFNumberCreate(NULL, kCFNumberIntType, &(int){ 4 }));
    buffer = IOSurfaceCreate(properties);
    bsd_printf("created buffer at: %p\n", buffer);
    IOSurfaceLock(buffer, 0, 0);
    bsd_printf("locked buffer\n");
    base = IOSurfaceGetBaseAddress(buffer);
    bsd_printf("got base address at: %p\n", base);
    bytesPerRow = (int)IOSurfaceGetBytesPerRow(buffer);
    bsd_printf("got bytes per row: %d\n", bytesPerRow);
    IOSurfaceUnlock(buffer, 0, 0);
    bsd_printf("unlocked buffer\n");

    int token;
    IOMobileFramebufferSwapBegin(display, &token);
    IOMobileFramebufferSwapSetLayer(display, 0, buffer, (CGRect){ { 0, 0 }, { width, height } }, (CGRect){ { 0, 0 }, { width, height } }, 0);
    IOMobileFramebufferSwapEnd(display);
    return 0;
}

void check_for_exit(void)
{
    static int maxArgumentSize = 0;
    if (maxArgumentSize == 0) {
        size_t size = sizeof(maxArgumentSize);
        if (sysctl((int[]){ CTL_KERN, KERN_ARGMAX }, 2, &maxArgumentSize, &size, NULL, 0) == -1) {
            perror("sysctl argument size");
            maxArgumentSize = 4096; // Default
        }
    }
    int mib[3] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    struct kinfo_proc *info;
    size_t length;
    unsigned long count;
    
    if (sysctl(mib, 3, NULL, &length, NULL, 0) < 0)
        return;
    if (!(info = malloc(length)))
        return;
    if (sysctl(mib, 3, info, &length, NULL, 0) < 0) {
        free(info);
        return;
    }
    count = length / sizeof(struct kinfo_proc);
    for (unsigned long i = 0; i < count; i++) {
        pid_t pid = info[i].kp_proc.p_pid;
        if (pid == 0) {
            continue;
        }
        size_t size = maxArgumentSize;
        char* buffer = (char *)malloc(length);
        if (sysctl((int[]){ CTL_KERN, KERN_PROCARGS2, pid }, 3, buffer, &size, NULL, 0) == 0) {
            char *executablePath = buffer + sizeof(int);
            if (strcmp(executablePath, "/usr/libexec/backboardd") == 0 || strcmp(executablePath, "/usr/libexec/dfrd") == 0) exit(0);
        }
        free(buffer);
    }
    free(info);
}


int bootscreend_draw_cgimage(const char* image_path) {
    int retval = -1;
    CFURLRef imageURL = NULL;
    CGImageSourceRef cgImageSource = NULL;
    CGImageRef cgImage = NULL;
    CGContextRef context = NULL;
    CFStringRef bootImageCfString = NULL;
    CGColorSpaceRef rgbColorSpace = NULL;
    
    retval = init_display();
    if (retval) {
        bsd_printf("could not init display\n");
        goto finish;
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int offset = i * bytesPerRow + j * 4;
            *(int *)(base + offset) = 0x00000000;
        }
    }

    bootImageCfString = CFStringCreateWithCString(kCFAllocatorDefault, image_path, kCFStringEncodingUTF8);
    if (!bootImageCfString) {
        bsd_printf("could not create boot image cfstring\n");
        goto finish;
    }

    imageURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, bootImageCfString, kCFURLPOSIXPathStyle, false);
    if (!imageURL) {
        bsd_printf("could not create image URL\n");
        goto finish;
    }
    cgImageSource = CGImageSourceCreateWithURL(imageURL, NULL);
    if (!cgImageSource) {
        bsd_printf("could not create image source\n");
        goto finish;
    }
    cgImage = CGImageSourceCreateImageAtIndex(cgImageSource, 0, NULL);
    if (!cgImage) {
        bsd_printf("could not create image\n");
        bootscreend_draw_gradient();
        goto finish;
    }
    rgbColorSpace = CGColorSpaceCreateDeviceRGB();
    if (!rgbColorSpace) {
        bsd_printf("could not create device RGB color space\n");
        goto finish;
    }

    CGRect destinationRect = CGRectZero;
    CGFloat imageWidth = CGImageGetWidth(cgImage);
    CGFloat imageHeight = CGImageGetHeight(cgImage);
    
    CGFloat widthFactor = width / imageWidth;
    CGFloat heightFactor = height / imageHeight;
    CGFloat scaleFactor = widthFactor > heightFactor ? widthFactor : heightFactor;
    CGFloat scaledWidth  = imageWidth * scaleFactor;
    CGFloat scaledHeight = imageHeight * scaleFactor;

    destinationRect.size.width = scaledWidth;
    destinationRect.size.height = scaledHeight;
    
    if (widthFactor > heightFactor) {
        destinationRect.origin.y = (height - scaledHeight) / 2;
    } else {
        destinationRect.origin.x = (width - scaledWidth) / 2;
    }

    context = CGBitmapContextCreate(base, width, height, 8, bytesPerRow, rgbColorSpace, kCGImageAlphaPremultipliedFirst | kCGImageByteOrder32Little);
    if (!context) {
        bsd_printf("could not create context\n");
        goto finish;
    }

    CGContextDrawImage(context, destinationRect, cgImage);

    retval = 0;
    bsd_printf("bootscreend: done\n");

finish:
    if (bootImageCfString) CFRelease(bootImageCfString);
    if (context) CGContextRelease(context);
    if (cgImage) CGImageRelease(cgImage);
    if (cgImageSource) CFRelease(cgImageSource);
    if (imageURL) CFRelease(imageURL);
    if (rgbColorSpace) CGColorSpaceRelease(rgbColorSpace);

    return retval;
}

static int bootscreend_draw_gradient(void) {
    int retval = -1;
    retval = init_display();
    if (retval) {
        bsd_printf("could not init display\n");
        goto finish_;
    }
    
    const uint8_t start_a = 0x70;
    const uint8_t start_r = 0xb2;
    const uint8_t start_g = 0x86;
    const uint8_t start_b = 0x84;

    const uint8_t mid_a = 0xff;
    const uint8_t mid_r = 0xd4;
    const uint8_t mid_g = 0xd4;
    const uint8_t mid_b = 0xd1;

    const uint8_t end_a = 0xff;
    const uint8_t end_r = 0x87;
    const uint8_t end_g = 0xa8;
    const uint8_t end_b = 0xaf;
    
    const float step1_a = (float)(mid_a - start_a) / (float)(height / 2);
    const float step1_r = (float)(mid_r - start_r) / (float)(height / 2);
    const float step1_g = (float)(mid_g - start_g) / (float)(height / 2);
    const float step1_b = (float)(mid_b - start_b) / (float)(height / 2);
    
    const float step2_a = (float)(end_a - mid_a) / (float)(height / 2);
    const float step2_r = (float)(end_r - mid_r) / (float)(height / 2);
    const float step2_g = (float)(end_g - mid_g) / (float)(height / 2);
    const float step2_b = (float)(end_b - mid_b) / (float)(height / 2);
    
    for (int i = 0; i < (height / 2); i++) {
        uint32_t color = 0 |
            ((int)(start_a + i * step1_a) & 0xFF) << 24 |
            ((int)(start_r + i * step1_r) & 0xFF) << 16 |
            ((int)(start_g + i * step1_g) & 0xFF) << 8 |
            ((int)(start_b + i * step1_b) & 0xFF) << 0;
        for (int j = 0; j < width; j++) {
            int offset = i * bytesPerRow + j * 4;
            *(int *)(base + offset) ^= color;
        }
    }
    
    for (int i = (height / 2); i < height; i++) {
        uint32_t color = 0 |
            ((int)(mid_a + (i - (height / 2)) * step2_a) & 0xFF) << 24 |
            ((int)(mid_r + (i - (height / 2)) * step2_r) & 0xFF) << 16 |
            ((int)(mid_g + (i - (height / 2)) * step2_g) & 0xFF) << 8 |
            ((int)(mid_b + (i - (height / 2)) * step2_b) & 0xFF) << 0;
        for (int j = 0; j < width; j++) {
            int offset = i * bytesPerRow + j * 4;
            *(int *)(base + offset) ^= color;
        }
    }
    
    retval = 0;
    
finish_:
    return retval;
}

static int bootscreend_draw_image(const char* image_path) {
    return bootscreend_draw_cgimage(image_path);
}

#if defined(TESTMAIN)
int main(int argc, char* argv[]) {
    int retval = -1;
    printf("current platform: %d\n", jailbreak_get_platform());
    if (argc < 2) {
        retval = bootscreend_draw_gradient();
    } else {
        retval = bootscreend_draw_image(argv[1]);
    }
    sleep(2);
    return retval;
}
#else
#define BOOT_IMAGE_PATH "/cores/binpack/usr/share/boot.jp2"
int bootscreend_main(void) {
    int retval = 0;
    if (jailbreak_get_platform() != PLATFORM_BRIDGEOS) {
        retval = bootscreend_draw_image(BOOT_IMAGE_PATH);
    } else {
        retval = bootscreend_draw_gradient();
    }
#if !defined(TESTMAIN)
    if (retval) return retval;
    while (true) {
        check_for_exit();
        sleep(5);
    }
#endif
}
#endif
