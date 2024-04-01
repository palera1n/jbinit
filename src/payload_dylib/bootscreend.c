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

#define WHITE 0xffffffff
#define BLACK 0x00000000
static void *base = NULL;
static int bytesPerRow = 0;
static int height = 0;
static int width = 0;

static int init_display(void) {
    IOMobileFramebufferReturn retval = 0;
    if (base) return 0;
    IOMobileFramebufferRef display = NULL;
    retval = IOMobileFramebufferGetMainDisplay(&display);
    if (retval) {
        fprintf(stderr, "IOMobileFramebufferGetMainDisplay: %s\n", mach_error_string(retval));
        printf("trying IOMobileFramebufferGetSecondaryDisplay instead\n");
        retval = IOMobileFramebufferGetSecondaryDisplay(&display);
        if (retval) {
            fprintf(stderr, "IOMobileFramebufferGetSecondaryDisplay: %s\n", mach_error_string(retval));
            return -1;
        }
    }
    IOMobileFramebufferDisplaySize size;
    IOMobileFramebufferGetDisplaySize(display, &size);
    IOSurfaceRef buffer;
    IOMobileFramebufferGetLayerDefaultSurface(display, 0, &buffer);
    printf("got display %p\n", display);
    width = size.width;
    height = size.height;
    printf("width: %d, height: %d\n", width, height);

    // create buffer
    CFMutableDictionaryRef properties = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    CFDictionarySetValue(properties, CFSTR("IOSurfaceIsGlobal"), kCFBooleanFalse);
    CFDictionarySetValue(properties, CFSTR("IOSurfaceWidth"), CFNumberCreate(NULL, kCFNumberIntType, &width));
    CFDictionarySetValue(properties, CFSTR("IOSurfaceHeight"), CFNumberCreate(NULL, kCFNumberIntType, &height));
    CFDictionarySetValue(properties, CFSTR("IOSurfacePixelFormat"), CFNumberCreate(NULL, kCFNumberIntType, &(int){ 0x42475241 }));
    CFDictionarySetValue(properties, CFSTR("IOSurfaceBytesPerElement"), CFNumberCreate(NULL, kCFNumberIntType, &(int){ 4 }));
    buffer = IOSurfaceCreate(properties);
    printf("created buffer at: %p\n", buffer);
    IOSurfaceLock(buffer, 0, 0);
    printf("locked buffer\n");
    base = IOSurfaceGetBaseAddress(buffer);
    printf("got base address at: %p\n", base);
    bytesPerRow = IOSurfaceGetBytesPerRow(buffer);
    printf("got bytes per row: %d\n", bytesPerRow);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int offset = i * bytesPerRow + j * 4;
            *(int *)(base + offset) = 0x00000000;
        }
    }
    printf("wrote to buffer\n");
    IOSurfaceUnlock(buffer, 0, 0);
    printf("unlocked buffer\n");

    int token;
    IOMobileFramebufferSwapBegin(display, &token);
    IOMobileFramebufferSwapSetLayer(display, 0, buffer, (CGRect){ { 0, 0 }, { width, height } }, (CGRect){ { 0, 0 }, { width, height } }, 0);
    IOMobileFramebufferSwapEnd(display);
    return 0;
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
        fprintf(stderr, "could not init display\n");
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
        fprintf(stderr, "could not create boot image cfstring\n");
        goto finish;
    }

    imageURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, bootImageCfString, kCFURLPOSIXPathStyle, false);
    if (!imageURL) {
        fprintf(stderr, "could not create image URL\n");
        goto finish;
    }
    cgImageSource = CGImageSourceCreateWithURL(imageURL, NULL);
    if (!cgImageSource) {
        fprintf(stderr, "could not create image source\n");
        goto finish;
    }
    cgImage = CGImageSourceCreateImageAtIndex(cgImageSource, 0, NULL);
    if (!cgImage) {
        fprintf(stderr, "could not create image\n");
        goto finish;
    }
    rgbColorSpace = CGColorSpaceCreateDeviceRGB();
    if (!rgbColorSpace) {
        fprintf(stderr, "could not create device RGB color space\n");
        goto finish;
    }

    CGRect destinationRect = CGRectZero;
    CGFloat imageAspectRatio = (CGFloat)CGImageGetWidth(cgImage) / CGImageGetHeight(cgImage);

    if (width / height > imageAspectRatio) {
        destinationRect.size.width = width;
        destinationRect.size.height = width / imageAspectRatio;
    } else {
        destinationRect.size.width = height * imageAspectRatio;
        destinationRect.size.height = height;
    }
    
    destinationRect.origin.x = (width - CGRectGetWidth(destinationRect)) / 2;
    destinationRect.origin.y = (height - CGRectGetHeight(destinationRect)) / 2;

    context = CGBitmapContextCreate(base, width, height, 8, bytesPerRow, rgbColorSpace, kCGImageAlphaPremultipliedFirst | kCGImageByteOrder32Little);
    if (!context) {
        fprintf(stderr, "could not create context\n");
        goto finish;
    }

    CGContextDrawImage(context, destinationRect, cgImage);

    retval = 0;
    fprintf(stderr, "bootscreend: done\n");

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
        fprintf(stderr, "could not init display\n");
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
uint32_t dyld_get_active_platform(void);

static int bootscreend_draw_image(const char* image_path) {
    return bootscreend_draw_cgimage(image_path);
}

#if defined(TESTMAIN)
int main(int argc, char* argv[]) {
    int retval = -1;
    if (argc < 2) {
        retval = bootscreend_draw_gradient();
    } else {
        retval = bootscreend_draw_image(argv[1]);
    }
    sleep(2);
    return retval;
}
#elif defined(TESTBRIDGE_MAIN)
#else
#define BOOT_IMAGE_PATH "/cores/binpack/usr/share/boot.jp2"
int bootscreend_main(void) {
    if (dyld_get_active_platform() != PLATFORM_BRIDGEOS) {
        return bootscreend_draw_image(BOOT_IMAGE_PATH);
    } else {
        return bootscreend_draw_gradient();
    }
}
#endif
