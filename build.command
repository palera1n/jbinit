#!/bin/sh

# This file is for Xcode only! Do not run directly

export PATH="/opt/procursus/bin:/usr/local/bin:/opt/homebrew/bin:${PATH}"

gmake -j$(sysctl -n hw.ncpu)

if [ "$TARGET_BUILD_DIR" != "" ]; then
	cp -a src/ramdisk.dmg src/binpack.dmg "$TARGET_BUILD_DIR"
fi
