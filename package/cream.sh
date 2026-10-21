#!/bin/sh
set -eu

fail() { printf '%s\n' "EU4DlcLinux: $*" >&2; exit 1; }
[ "$#" -gt 0 ] || fail '请在 Steam 启动选项中填写：sh ./cream.sh %command%'
[ "$(uname -s)" = Linux ] || fail '请使用原生 Linux 版 EU4。'
[ "$(uname -m)" = x86_64 ] || fail '此补丁仅支持 x86_64（64 位）Linux。'

root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
config=${CREAM_CONFIG_PATH:-"$root/cream_api.ini"}
[ -f "$config" ] || fail "缺少配置文件，请重新解压补丁：$config"
config=$(CDPATH= cd -- "$(dirname -- "$config")" && printf '%s/%s\n' "$(pwd -P)" "$(basename -- "$config")")
library="$root/lib64Creamlinux.so"
[ -f "$library" ] || fail "补丁文件不完整，请重新解压：$library"

if [ -n "${CREAM_STEAM_API:-}" ]; then
    steam_api=$CREAM_STEAM_API
elif [ -f "$root/libsteam_api.so" ]; then
    steam_api="$root/libsteam_api.so"
else
    candidates=$(find "$root" -type f -name libsteam_api.so -print)
    [ -n "$candidates" ] || fail "找不到游戏运行库，请确认补丁放在原生 Linux 版 EU4 的游戏根目录：$root"
    [ "$(printf '%s\n' "$candidates" | wc -l)" -eq 1 ] || fail '找到多个 Steam 运行库，请通过 CREAM_STEAM_API 指定游戏运行库的完整路径。'
    steam_api=$candidates
fi
[ -f "$steam_api" ] || fail "找不到指定的游戏运行库：$steam_api"

check_elf64() {
    magic=$(od -An -tx1 -N5 "$1" | tr -d ' \n')
    machine=$(od -An -tx1 -j18 -N2 "$1" | tr -d ' \n')
    [ "$magic" = 7f454c4602 ] && [ "$machine" = 3e00 ] || fail "运行库格式不匹配，请使用 64 位原生 Linux 版：$1"
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
# Steam's overlay uses RTLD_NEXT when resolving its loader hooks. It must
# precede our dlsym hook so its lookup cannot resolve back to the overlay.
preload="${LD_PRELOAD:+$LD_PRELOAD:}$runtime/lib64Creamlinux.so:$runtime/libsteam_api.so"
set +e
LD_PRELOAD="$preload" "$@"
status=$?
exit "$status"
