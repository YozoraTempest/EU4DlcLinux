#!/bin/sh
set -eu

fail() { printf '%s\n' "EU4DlcLinux: $*" >&2; exit 1; }
[ "$#" -gt 0 ] || fail 'Usage: sh /path/to/cream.sh command [arguments...]'
[ "$(uname -s)" = Linux ] || fail 'This launcher requires native Linux.'
[ "$(uname -m)" = x86_64 ] || fail 'This package requires x86_64.'

root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
config=${CREAM_CONFIG_PATH:-"$root/cream_api.ini"}
[ -f "$config" ] || fail "Configuration file not found: $config"
config=$(CDPATH= cd -- "$(dirname -- "$config")" && printf '%s/%s\n' "$(pwd -P)" "$(basename -- "$config")")
library="$root/lib64Creamlinux.so"
[ -f "$library" ] || fail "Library not found: $library. Run sh ./build.sh first."

if [ -n "${CREAM_STEAM_API:-}" ]; then
    steam_api=$CREAM_STEAM_API
elif [ -f "$root/libsteam_api.so" ]; then
    steam_api="$root/libsteam_api.so"
else
    candidates=$(find "$root" -type f -name libsteam_api.so -print)
    [ -n "$candidates" ] || fail "libsteam_api.so not found under: $root"
    [ "$(printf '%s\n' "$candidates" | wc -l)" -eq 1 ] || fail 'Multiple Steam libraries found; set CREAM_STEAM_API to the full file path.'
    steam_api=$candidates
fi
[ -f "$steam_api" ] || fail "Steam library not found: $steam_api"

check_elf64() {
    magic=$(od -An -tx1 -N5 "$1" | tr -d ' \n')
    machine=$(od -An -tx1 -j18 -N2 "$1" | tr -d ' \n')
    [ "$magic" = 7f454c4602 ] && [ "$machine" = 3e00 ] || fail "Expected an x86_64 ELF library: $1"
}
check_elf64 "$library"
check_elf64 "$steam_api"

# LD_PRELOAD splits paths at whitespace. A private path also isolates launches.
runtime=$(mktemp -d /tmp/eu4-dlc.XXXXXXXX)
cleanup() { rm -rf -- "$runtime"; }
trap cleanup 0
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM
cp -- "$library" "$runtime/lib64Creamlinux.so"
cp -- "$steam_api" "$runtime/libsteam_api.so"
export CREAM_CONFIG_PATH="$config"
preload="$runtime/lib64Creamlinux.so:$runtime/libsteam_api.so${LD_PRELOAD:+:$LD_PRELOAD}"
set +e
LD_PRELOAD="$preload" "$@"
status=$?
exit "$status"
