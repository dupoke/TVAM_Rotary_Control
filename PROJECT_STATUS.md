# 项目当前状态（2026-05-01）

## 当前版本

- 版本定位：`v0.1`
- 目标平台：Windows 11
- 当前交付方式：复制整个便携文件夹即可运行
- 当前便携目录：`dist/RotaryTableControl-win11-portable-20260429`
- 当前压缩包：`dist/RotaryTableControl-win11-portable-20260429.zip`

## 已完成

- Qt Widgets 单页面 UI 与中文操作文案
- `COM_GAS_N` 板卡串口连接、DI 轮询和状态显示
- 点动、停止、急停、绝对/相对运动、连续旋转
- 快速回零流程，默认 `DI1`
- 精确回零流程，默认旋转台限位 `DI1`
- 精确回零边沿中点逻辑：记录 `P1/P2`，计算 `P0`，回中后清零
- 精确回零启动时如果 `DI1` 已触发，会把当前脉冲作为 `P1`，继续寻找释放边沿
- 相机枚举、预览、拍照和录像流程
- 光机串口控制：开关、LED 开关、亮度百分比
- 光机开机成功后默认关闭 LED，未通过软件开启光机时不能开始同步
- 确认同步后黑屏开启 LED 并等待 3 帧，再启动旋转同步和正常投影
- 图片序列检查支持 `0deg.png` 和 `0000.png` 两类命名
- `EXR` 目录明确拒绝，提示选择转换后的 `png_sequence`
- Win11 便携打包脚本与当前便携包

## 当前默认配置

配置文件：`config/app_config.ini`

| 项 | 当前值 |
| --- | --- |
| 轴号 | `1` |
| 电机每圈脉冲 | `1000` |
| 减速比 | `180` |
| 每度脉冲 | `500` |
| 快速回零 DI | `DI1` |
| 精确回零 DI | `DI1` |
| 精确回零极性 | `active_low=false` |
| Z 对射 DI | `DI0` |
| 默认亮度 | `20%` |
| 默认一圈时间 | `20 s` |

## 仍需现场确认

- `DI0` 的触发/未触发显示是否与实际 Z 对射状态一致
- 精确回零在“DI1 启动时已触发”和“DI1 启动时未触发”两种状态下的重复精度
- 黑屏 3 帧预点亮 LED 是否足够稳定
- 同步开始后 LED 与第一帧投影是否满足工艺时序
- 便携包在目标 Win11 笔记本上的相机和串口枚举情况

## 常用命令

```powershell
cmake --build build --config Release
```

刷新当前便携包：

```powershell
Copy-Item -LiteralPath build\Release\RotaryTableControl.exe -Destination dist\RotaryTableControl-win11-portable-20260429\RotaryTableControl.exe -Force
Compress-Archive -LiteralPath dist\RotaryTableControl-win11-portable-20260429 -DestinationPath dist\RotaryTableControl-win11-portable-20260429.zip -Force
```
