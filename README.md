# jbinit
based off jbinit by tihmstar, to compile you would need Procursus [binpack](https://github.com/ProcursusTeam/binpack),
compiled with `MEMO_CFVER=1800 MEMO_PREFIX=/usr` and have a dropbear.plist in binpack/Library/LaunchDaemons for the ssh server, then,
you should place the untar'd binpack into the current directory at binpack, binpack.dmg will be created from it.

used in palera1n-c.

thanks to [Serena](https://github.com/SerenaKit) for helping me

`ASAN` builds will exceed launchd memory limits. Do not use under non-development situations.
