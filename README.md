# TVAM Rotary Control v0.1

[中文说明](README.zh-CN.md)

## Overview

TVAM Rotary Control is a Windows desktop application for synchronized rotary motion and projection playback in a TVAM workflow.

Current scope in `v0.1`:

- Motion board connection, DI polling, jog, absolute/relative move, stop and emergency stop
- Coarse home and fine home workflows for the rotary stage
- Projector serial control, power, LED and brightness commands
- Synchronized projection playback with continuous rotary motion
- Camera preview, capture and recording workflow
- Portable Windows packaging script for Win11 deployment

This repository is intended to be the source repository. Build outputs, portable packages and local runtime data are intentionally excluded.

## Projection Sequence Support

The application currently supports bitmap image sequences only. `EXR` input is not supported in-app.

Supported naming modes:

- Angle-based names, for example `0deg.png`, `2deg.png`, `15.5deg.png`
- Pure indexed names, for example `0000.png` to `0179.png`

Rules for indexed sequences:

- The first frame must start at `0000`
- Indices must be continuous with no gaps
- The full sequence is mapped uniformly to `360°`
- A `180` frame sequence maps to `2°` per frame

Recommended TVAM workflow:

1. Export patterns from DrTVAM as `EXR`
2. Convert them to `PNG` outside this application
3. Select the converted `png_sequence` folder in the UI

## Dependencies

### Build-time

- Windows 10/11
- CMake `>= 3.16`
- MSVC toolchain (Visual Studio 2019/2022 or compatible Build Tools)
- Qt with these modules:
  - `Widgets`
  - `SerialPort`
  - `Multimedia`
  - `MultimediaWidgets`

### Optional build/runtime

- OpenCV
  - Used for camera-related fallback paths
  - The project looks for OpenCV under `third_party/opencv/...`
  - OpenCV is intentionally not stored in this repository

### Runtime / vendor dependency

- `third_party/board/x64/GAS.dll`
  - Vendor runtime for the motion control board
  - Included in this source repository because the motion path depends on it

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Output executable:

```text
build/Release/RotaryTableControl.exe
```

## Portable Packaging

Portable packaging is handled by:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package_win_portable.ps1
```

The script:

- Builds `Release`
- Copies `GAS.dll`
- Deploys Qt runtime with `windeployqt`
- Copies OpenCV runtime DLLs when available
- Copies `config/`
- Creates runtime folders such as `logs`, `captures`, `RunTimeLog`, `WatchData`

## Repository Layout

```text
config/       Application configuration
include/      Public/internal headers
scripts/      Packaging and utility scripts
src/          C++ source files
third_party/  Vendor runtime kept in source control when required
ui/           Qt Designer UI files
```

## Notes

- This repository is versioned as `v0.1`
- No major architectural changes are expected for the current release line
- `dist/`, `build/`, `tmp/`, runtime logs and local OpenCV binaries are excluded by `.gitignore`
- Hardware model summary: [HARDWARE_REFERENCE.zh-CN.md](HARDWARE_REFERENCE.zh-CN.md)
