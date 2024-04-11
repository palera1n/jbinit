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
#include <time.h>

#define WHITE 0xffffffff
#define BLACK 0x00000000
static void *base = NULL;
static int bytesPerRow = 0;
static int height = 0;
static int width = 0;
//#define BSD_LOG_TO_FILE

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
#define bsd_printf(__VA_ARGS__) printf(__VA_ARGS__);
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
    IOSurfaceRef buffer;
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
    bytesPerRow = IOSurfaceGetBytesPerRow(buffer);
    bsd_printf("got bytes per row: %d\n", bytesPerRow);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int offset = i * bytesPerRow + j * 4;
            *(int *)(base + offset) = 0x00000000;
        }
    }
    bsd_printf("wrote to buffer\n");
    IOSurfaceUnlock(buffer, 0, 0);
    bsd_printf("unlocked buffer\n");

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

uint32_t current_alt_color(uint32_t current, uint32_t total) {
    uint32_t retval = 0xff000000;
    const uint8_t color1_r = 0xf5, color1_g = 0xa9, color1_b = 0xb8;
    const uint8_t color2_r = 0x5b, color2_g = 0xce, color2_b = 0xfa;
    const uint8_t color3_r = 0xff, color3_g = 0xff, color3_b = 0xff;
    const uint8_t color4_r = 0x5b, color4_g = 0xce, color4_b = 0xfa;
    const uint8_t color5_r = 0xf5, color5_g = 0xa9, color5_b = 0xb8;

    const float step_1_2_r = (float)(color2_r - color1_r) / ((float)total / 4);
    const float step_1_2_g = (float)(color2_g - color1_g) / ((float)total / 4);
    const float step_1_2_b = (float)(color2_b - color1_b) / ((float)total / 4);

    const float step_2_3_r = (float)(color3_r - color2_r) / ((float)total / 4);
    const float step_2_3_g = (float)(color3_g - color2_g) / ((float)total / 4);
    const float step_2_3_b = (float)(color3_b - color2_b) / ((float)total / 4);

    const float step_3_4_r = (float)(color4_r - color3_r) / ((float)total / 4);
    const float step_3_4_g = (float)(color4_g - color3_g) / ((float)total / 4);
    const float step_3_4_b = (float)(color4_b - color3_b) / ((float)total / 4);

    const float step_4_5_r = (float)(color5_r - color4_r) / ((float)total / 4);
    const float step_4_5_g = (float)(color5_g - color4_g) / ((float)total / 4);
    const float step_4_5_b = (float)(color5_b - color4_b) / ((float)total / 4);

    if (current < (total/4)) {
        return retval |
        ((uint32_t)(color1_r + current * step_1_2_r) & 0xff) << 16 |
        ((uint32_t)(color1_g + current * step_1_2_g) & 0xff) << 8 |
        ((uint32_t)(color1_b + current * step_1_2_b) & 0xff) << 0;
    } else if (current < (total/2)) {
        return retval |
        ((uint32_t)(color2_r + (current - ((float)total/4)) * step_2_3_r) & 0xff) << 16 |
        ((uint32_t)(color2_g + (current - ((float)total/4)) * step_2_3_g) & 0xff) << 8 |
        ((uint32_t)(color2_b + (current - ((float)total/4)) * step_2_3_b) & 0xff) << 0;
    } else if (current < ((total/4) * 3)) {
        return retval |
        ((uint32_t)(color3_r + (current - ((float)total/2)) * step_3_4_r) & 0xff) << 16 |
        ((uint32_t)(color3_g + (current - ((float)total/2)) * step_3_4_g) & 0xff) << 8 |
        ((uint32_t)(color3_b + (current - ((float)total/2)) * step_3_4_b) & 0xff) << 0;
    } else {
        return retval |
        ((uint32_t)(color4_r + (current - (((float)total/4) * 3)) * step_4_5_r) & 0xff) << 16 |
        ((uint32_t)(color4_g + (current - (((float)total/4) * 3)) * step_4_5_g) & 0xff) << 8 |
        ((uint32_t)(color4_b + (current - (((float)total/4) * 3)) * step_4_5_b) & 0xff) << 0;
    }

}

static int bootscreend_draw_alt_gradient(void) {
    int retval = -1;
    retval = init_display();
    if (retval) {
        bsd_printf("could not init display\n");
        goto finish__;
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int offset = i * bytesPerRow + j * 4;
            *(int *)(base + offset) ^= current_alt_color(i, height);
        }
    }
finish__:
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
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    if (tm && (tm->tm_mon == 3 && tm->tm_mday == 1)) {
        return bootscreend_draw_alt_gradient();
    }
    if (dyld_get_active_platform() != PLATFORM_BRIDGEOS) {
        return bootscreend_draw_image(BOOT_IMAGE_PATH);
    } else {
        return bootscreend_draw_gradient();
    }
}
#endif
