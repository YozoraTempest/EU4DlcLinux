#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
test_root=$(mktemp -d "${TMPDIR:-/tmp}/eu4-preload-tests.XXXXXXXX")
trap 'rm -rf -- "$test_root"' 0
game="$test_root/game with spaces"
mkdir "$game"
cp -- "$1" "$game/cream.sh"
cp -- "$2" "$game/lib64Creamlinux.so"
cp -- "$3" "$game/libsteam_api.so"
cp -- "$root/package/cream_api.ini" "$game/cream_api.ini"
LD_PRELOAD="$4" sh "$game/cream.sh" "$5"
