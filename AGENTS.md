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
- Current default brightness: `100%`.
- Current default round time: `20 s`.
- Z / through-hole through-beam input: `DI0`, configured by `z_laser_di_index=0`.
- Rotary through-hole fine-zero input: `DI0`, configured by `fine_home_di_index=0`.
- Through-hole beam logic: blocked = raw `DI0 ON`, clear through hole = raw `DI0 OFF`; software uses `active_low=true`.
- Mechanical home input: `DI1`, configured by `coarse_home_di_index=1`.
- Rotary through-hole fine-zero speed: `fine_home_speed_deg_s=0.2`, with `fine_home_poll_interval_ms=5`.

## Current Behavior Notes

- The motion area uses a software-relative `0~360°` angle; pulses remain internal hardware counts.
- `归零` marks the current position as software `0°` without motion or `GA_ZeroPos`; `回零` returns to relative `0°` by the shortest path.
- Rotary through-hole fine zero scans the through-hole on `DI0` at reduced speed. If no hole is found within the configured travel, it stops and prompts the operator to adjust Z.
- After the through-hole is found, through-hole fine zero records one hole edge, reverses through the hole, records the opposite edge, returns to the midpoint, zeroes the axis, then prompts the operator to adjust Z.
- Start projection is enabled only after software projector power-on. After confirmation, the app preloads the first sequence frame while the LED is off, sets brightness, turns the LED on, waits 200 ms for it to settle, then starts synchronized rotation; no deliberate black frame is shown.
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
