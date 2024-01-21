#!/bin/sh

# This file is for Xcode only! Do not run directly

export PATH="/opt/procursus/bin:/usr/local/bin:/opt/homebrew/bin:${PATH}"
echo "Invoked as: $0 $@"

if [ "$XCODE_VERSION_ACTUAL" = "" ]; then
	>&2 echo "$0 cannot be ran directly."
	exit 1;
fi

if [ "$ACTION" = "clean" ]; then
	gmake -j$(sysctl -n hw.ncpu) clean
else
	gmake -j$(sysctl -n hw.ncpu)
	cp -a src/ramdisk.dmg src/binpack.dmg "$TARGET_BUILD_DIR"

fi
