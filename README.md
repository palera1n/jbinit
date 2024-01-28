# plooshInit

how to compile on macos:

1. make sure xcode is installed
2. install gnu sed, procursus ldid and gnu make
    (`brew install gnu-sed make ldid-procursus`)
3. get the two dependency files, they are the palera1n loader
    ipa and the procursus binpack.
    get [palera1nLoader.ipa](https://static.palera.in/artifacts/loader/universal_lite/palera1nLoader.ipa) and place it into the src directory.
    get [binpack.tar](https://static.palera.in/binpack.tar) and also place it in the src directory
4. `gmake -j$(sysctl -n hw.ncpu)`

compiling on ios/ipados:

1. must be jailbroken and strapped with Procursus (or palera1n strap which is effectively Procursus)
2. `sudo apt install build-essential git make unzip ldid odcctools llvm bash sed`
3. place [palera1nLoader.ipa](https://static.palera.in/artifacts/loader/universal_lite/palera1nLoader.ipa) [binpack.tar](https://static.palera.in/binpack.tar) into src directory
    place [libellekit.dylib](https://static.palera.in/development/libellekit.dylib) into src/ellekit directory
4. prepare iPhoneOS.sdk and MacOSX.sdk of the same darwin version,
    export path to iPhoneOS.sdk in TARGET_SYSROOT environmental variable,
    and path to MacOSX.sdk in MACOSX_SYSROOT environmental variable
5. `make -j$(sysctl -n hw.ncpu)`

compiling on linux/other unix:

more or less the same as compiling on ios, but notably you need to make sure
your toolchain includes the compiler-rt builtins for ios, otherwise you will
get an error related to ___isPlatformVersionAtLeast.

you can get the builtins from xcode and copy them onto your toolchains's
compiler-rt builtin path

compiler-rt builtin path in Xcode.app:
`Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/${APPLE_CLANG_VERSION}/lib`

usual distro compiler-rt builtin path
`/usr/lib/clang/${LLVM_MAJOR_V}/lib` or `/usr/lib/llvm-${LLVM_MAJOR_V}/lib/clang/${LLVM_VERSION}`

useful links:
- [cctools-port](https://github.com/tpoechtrager/cctools-port) - Apple cctools port for Linux/*BSD
- [unxip](https://github.com/saagarjha/unxip) - Fast Xcode unarchiver in swift
- [Xcode releases](https://xcodereleases.com/) - Xcode download site
- [Darling unxip](https://github.com/darlinghq/darling/tree/master/src/unxip) - Another Xcode unarchiver if you can't build swift for some reason

after compiling:
- check the output files, src/ramdisk.dmg and src/binpack.dmg
- you can use the files in palera1n like this: `palera1n -r /path/to/ramdisk.dmg -o /path/to/binpack.dmg`
