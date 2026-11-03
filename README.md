# TVAM Rotary Control v0.1

[中文说明](README.zh-CN.md)

## Overview

TVAM Rotary Control is a Windows desktop application for synchronized rotary motion and projection playback in a TVAM workflow.

Current `v0.1` scope:

- Motion board connection, DI polling, jog, relative-angle zero/return-to-zero, stop and emergency stop
- Mechanical home and through-hole fine-zero workflows for the rotary stage
- Projector serial control for power, LED enable and brightness
- Synchronized projection playback with continuous positive rotary motion
- Camera preview, capture and recording workflow
- Portable Windows 11 copy-and-run packaging

## Current Runtime Behavior

- The motion area shows a `0~360°` current relative angle. `Zero` only marks the current position as `0°`; `Return to zero` moves back to relative zero by the shortest path.
- Z calibration and through-hole fine zero share `DI0`, configured as `z_laser_di_index=0` and `fine_home_di_index=0`.
- Mechanical home uses `coarse_home_di_index=1` by default.
- Through-hole fine zero uses the independent `fine_home_speed_deg_s` speed and `fine_home_poll_interval_ms` DI polling interval while searching for the `DI0` through-hole signal. If no hole is found, it stops and prompts the operator to adjust Z.
- After the hole is found, the workflow records one edge, reverses through the hole, records the opposite edge, returns to `P0=(P1+P2)/2`, zeroes the axis, and prompts the operator to adjust Z.
- Projector power must be enabled from the software before `Start Projection` is available. The projection window is assigned to the external screen before it is first shown; startup is cancelled if no separate screen is available. The application shows a black frame only on that screen, sets brightness, turns the LED on, waits 200 ms, and then starts rotary synchronization. On completion or interruption, only the LED is turned off; projector power stays on.

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
