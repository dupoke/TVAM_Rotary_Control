# 旋转台控制程序当前计划（2026-05-01）

> 历史 M1-M7 阶段计划已经完成到 `v0.1` 可运行便携版。后续工作以现场联调和小范围修正为主，不再按 3 月初始开发里程碑推进。

## 1. 当前目标

- 稳定 `v0.1` 便携版交付。
- 保持源码、配置、文档与当前现场逻辑一致。
- 后续只做有明确现场反馈的小改动，避免重新设计架构。

## 2. 当前模块

| 模块 | 文件 | 当前职责 |
| --- | --- | --- |
| UI 编排 | `src/MainWindow.cpp`, `ui/MainWindow.ui` | 单页面 UI、按钮事件、回零与同步流程编排 |
| 板卡 | `src/BoardAdapter.cpp` | `COM_GAS_N` 串口连接、DI 读取、运动板卡 API 封装 |
| 运动 | `src/MotionService.cpp` | 点动、定位、连续旋转、角度/脉冲换算 |
| 光机 | `src/ProjectorService.cpp` | 光机串口、`WT+PWRE/WT+LEDE/WT+LEDS` 命令 |
| 序列 | `src/ImageSequenceService.cpp` | `deg` 命名与纯序号 PNG 序列解析 |
| 同步 | `src/SyncPlaybackService.cpp` | 连续旋转与投影切帧调度 |
| 相机 | `src/CameraService.cpp` | 相机枚举、预览、拍照、录像 |
| 投影窗口 | `src/ProjectionWindow.cpp` | 投影显示、对准图案、同步切帧 |
| 配置 | `src/ConfigManager.cpp`, `config/app_config.ini` | 默认参数、运动参数、串口参数 |

## 3. 当前关键行为

- Z 对射信号为 `DI0`，配置项为 `z_laser_di_index=0`。
- 旋转台机械回零信号为 `DI1`，配置项为 `coarse_home_di_index=1`。
- 旋转台通孔精零对射信号为 `DI0`，配置项为 `fine_home_di_index=0`。
- 通孔精零按 `fine_home_direction` 慢速寻找通孔，记录通孔两侧边缘 `P1/P2`，回到 `P0=(P1+P2)/2` 后清零。
- 通孔精零未找到通孔会停止并提示调整 Z，完成归零后也提示调整 Z。
- 同步开始要求先通过软件开启光机，光机开启后默认 LED 关闭；确认后先在 LED 关闭时预载首帧，设置亮度、开启 LED 后立即启动旋转，不主动显示黑帧或固定等待。
- 图片序列支持 `0deg.png` 角度命名和 `0000.png` 连续纯序号命名。
- 程序不直接读取 `EXR`，DrTVAM 图案需要先转成 `PNG`。

## 4. 后续最小任务清单

1. 现场确认 `DI0` 对射显示和通孔精零重复性。
2. 现场确认预载首帧后 LED 与旋转立即启动的同步时序是否合适。
3. 在目标 Win11 笔记本验证当前便携包完整运行。
4. 如果需要发布源码，更新 `E:\RotaryTableControl_source_export` 后再推送私有 GitHub 仓库。
5. 如果现场修改了配置默认值，同步更新 `config/app_config.ini`、`README.zh-CN.md` 和 `docs/OPERATIONS.zh-CN.md`。

## 5. 不做事项

- 不新增 `EXR` 直接读取。
- 不在程序内做 `EXR -> PNG` 转换。
- 不重做 UI 架构。
- 不把便携包、构建目录、运行日志提交到源码仓库。
- 不删除 `third_party/board/x64/GAS.dll`，它是运行时必要厂商库。
