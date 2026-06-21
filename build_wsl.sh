#!/bin/bash

set -eu
cd "$(dirname "$0")"
set -o pipefail

WIN_PATH=$(wslpath -w "$(pwd)")
ARGS="$*"
CMD="build.bat \"$WIN_PATH\" $ARGS"

stdbuf -oL /mnt/c/Windows/System32/cmd.exe /c "$CMD" 2>&1 | \
  sed -u \
  -e 's@\([a-zA-Z]\):\\@/mnt/\L\1/@g' \
  -e 's@\\@/@g'
