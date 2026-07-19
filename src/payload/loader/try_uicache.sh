#!/cores/binpack/bin/sh

UICACHE="/cores/binpack/usr/bin/uicache";
GREP="/cores/binpack/usr/bin/grep";

if [ "$1" = "" ]; then
    exit 1;
fi

while ! "$UICACHE" -l | "$GREP" -qF " : $1"; do
    "$UICACHE" -p "$1";
done

exit 0
