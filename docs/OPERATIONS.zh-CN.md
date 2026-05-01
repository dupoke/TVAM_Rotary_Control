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
| Z 对射校准 | `DI0` |
| 快速回零 / 精确回零机械限位 | `DI1` |
| 急停 | `DI2` |
| 伺服报警 | `DI3` |

当前配置文件：

```text
config/app_config.ini
```

关键项：

```ini
fine_home_di_index=1
fine_home_active_low=false
fine_home_max_travel_deg=360
fine_home_timeout_ms=60000
z_laser_di_index=0
z_laser_active_low=true
coarse_home_di_index=1
```

## 精确回零流程

1. 程序按当前点动速度的低速比例正向旋转。
2. `DI1` 进入限位触发状态时记录 `P1`。
3. 继续正向旋转，`DI1` 离开限位触发状态时记录 `P2`。
4. 程序计算 `P0=(P1+P2)/2`。
5. 程序反向回到 `P0`。
6. 回到 `P0` 后执行轴清零。

如果点击“精确回零”时 `DI1` 已经触发，程序不再拒绝操作，会把当前脉冲作为 `P1`，继续正向寻找离开触发区的边沿。

`DI0` 只用于 Z 对射状态显示和“确认 Z 到位”，不参与旋转台回零。

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

## 开始同步

点击“开始同步”后的当前顺序：

1. 程序检查板卡、回零状态、安全状态、图片序列和一圈时间。
2. 如果未先通过软件点击“光机开”，开始同步按钮不可点击。
3. 点击“光机开”成功后，程序发送光机开机命令，并立即发送 LED 关闭命令。
4. 点击“开始同步”后弹出确认框，显示亮度、循环次数、一圈时间和投影序列文件夹。
5. 点击“确定”后，程序打开投影窗口并显示全黑图案。
6. 程序设置当前亮度，发送 LED 开启命令。
7. 程序按当前序列帧间隔保持 3 帧全黑，期间旋转台不动。
8. 黑图结束后，程序启动旋转与正常投影同步。

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
