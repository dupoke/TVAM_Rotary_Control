# 操作运行手册

更新日期：2026-05-01

## 便携版运行

当前交付路径：

```text
E:\RotaryTableControl\dist\RotaryTableControl-win11-portable-20260429
```

复制整个文件夹到 Windows 11 笔记本后，双击：

```text
RotaryTableControl.exe
```

不要只复制单个 `exe`。便携版运行依赖同目录下的 Qt DLL、`GAS.dll`、`config/` 和运行目录。

## 默认硬件映射

| 功能 | 默认输入 |
| --- | --- |
| Z 对射校准 / 通孔精零对射 | `DI0` |
| 机械回零限位 | `DI1` |
| 急停 | `DI2` |
| 伺服报警 | `DI3` |

当前配置文件：

```text
config/app_config.ini
```

关键项：

```ini
fine_home_di_index=0
fine_home_active_low=true
fine_home_direction=-1
fine_home_speed_deg_s=0.2
fine_home_poll_interval_ms=5
fine_home_max_travel_deg=360
fine_home_timeout_ms=0
z_laser_di_index=0
z_laser_active_low=true
coarse_home_di_index=1
```

## 运动区

运动区面向操作员只使用 `0~360°` 循环相对角度。底层仍使用脉冲完成板卡定位、同步投影、机械回零和通孔精零，但脉冲不作为日常操作项显示。

- `当前相对角度`：以最近一次软件归零点为 `0°` 显示当前循环角。
- `归零`：只把当前位置记为 `0°`，不运动，不调用 `GA_ZeroPos`。
- `回零`：按当前相对角度走最短路径回到 `0°`。例如 `30°` 回零走 `-30°`，`350°` 回零走 `+10°`。
- `机械回零`：使用 `DI1` 机械限位完成硬件回零，成功后同步把相对角度置为 `0°`。
- `通孔精零`：使用 `DI0` 通孔对射完成传感器精零，成功后同步把相对角度置为 `0°`。

同步运行中、报警中、机械回零或通孔精零进行中，不允许执行 `归零` 或 `回零`。

## 通孔精零流程

1. 程序按 `fine_home_direction` 方向以 `fine_home_speed_deg_s` 独立低速旋转，并按 `fine_home_poll_interval_ms` 轮询 `DI0` 通孔对射。
2. 如果超过 `fine_home_max_travel_deg` 仍未检测到通孔，程序停止并弹窗提示调整 Z 位置。
3. 找到通孔后，继续同向低速旋转到通孔第一侧边缘并记录 `P1`。
4. 程序反向低速穿过通孔，到另一侧边缘时记录 `P2`。
5. 程序计算 `P0=(P1+P2)/2`，回到 `P0` 后执行轴清零。
6. 通孔精零完成后，程序弹窗提示继续调整 Z 位置。

如果点击“通孔精零”时 `DI0` 已经触发，程序会认为当前已在通孔内，继续寻找第一侧边缘。

`DI1` 只用于机械回零限位，不参与通孔精零。

孔径较小时应优先降低 `fine_home_speed_deg_s`。默认 `0.2 °/s` 和 `5 ms` 轮询下，理论采样步进约 `0.001°`。

现场实测对射逻辑：遮挡时 raw `DI0 ON`，通光/通孔时 raw `DI0 OFF`；因此 `fine_home_active_low` 与 `z_laser_active_low` 默认为 `true`。

## 投影序列选择

程序支持位图序列，不支持直接选择 `EXR` 目录。

推荐流程：

1. DrTVAM 生成 `EXR` 图案。
2. 在程序外转换成 `PNG`。
3. 在本程序中选择转换后的 `png_sequence` 文件夹。

支持两类命名：

| 模式 | 示例 | 规则 |
| --- | --- | --- |
| 角度命名 | `0deg.png`, `2deg.png` | 按文件名角度排序 |
| 纯序号命名 | `0000.png` 到 `0179.png` | 必须从 `0000` 开始且连续，按总帧数均分 `360°` |

如果选择的目录只有 `EXR`，程序会提示选择转换后的 `png_sequence` 目录。

## 开始投影

点击“开始投影”后的当前顺序：

1. 程序检查板卡、回零状态、安全状态、图片序列和一圈时间。
2. 只有软件已成功执行“光机开”且光机串口仍连接时，`开始投影` 按钮才可点击。
3. 程序弹出确认窗口，显示亮度、循环次数、一圈时间和投影序列文件夹。
4. 点击“确定”后，程序只在外接投影屏显示黑帧；主控屏保持正常显示。
5. 程序设置当前亮度并发送 LED 开启命令。
6. LED 开启后等待约 200 ms 让光机命令生效，期间旋转台不动。
7. 等待结束后，程序启动旋转与正常投影同步。

点击“取消”不会启动 LED，也不会启动同步。

## 重新封装

只刷新当前便携包：

```powershell
cmake --build build --config Release
Copy-Item -LiteralPath build\Release\RotaryTableControl.exe -Destination dist\RotaryTableControl-win11-portable-20260429\RotaryTableControl.exe -Force
Compress-Archive -LiteralPath dist\RotaryTableControl-win11-portable-20260429 -DestinationPath dist\RotaryTableControl-win11-portable-20260429.zip -Force
```

完全重建便携目录：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package_win_portable.ps1
```

注意：脚本默认输出 `dist/RotaryTableControl-win11-portable`。如果要继续沿用当前交付名，需要再同步到 `RotaryTableControl-win11-portable-20260429`。
