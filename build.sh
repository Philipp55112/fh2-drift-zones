#!/bin/sh
# Build the DriftZones logic plugin (freestanding, no C runtime).
# Needs: clang, lld-link, llvm-dlltool (LLVM). The two tiny import libraries are generated from lib/*.def.
# Usage:  sh build.sh            -> out/DriftZonesLogic.dll        (release, quiet)
#         sh build.sh debug      -> out/DriftZonesLogic_debug.dll  (hotkeys + verbose logs, development only)
set -e
cd "$(dirname "$0")"; mkdir -p out obj
[ -f lib/k32r.lib ] || llvm-dlltool -m i386:x86-64 -d lib/kernel32.def -l lib/k32r.lib
[ -f lib/u32.lib ]  || llvm-dlltool -m i386:x86-64 -d lib/user32.def  -l lib/u32.lib
R=1; NAME=DriftZonesLogic; [ "$1" = debug ] && { R=0; NAME=DriftZonesLogic_debug; }
CF="--target=x86_64-pc-windows-msvc -ffreestanding -O2 -fno-stack-protector -mno-stack-arg-probe -fno-builtin -Isrc -DRELEASE=$R"
clang $CF -c src/main.c  -o obj/main.obj
clang $CF -c src/rules.c -o obj/rules.obj
clang $CF -c src/flags.c -o obj/flags.obj
lld-link /dll /noentry /nodefaultlib /out:out/$NAME.dll obj/main.obj obj/rules.obj obj/flags.obj lib/k32r.lib lib/u32.lib
echo built out/$NAME.dll
