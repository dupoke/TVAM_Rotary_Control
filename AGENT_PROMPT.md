# Agent Prompt

本文件保留给历史兼容。当前项目内 AI 约束以根目录 `AGENTS.md` 为准。

## 当前执行原则

- 先查 live code 和 `config/app_config.ini`，不要直接相信旧计划文档。
- 每次只做一个明确目标的最小闭环改动。
- 不重构无关代码，不删除用户已有改动。
- 手动代码/文档编辑使用 `apply_patch`。
- 操作员可见文案保持中文。
- 修改后尽量执行 `cmake --build build --config Release` 验证。

## 当前核心事实

- 当前版本为 `v0.1`。
- 当前便携包为 `dist/RotaryTableControl-win11-portable-20260429`。
- Z 对射为 `DI0`，快速回零和精确回零使用旋转台机械限位 `DI1`。
- 同步启动要求先通过软件点击“光机开”；光机开机会默认关闭 LED，确认同步后先黑屏开启 LED 并等待 3 帧，再启动旋转与正常投影。
- 图片序列支持 `0deg.png` 和 `0000.png` 两类命名。
- 程序不支持直接读取 `EXR`。
