# TVAM 旋转控制软件 v0.1

[English Version](README.md)

## 项目概述

TVAM 旋转控制软件是一个面向 Windows 的桌面程序，用于 TVAM 工作流中的旋转台运动控制与投影同步播放。

当前 `v0.1` 范围包括：

- 板卡连接、DI 轮询、点动、绝对/相对运动、停止与急停
- 旋转台粗零点与精零点流程
- 光机串口控制、光机开关、LED 开关与亮度设置
- 连续旋转下的同步投影播放
- 相机预览、拍照与录像流程
- 面向 Win11 的便携版打包脚本

这个仓库定位为源码仓库，不保存构建产物、便携发布包和本地运行数据。

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

推荐的 TVAM 使用方式：

1. DrTVAM 先输出 `EXR`
2. 在程序外部把 `EXR` 转成 `PNG`
3. 在本程序里选择转换后的 `png_sequence` 文件夹

## 依赖说明

### 构建依赖

- Windows 10/11
- CMake `>= 3.16`
- MSVC 工具链（Visual Studio 2019/2022 或兼容 Build Tools）
- Qt，至少包含以下模块：
  - `Widgets`
  - `SerialPort`
  - `Multimedia`
  - `MultimediaWidgets`

### 可选依赖

- OpenCV
  - 用于相机相关的回退路径
  - 工程默认从 `third_party/opencv/...` 查找
  - OpenCV 二进制不随本仓库保存

### 运行时 / 厂商依赖

- `third_party/board/x64/GAS.dll`
  - 运动控制板卡厂商运行库
  - 因为运动控制链路依赖它，所以源码仓库中保留该文件

## 构建方式

```powershell
cmake -S . -B build
cmake --build build --config Release
```

生成的可执行文件：

```text
build/Release/RotaryTableControl.exe
```

## 便携版打包

便携版打包脚本：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package_win_portable.ps1
```

脚本会执行：

- `Release` 编译
- 复制 `GAS.dll`
- 使用 `windeployqt` 部署 Qt 运行库
- 如果检测到 OpenCV，则复制对应运行库
- 复制 `config/`
- 创建 `logs`、`captures`、`RunTimeLog`、`WatchData` 等运行目录

## 目录结构

```text
config/       程序配置
include/      头文件
scripts/      打包与辅助脚本
src/          C++ 源码
third_party/  必要的厂商运行库
ui/           Qt Designer 界面文件
```

## 备注

- 当前仓库版本定义为 `v0.1`
- 这一版预计不会再有大的架构调整
- `.gitignore` 已排除 `dist/`、`build/`、`tmp/`、运行日志以及本地 OpenCV 二进制
- 硬件型号参考见：[HARDWARE_REFERENCE.zh-CN.md](HARDWARE_REFERENCE.zh-CN.md)
