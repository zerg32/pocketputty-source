# Building PocketPuTTY for Windows CE 2.11 (Jornada 820)

The HP Jornada 820 uses an ARMv4 processor and Windows CE 2.11. The supplied
`PocketPuTTY.vcp` is an old eMbedded Visual C++ project and its default CE
settings target a newer platform. `build-ce211.bat` builds the ARMv4 release
directly with EVC4's compiler while explicitly setting the CE 2.11 target.

## Requirements

- EVC4 compiler and linker under `C:\evc4\wince\bin`
- Standard SDK 4.2 ARMv4 headers and import libraries under
  `C:\evc4\sdk\STANDARDSDK_420`
- A Windows checkout of this repository; for example
  `C:\evc4\src\pocketputty-source`.

## Build

Open a Windows Command Prompt, change to the repository directory, and run:

```bat
build-ce211.bat
```

The output is `build\ce211\PocketPuTTY.exe`. The script compiles the project
sources for ARMV4 with `_WIN32_WCE=211`, links for the Windows CE 2.11 GUI
subsystem, and uses the ARMv4 `winsock.lib` rather than the newer `ws2.lib`.
It does not link `aygshell.lib` or `iphlpapi.lib`; those DLLs are not present in
the Jornada 820 ROM. For the RC step only, it creates and removes a temporary
empty `winver.h` because the resource script does not use version resources
and EVC4's SDK does not include that header.

The CE 2.11 compatibility code in `porting.c`, `winnet.c`, `window.c`, and
`noise.c` provides the old-platform fallbacks. Winsock starts
with version 1.0 on CE 2.11. Local-address detection on this OS recognizes
loopback addresses; the old Winsock does not offer the IP Helper enumeration
used by newer CE releases.

## Check the executable

From a Visual Studio/EVC command prompt, inspect the output with:

```bat
C:\evc4\wince\bin\dumpbin.exe /headers build\ce211\PocketPuTTY.exe
C:\evc4\wince\bin\dumpbin.exe /imports build\ce211\PocketPuTTY.exe
```

The headers should identify `ARM` and subsystem version `2.11`. The import
table should contain `commctrl.dll`, `COREDLL.dll`, and `WINSOCK.dll`; it must
not require `AYGSHELL.dll`, `WS2.dll`, or `iphlpapi.dll`. Check every imported
`COREDLL.dll` ordinal against the target ROM when changing the toolchain or
SDK.

## Test with CERF

From WSL, start the Jornada profile with the build output directory shared to
the guest (adjust the checkout path if needed):

```bash
cd "/home/serg/CE Runtime Foundation"
./cerf --device=jornada_820 --guest-additions \
  --ga-share-folder="/mnt/c/evc4/src/pocketputty-source/build/ce211" \
  --ga-autorun='"\CERF Storage\PocketPuTTY.exe"' \
  --tab=fb --log=ALL --log-file=/tmp/opencode/cerf-pocketputty.log \
  --flush-outputs
```

Complete the Windows CE first-boot Welcome wizard if it appears. The autorun
command must be quoted because the guest path contains a space. A successful
CERF autorun reports `autorun result 0x00000001`. Alternatively, open
`\CERF Storage\` in the guest and run `PocketPuTTY.exe` manually.
