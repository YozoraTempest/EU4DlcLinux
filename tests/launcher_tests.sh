#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
test_root=$(mktemp -d "${TMPDIR:-/tmp}/eu4-launcher-tests.XXXXXXXX")
trap 'rm -rf -- "$test_root"' 0
game="$test_root/game with spaces"
mkdir -p "$game" "$test_root/bin"
cp "$root/package/cream.sh" "$game/cream.sh"
cp "$root/package/cream_api.ini" "$game/cream_api.ini"

# These fixtures exercise shell behavior, without loading Steam or an ELF library.
printf '\177ELF\002\001\001\000\000\000\000\000\000\000\000\000\003\000\076\000' > "$game/lib64Creamlinux.so"
cp "$game/lib64Creamlinux.so" "$game/libsteam_api.so"
cat > "$test_root/bin/uname" <<'SH'
#!/bin/sh
case "$1" in
    -s) printf '%s\n' "${TEST_UNAME_OS:-Linux}" ;;
    -m) printf '%s\n' x86_64 ;;
esac
SH
cat > "$test_root/probe" <<'SH'
#!/bin/sh
preload=$LD_PRELOAD
unset LD_PRELOAD
hook=${preload%%:*}
remaining=${preload#*:}
steam=${remaining%%:*}
[ -f "$hook" ] && [ -f "$steam" ] || return 90
printf '%s\n' "$hook" "$steam" "$CREAM_CONFIG_PATH" "$#" "$1" "$2" > "$PROBE_OUTPUT"
return 37
SH
chmod +x "$test_root/bin/uname" "$test_root/probe"
export PATH="$test_root/bin:$PATH"
export PROBE_OUTPUT="$test_root/probe-output"
unset CREAM_CONFIG_PATH CREAM_STEAM_API

set +e
sh "$game/cream.sh" . "$test_root/probe" 'argument with spaces' 'literal;command' 2> "$test_root/stderr"
status=$?
set -e
[ "$status" -eq 37 ] || { cat "$test_root/stderr" >&2; exit 1; }
{
    IFS= read -r hook
    IFS= read -r steam
    IFS= read -r config
    IFS= read -r count
    IFS= read -r first
    IFS= read -r second
} < "$PROBE_OUTPUT"
[ ! -e "$hook" ] && [ ! -e "$steam" ]
[ "$config" = "$game/cream_api.ini" ]
[ "$count" -eq 2 ] && [ "$first" = 'argument with spaces' ] && [ "$second" = 'literal;command' ]

expect_failure() {
    if "$@" > "$test_root/stdout" 2> "$test_root/stderr"; then
        printf '%s\n' "Expected failure: $*" >&2
        exit 1
    fi
}
expect_failure sh "$game/cream.sh"
expect_failure env TEST_UNAME_OS=Windows_NT sh "$game/cream.sh" true
expect_failure env CREAM_CONFIG_PATH="$test_root/missing.ini" sh "$game/cream.sh" true
expect_failure env CREAM_CONFIG_PATH="$game" sh "$game/cream.sh" true
expect_failure env CREAM_STEAM_API="$test_root/missing.so" sh "$game/cream.sh" true
mv "$game/libsteam_api.so" "$game/original.so"
mkdir "$game/one" "$game/two"
cp "$game/original.so" "$game/one/libsteam_api.so"
cp "$game/original.so" "$game/two/libsteam_api.so"
expect_failure sh "$game/cream.sh" true
set +e
CREAM_STEAM_API="$game/one/libsteam_api.so" sh "$game/cream.sh" . "$test_root/probe" one two 2> "$test_root/stderr"
status=$?
set -e
[ "$status" -eq 37 ]
printf 'MZ' > "$game/lib64Creamlinux.so"
expect_failure env CREAM_STEAM_API="$game/one/libsteam_api.so" sh "$game/cream.sh" true
printf '%s\n' 'Launcher path, argument, exit status, cleanup, platform and library selection tests passed.'
