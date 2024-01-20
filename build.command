#!/bin/sh

export PATH="/opt/procursus/bin:/usr/local/bin:/opt/homebrew/bin:${PATH}"

exec gmake -j$(sysctl -n hw.ncpu) $@
