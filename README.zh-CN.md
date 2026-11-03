# TVAM 旋转控制软件 v0.1

[English Version](README.md)

## 项目概述

TVAM 旋转控制软件是一个面向 Windows 的桌面程序，用于 TVAM 工作流中的旋转台运动控制与投影同步播放。

当前 `v0.1` 范围：

- 板卡连接、DI 轮询、点动、相对角度归零/回零、停止与急停
- 旋转台机械回零与通孔精零流程
- 光机串口控制、光机开关、LED 开关与亮度设置
- 连续正方向旋转下的同步投影播放
- 相机预览、拍照与录像流程
- 面向 Windows 11 的复制即用便携版打包

## 当前运行行为

- 运动区显示 `0~360°` 当前相对角度；`归零` 只把当前位置记为 `0°`，`回零` 按最短路径回到相对零点。
- Z 校准对射与旋转台通孔精零定位默认共用 `DI0`，配置为 `z_laser_di_index=0`、`fine_home_di_index=0`。
- 机械回零默认使用 `coarse_home_di_index=1`。
- 通孔精零会按 `fine_home_speed_deg_s` 独立低速和 `fine_home_poll_interval_ms` 轮询寻找 `DI0` 通孔；未找到通孔会停止并提示调整 Z 位置。
- 找到通孔后，程序会同向记录一侧边缘，反向穿过通孔记录另一侧边缘，计算 `P0=(P1+P2)/2`，回到 `P0` 后清零，并提示调整 Z 位置。
- 需要先点击“光机开”；光机开成功后才允许点击“开始投影”。确认后程序先在 LED 关闭时预载首帧，再设置亮度并开启 LED，稳定 200 ms 后启动旋转同步，不再主动显示黑帧。

## 投影序列支持

当前程序只支持位图序列，不支持在程序内直接读取 `EXR`。

支持的命名方式：

- 角度命名，例如 `0deg.png`、`2deg.png`、`15.5deg.png`
- 纯序号命名，例如 `0000.png` 到 `0179.png`

纯序号序列规则：

- 必须从 `0000` 开始
- 序号必须连续，中间不能缺号
- 整个序列按一整圈 `360°` 均匀映射
- 如果总帧数是 `180`，则每帧对应 `2°`

推荐 DrTVAM 使用方式：

1. DrTVAM 先输出 `EXR`
2. 在程序外部把 `EXR` 转成 `PNG`
3. 在本程序里选择转换后的 `png_sequence` 文件夹

## 依赖说明

构建依赖：

- Windows 10/11
- CMake `>= 3.16`
- MSVC 工具链
- Qt `Widgets`、`SerialPort`、`Multimedia`、`MultimediaWidgets`

运行时/厂商依赖：

- `third_party/board/x64/GAS.dll`，用于 `COM_GAS_N` 运动控制板卡
- 可选 OpenCV 运行库，默认从 `third_party/opencv/...` 查找，用于相机回退路径

## 构建方式

```powershell
cmake -S . -B build
cmake --build build --config Release
```

生成文件：

```text
build/Release/RotaryTableControl.exe
```

## 便携版

当前已交付便携包：

```text
dist/RotaryTableControl-win11-portable-20260429/
dist/RotaryTableControl-win11-portable-20260429.zip
```

打包脚本：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package_win_portable.ps1
```

脚本会编译 Release、复制 `GAS.dll`、用 `windeployqt` 部署 Qt 运行库、复制配置，并创建 `logs`、`captures`、`RunTimeLog`、`WatchData` 等运行目录。

## 目录结构

```text
config/       程序配置
docs/         操作与交接文档
include/      头文件
scripts/      打包与辅助脚本
src/          C++ 源码
third_party/  必要厂商运行库与本地可选依赖
ui/           Qt Designer 界面文件
```

操作说明见 [docs/OPERATIONS.zh-CN.md](docs/OPERATIONS.zh-CN.md)，硬件型号说明见 [HARDWARE_REFERENCE.zh-CN.md](HARDWARE_REFERENCE.zh-CN.md)。
