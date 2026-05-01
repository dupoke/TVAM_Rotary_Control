# TVAM Rotary Control v0.1

[中文说明](README.zh-CN.md)

## Overview

TVAM Rotary Control is a Windows desktop application for synchronized rotary motion and projection playback in a TVAM workflow.

Current `v0.1` scope:

- Motion board connection, DI polling, jog, absolute/relative move, stop and emergency stop
- Fast home and precise home workflows for the rotary stage
- Projector serial control for power, LED enable and brightness
- Synchronized projection playback with continuous positive rotary motion
- Camera preview, capture and recording workflow
- Portable Windows 11 copy-and-run packaging

## Current Runtime Behavior

- Z calibration uses `z_laser_di_index=0`; rotary precise home uses `fine_home_di_index=1`.
- Fast home uses `coarse_home_di_index=1` by default.
- Precise home rotates in the positive direction at a reduced speed, records `P1` when `DI1` enters the limit-triggered region, records `P2` when `DI1` exits that region, returns to `P0=(P1+P2)/2`, then zeroes the axis.
- If rotary precise home starts while `DI1` is already triggered, the current pulse is treated as `P1`; the workflow continues to search for the exit edge.
- Synchronized projection requires projector power to be enabled from the software first. Power-on sends LED off by default. Start sync shows black frames, turns LED on, waits 3 black frames, then starts rotary synchronization and normal projection.

## Projection Sequence Support

The application supports bitmap image sequences only. `EXR` input is not supported in-app.

Supported naming modes:

- Angle-based names, for example `0deg.png`, `2deg.png`, `15.5deg.png`
- Pure indexed names, for example `0000.png` to `0179.png`

Indexed sequence rules:

- The first frame must start at `0000`
- Indices must be continuous with no gaps
- The full sequence is mapped uniformly to `360°`
- A `180` frame sequence maps to `2°` per frame

Recommended DrTVAM workflow:

1. Export patterns from DrTVAM as `EXR`
2. Convert them to `PNG` outside this application
3. Select the converted `png_sequence` folder in the UI

## Dependencies

Build-time:

- Windows 10/11
- CMake `>= 3.16`
- MSVC toolchain
- Qt `Widgets`, `SerialPort`, `Multimedia`, `MultimediaWidgets`

Runtime/vendor:

- `third_party/board/x64/GAS.dll` for the COM_GAS_N motion board
- Optional OpenCV runtime under `third_party/opencv/...` for camera fallback paths

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Output executable:

```text
build/Release/RotaryTableControl.exe
```

## Portable Package

Current delivered package:

```text
dist/RotaryTableControl-win11-portable-20260429/
dist/RotaryTableControl-win11-portable-20260429.zip
```

Packaging script:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package_win_portable.ps1
```

The script builds Release, copies `GAS.dll`, deploys Qt runtime with `windeployqt`, copies config, and creates runtime folders such as `logs`, `captures`, `RunTimeLog`, and `WatchData`.

## Repository Layout

```text
config/       Application configuration
docs/         Operator and handoff documentation
include/      Headers
scripts/      Packaging and utility scripts
src/          C++ source files
third_party/  Required vendor runtime and local optional dependencies
ui/           Qt Designer UI files
```

See [docs/OPERATIONS.zh-CN.md](docs/OPERATIONS.zh-CN.md) for the operator runbook and [HARDWARE_REFERENCE.zh-CN.md](HARDWARE_REFERENCE.zh-CN.md) for hardware model references.
