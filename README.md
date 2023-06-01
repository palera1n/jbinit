# jbinit

This project is used to build the `ramdisk.dmg` used in [palera1n](https://github.com/palera1n/palera1n) for the first user-mode post-exploitation steps. This will patch `launchd` and allow us to load additional services at system startup.

## Files not included, but needed

- `palera1n.ipa`: palera1n loader. [Click here to read more](https://github.com/palera1n/loader).
- `binpack.tar`: Procursus binpack. [Click here to read more](https://github.com/ProcursusTeam/Procursus).

## Additional notes

`ASAN` builds will exceed launchd memory limits. Do not use under non-development situations.

## How to build

```shell
git clone git@github.com:palera1n/jbinit.git
cd jbinit
./build.sh
```
