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
5. you will probably be required to enter macOS password via the GUI and CLI, if you want to do it headless run the makefile as root.
6. check the output files, src/ramdisk.dmg and src/binpack.dmg
7. you can use the files in palera1n like this: `palera1n -r /path/to/ramdisk.dmg -o /path/to/binpack.dmg`
