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

compiling on ios/ipados (experimental):

1. must be jailbroken and strapped with Procursus (or palera1n strap which is effectively Procursus)
2. `sudo apt install build-essential git make unzip ldid odcctools llvm bash sed`
3. place [palera1nLoader.ipa](https://static.palera.in/artifacts/loader/universal_lite/palera1nLoader.ipa) [binpack.tar](https://static.palera.in/binpack.tar) into src directory
    place [libellekit.dylib](https://static.palera.in/development/libellekit.dylib) into src/ellekit directory
4. prepare iPhoneOS.sdk and MacOSX.sdk of the same darwin version,
    export path to iPhoneOS.sdk in TARGET_SYSROOT environmental variable,
    and path to MacOSX.sdk in MACOSX_SYSROOT environmental variable
5. `make -j$(sysctl -n hw.ncpu)`

after compiling:
- check the output files, src/ramdisk.dmg and src/binpack.dmg
- you can use the files in palera1n like this: `palera1n -r /path/to/ramdisk.dmg -o /path/to/binpack.dmg`
