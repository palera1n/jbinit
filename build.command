#!/bin/sh

# This file is for Xcode only! Do not run directly

set -e

UNAME_M="$(uname -m)"

export PATH="${HOME}/.local/bin:/opt/procursus/bin:/usr/local/bin:/opt/homebrew/bin:${PATH}"
echo "Invoked as: $0 $@"

if [ "$XCODE_VERSION_ACTUAL" = "" ]; then
	>&2 echo "$0 cannot be ran directly."
	exit 1;
fi

if [ "$PALERA1N" = "" ]; then
	PALERA1N="palera1n"
fi

PALERA1NS="palera1n palera1n-macos-universal palera1n-macos-x86_64"

if [ "$UNAME_M" = "arm64" ]; then
	PALERA1NS="${PALERA1NS} palera1n-macos-arm64"
fi

for p1 in ${PALERA1NS}; do
	if [ "$(command -v $p1)" != "" ]; then
		PALERA1N="$(command -v $p1)"
		break;
	fi
done
case "$ACTION" in
	clean)
		gmake -j$(sysctl -n hw.ncpu) clean
		;;
	run)
		"${PALERA1N}" -le serial=3 -r "${RAMDISK_DMG}" -o "${BINPACK_DMG}"
		;;
	*)
		gmake -j$(sysctl -n hw.ncpu)
		cp -a src/ramdisk.dmg src/binpack.dmg "$BUILT_PRODUCTS_DIR"
	;;
esac
