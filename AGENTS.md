# AGENTS.md

## Coding Rules

- Prefer small, surgical changes. Do not refactor unrelated code.
- Verify against live code and `config/app_config.ini` before trusting older planning documents.
- Do not revert existing user changes in this dirty worktree.
- Use `apply_patch` for manual edits.
- Keep operator-facing UI text in Chinese.
- Keep source, config and docs in UTF-8.

## Current Project Facts

- Main app: Qt Widgets / CMake / C++17.
- Target platform: Windows 11 portable folder.
- Current portable artifact: `dist/RotaryTableControl-win11-portable-20260429`.
- Motion board runtime: `third_party/board/x64/GAS.dll`.
- Board type: `COM_GAS_N`.
- Projector: Wentins `4710`, controlled through serial commands `WT+PWRE`, `WT+LEDE`, `WT+LEDS`.
- Current default brightness: `20%`.
- Current default round time: `20 s`.
- Z through-beam input: `DI0`, configured by `z_laser_di_index=0`.
- Rotary precise home input: `DI1`, configured by `fine_home_di_index=1`.
- Coarse home input: `DI1`, configured by `coarse_home_di_index=1`.

## Current Behavior Notes

- Rotary precise home scans DI1 in the positive direction at reduced speed, records switch entry and exit pulses, returns to the midpoint, then zeroes the axis.
- If rotary precise home starts while `DI1` is already triggered, the current pulse is used as `P1`; the workflow continues to seek the exit edge.
- Start sync is enabled only after software projector power-on. Power-on sends LED off. Sync confirmation shows black frames, turns LED on, waits 3 black frames, then starts synchronized rotation and normal projection.
- Projection sequences support both `0deg.png` style names and continuous indexed names such as `0000.png` to `0179.png`.
- In-app `EXR` loading is not supported; use an external `EXR -> PNG` conversion and select the resulting `png_sequence` folder.

## Packaging

For normal package refresh:

```powershell
cmake --build build --config Release
Copy-Item -LiteralPath build\Release\RotaryTableControl.exe -Destination dist\RotaryTableControl-win11-portable-20260429\RotaryTableControl.exe -Force
Compress-Archive -LiteralPath dist\RotaryTableControl-win11-portable-20260429 -DestinationPath dist\RotaryTableControl-win11-portable-20260429.zip -Force
```

For a clean portable folder rebuild:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package_win_portable.ps1
```

The script creates `dist/RotaryTableControl-win11-portable` by default. The dated `20260429` folder is the currently handed-off package name.
