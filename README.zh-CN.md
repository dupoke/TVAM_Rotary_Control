# TVAM 旋转控制软件 v0.1

[English Version](README.md)

## 项目概述

TVAM 旋转控制软件是一个面向 Windows 的桌面程序，用于 TVAM 工作流中的旋转台运动控制与投影同步播放。

当前 `v0.1` 范围：

- 板卡连接、DI 轮询、点动、绝对/相对运动、停止与急停
- 旋转台快速回零与精确回零流程
- 光机串口控制、光机开关、LED 开关与亮度设置
- 连续正方向旋转下的同步投影播放
- 相机预览、拍照与录像流程
- 面向 Windows 11 的复制即用便携版打包

## 当前运行行为

- Z 校准对射默认使用 `z_laser_di_index=0`；旋转台精确回零默认使用 `fine_home_di_index=1`。
- 快速回零默认使用 `coarse_home_di_index=1`。
- 精确回零会慢速正向旋转，进入 DI1 限位触发区时记录 `P1`，离开触发区时记录 `P2`，计算 `P0=(P1+P2)/2`，反向回到 `P0` 后清零。
- 如果启动精确回零时 `DI1` 已经触发，程序会把当前脉冲作为 `P1`，继续正向寻找释放边沿。
- “开始同步”要求先通过软件点击“光机开”。光机开机会默认发送 LED 关闭命令；同步确认后先黑屏开启 LED 并等待 3 帧，再启动旋转同步和正常投影。

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
