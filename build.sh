#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
cmake -S "$root" -B "$root/build" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build "$root/build" --parallel
ctest --test-dir "$root/build" --output-on-failure
cmake --install "$root/build" --prefix "$root/output"
printf '%s\n' "Package ready: $root/output"
