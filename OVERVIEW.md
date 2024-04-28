# plooshInit overview

plooshInit may be booted in one of the follow modes: **ramdisk root** or **md0 on /cores**.
palera1n uses md0 on cores.

Order of execution: fakedyld -> payload_dylib constructor -> /cores/payload prelaunchd stage
-> payload_dylib constructor -> /sbin/launchd -> payload_dylib boot-task hook ->
/cores/payload sysstatuscheck stage -> /usr/libexec/sysstatuscheck -> /sbin/launchd
-> payload_dylib daemon hook -> jaibreakd, /cores/payload launch daemons stage ->
uicache palera1n loader, spawn dropbear -> SpringBoard/PineBoard

## fakedyld

When kernel loads the init program, it will invoke dyld, this will be `/cores/usr/lib/dyld`,
which is the fakedyld, via a kernel patch. On ramdisk root the fakedyld is also accessible
at `/usr/lib/dyld`, the normal dyld location at this stage. Should fakedyld encounter an
error, it will crash with a custom reason string.

fakedyld will carry out the following tasks:

1. open `/dev/console` for logging
2. Parse argc, argv, envp, apple, from `KernelArgs`
3. Load and check boot-args and paleinfo appended to end of `/dev/md0`
4. Check for boot mode: ramdisk root or md0 on /cores
5. (ramdisk root) read ramdisk files into memory as those will not be accessible after 
mounting root filesystem. 
6. (ramdisk root) clean fakefs if requested
7. (ramdisk root) mount root filesystem, using information supplied in paleinfo.
8. Prepare the root filesystem, such as binding filesystems on a partial fakefs,
or mounting devfs in case of ramdisk root.
9. Make /cores writable, on ramdisk root, mount tmpfs onto /cores, on md0 on /cores
remount /cores.
10. (ramdisk root) write the read files into `/cores`
11. (md0 on /cores) Delete the fakedyld file so the kernel will use the real dyld.
12. Patch dyld so that we can use the same set of bianries for iOS, iPadOS, tvOS,
HomePod software and bridgeOS. The patched dyld goes to `/cores/usr/lib/dyld`
13. Execute the real launchd. Environment variables is used to make
the next stage, `payload_dylib` run before any launchd code can execute.

## payload_dylib

This stage is a dylib inserted into `/sbin/launchd` with `DYLD_INSERT_LIBRARIES`.
It is used to persist the jailbreak's code execution during boot or userspace reboots.
payload_dylib provides the following services:

When first loaded, it performs the following task:
1. open `/dev/console` for logging
2. Load launchd crash reporter, palera1n flags
3. Spawns `/cores/payload` to carry out extra things which may require entitlements
4. Stop and reboot here if we are trying to setup fakefs
5. Draw something cool onto the device framebuffer, if one exists.
6. Use `ellekit` or `Dobby` to hook functions in launchd to provide services.

These hooks are installed:
- A xpc handler running in launchd to set certain environment variables system-wide
- Loads additional LaunchDaemons for jailbreak.
- posix_spawn hooks to inject `systemhook.dylib` system-wide

## payload

This file is part of `payload`, a multi-call binary.
This section only concerns when its name is `payload`.

### prelaunchd stage

It performs the following tasks:

1. Mount varies disk images such as the binpack and loader.
2. Delete the fakefs if we are force reverting on rootful
3. Use a bind mount to use `Dobby` over `ellekit` when `ellekit` cannot be used.
4. Setup fakefs if requested

### sysstatuscheck stage

At this point, all file systems has been mounted.
The following tasks are performed:

1. Remount filesystems
2. Generate the SSH host keys for dropbear
3. Write to preferences plists for palera1n loader
4. Delete files if we are trying to force revert
5. Revert the snapshot if we are trying to force revert rootful realfs.
6. Remove `/var/jb` symlink on rootful, or make it again on rootless if preboot path
exists.
7. Execute most (with some exceptions) files in the `/etc/rc.d` or `/var/jb/etc/rc.d`
directory.

### launchdaemons stage

XPC services may be used at this stage.
The following tasks are performed:

1. uicache the palera1n loader and jailbreak apps
2. show the safe mode alert if we are in safe mode

## jailbreakd

This file is part of `payload`, a multi-call binary.
This section only concerns when its name is `jailbreakd`.

jailbreakd has the following uses:

1. Act as the root helper for `p1ctl` and palera1n loader
2. Handles userspace reboot requests because stock userspace
reboots breaks Xcode debugging.
3. Holds various information in memory such that they stay consistent
system-wide.

## p1ctl

This file is part of `payload`, a multi-call binary.
This section only concerns when its name is `p1ctl`.

p1ctl is the command line access to palera1n loader, see `p1ctl(8)` for details.

## systemhook

systemhook.dylib is a dylib which is inserted into most processes.
It is responsible for:

- Loading tweaks
- Re-inject itself when the process it is injected into spawns another process.
- Make sure `dlopen("@rpath/usr/lib/libroot.dylib")` works correctly
- Loading `universalhooks`
- Redirect userspace reboot requests to `jailbreakd`
- Fix up `__builtin_available`, `@available` etc for binaries made to run with the dyld patch.

## universalhooks

universalhooks is injected into various daemons. It is used to alter the behaviour of daemons in
a way that is suitable for the jailbreak.

Examples:
- rootless path redirection
- Force enable apps on tvOS
- watchdogd hook to prevent watchdog timeout from panicking the system (perform userspace reboot instead)
