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
- 点动、停止、急停、相对角度归零/回零、连续旋转
- 运动区显示 `0~360°` 当前相对角度，脉冲仅作为内部硬件计数
- 机械回零流程，默认 `DI1`
- 通孔精零流程，默认使用 `DI0` 通孔对射
- 通孔精零通孔中点逻辑：找到通孔后记录两侧边缘 `P1/P2`，计算 `P0`，回中后清零
- 通孔精零未找到通孔时停止并提示调整 Z 位置，完成后也提示调整 Z 位置
- 相机枚举、预览、拍照和录像流程
- 光机串口控制：开关、LED 开关、亮度百分比
- 光机开成功后才允许开始投影
- 开始投影确认后先在 LED 关闭时预载首帧，再设置亮度并开启 LED
- 开启 LED 后立即启动旋转同步，不主动插入黑帧或固定等待
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
| 机械回零 DI | `DI1` |
| 通孔精零 DI | `DI0` |
| 通孔精零极性 | `active_low=true` |
| 通孔精零速度 | `0.2 °/s` |
| 通孔精零 DI 轮询 | `5 ms` |
| Z 对射 DI | `DI0` |
| Z 对射极性 | `active_low=true` |
| 默认亮度 | `100%` |
| 默认一圈时间 | `20 s` |

## 仍需现场确认

- `DI0` 的“通光/遮挡”显示是否与实际 PNP 通孔对射状态一致
- 通孔精零在“DI0 启动时已触发”和“DI0 启动时未触发”两种状态下的重复精度
- LED 开启与运动启动的实际同步效果是否满足现场工艺要求
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
