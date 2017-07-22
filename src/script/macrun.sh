#!/usr/bin/env bash
# run from project root

set -euo pipefail
IFS=$'\n\t'

premake/premake5.osx gmake
make -C build/macosx/gmake config=debug_x64
bash -c 'cd build/macosx/gmake && ../../../bin/macosx/gmake/x64/Debug/test-influxdb-c-rest'
