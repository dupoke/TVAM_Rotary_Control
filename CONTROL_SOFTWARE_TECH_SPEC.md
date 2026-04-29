# 旋转台控制软件技术文档（V1.1）

## 1. 文档目的
本文件用于指导从零开始开发“单页面旋转台控制软件”，覆盖板卡通信、伺服运动、孔位精定位、相机功能、光机投影联动、安全策略、参数管理与分阶段实施路径。

## 2. 已确认需求（冻结项）
- 控制轴：旋转轴 `Axis 1`
- 伺服：一体化伺服，`1000 脉冲/电机圈`
- 减速比：`1:90`
- 旋转台零点：
  - 粗零点：旋转台自带零点开关（用于上电自检）
  - 精零点：激光对射检测定位孔（边沿中点法）
- Z 轴：手动位移台，不做电动控制；软件提供“手动到位确认”
- 激光开关：`NPN + NC`
- 最大电机转速：`1500 rpm @ 24V`
- 相机：USB UVC，相机功能优先 `MP4` 录像
- 光机：参考 `QCOM_V1.6`/`pro4710.ini` 中的协议命令进行控制
  - `WT+PWRE=1/0`：光机总开关
  - `WT+LEDE=1/0`：LED 开关
  - `WT+LEDS=0~1023`：LED 亮度
  - 串口参数：`115200, 8N1, FlowControl=None`（COM 口随 USB 插拔动态变化）
- 需支持导入按角度编号的图片文件夹（示例：`0.0deg`, `2.0deg`, ... , `358deg`）
- 伺服使能：硬件常使能（软件不控制使能DO）
- UI：单页面（不做多页面切换）
- UI 语言：统一中文（操作员可见文案不使用英文）
- 文本编码：项目源码/配置/文档统一 UTF-8（建议无 BOM）

## 3. 关键换算
- 电机 1 圈脉冲：`1000 pulse`
- 旋转台 1 圈脉冲：`1000 * 90 = 90000 pulse`
- 角度换算：`1 deg = 250 pulse`
- 角度转脉冲：`pulse = deg * 250`
- 脉冲转角度：`deg = pulse / 250`

速度换算：
- 电机 `1500 rpm` -> `25 rev/s` -> `25000 pulse/s` -> `25 pulse/ms`
- 旋转台最高转速约 `16.67 rpm`
- 旋转台最高角速度约 `100 deg/s`

## 4. 硬件与I/O映射（建议）
> 最终接线以现场端子编号为准，本表先定义软件逻辑映射。

- `DI0`：孔位激光开关（NPN NC）
- `DI1`：旋转台自带零点开关（粗回零/自检）
- `DI2`：急停（预留）
- `DI3`：伺服报警（预留）
- `DO0`：预留（蜂鸣器/指示灯）
- `DO1`：预留

说明：
- DI逻辑统一做“可配置极性”，默认按 NC 初始化。
- 现场可通过配置切换“高有效/低有效”，不改代码。

## 5. 软件功能范围

### 5.1 设备连接
- 串口连接板卡（自动搜索或指定COM）
- 板卡复位、状态查询、错误码显示
- 实时显示DI位状态
- 光机串口枚举（USB 动态 COM）与手动选择
- 光机串口打开/关闭与连接状态显示

### 5.2 运动控制
- 点动（正转/反转）
- 绝对角度运动
- 相对角度运动
- 平滑停止/急停
- 当前角度显示（由规划位置换算）

### 5.3 回零与对位
- 上电后强制执行一次“粗零点自检”
- 精零点：通过定位孔激光边沿中点计算零点
- 零点偏移参数管理（可保存）

### 5.4 Z手动确认
- 操作员手动调Z
- 软件显示孔位传感器状态稳定后，允许点击“确认Z到位”
- 记录确认时间、操作者、当前角度

### 5.5 相机功能（单页面内）
- 相机枚举与切换
- 预览
- 拍照
- 录像（MP4优先）
- 参数调节：亮度、对比度、曝光、饱和度、黑白/彩色

### 5.6 光机与同步投影
- 光机总开关控制（发送 `WT+PWRE`）
- LED 开关控制（写 `WT+LEDE`）
- LED 亮度百分比输入（`0~100%`），并换算写入 `WT+LEDS(0~1023)`
- 光机命令通过独立串口链路发送，不依赖固定 COM 号
- 导入图片文件夹后自动检查：
  - 识别文件名中的角度编号（`*deg`，允许前后缀）
  - 角度值可排序、可追溯，检查非法命名/重复角度
  - 给出检查结果（通过/失败 + 原因）
- 同步运行：
  - 点击“开始”后，旋转台连续旋转，同时按角度序列投影对应图片
  - 默认策略：连续旋转 + 按角度触发切图（不采用逐点停转）
  - 当设置一圈时长为 `T`、有效图片数为 `N` 时，平均切图节拍为 `T/N`
  - 支持循环次数（有限循环）与“无限循环”两种模式
  - 达到设定循环次数后自动停止；支持人工停止

### 5.7 日志与告警
- 操作日志（连接、回零、运动、拍照录像、光机开关、LED开关、亮度调整、图片目录导入、同步开始/停止、循环计数）
- 告警日志（通信失败、回零超时、DI异常、图片序列检查失败、同步中断等）

## 6. 安全策略（强制）

### 6.1 上电联锁
- 软件启动后进入 `SELF_CHECK_COARSE` 状态
- 未通过粗零点自检前，禁止运动命令

### 6.2 运行联锁
- 通信丢失、急停触发、报警触发时：
  - 立即 `GA_Stop(..., 紧急停止)`
  - 状态切换至 `ALARM`
  - 需人工复位后恢复

### 6.3 失效安全
- 传感器使用 NC，断线/掉电更易显性化
- DI异常抖动采用去抖和稳定时间判定

### 6.4 光机联锁（新增）
- `ALARM`、急停或通信丢失时，必须立即终止同步流程（旋转命令与投影切换）
- 同步流程异常退出时，禁止继续递进下一张投影图
- 默认策略：异常停机后将 LED 关闭（写 `WT+LEDE=0`）

## 7. 状态机定义
- `BOOT`：程序启动
- `CARD_CONNECTING`：板卡连接中
- `SELF_CHECK_COARSE`：粗零点自检
- `READY`：可运行
- `FINE_HOMING`：孔位精零点中
- `JOGGING`：点动中
- `POSITIONING`：定位运动中
- `SYNC_PREPARE`：同步任务准备中（目录检查/参数校验）
- `SYNC_RUNNING`：旋转与投影同步循环中
- `ALARM`：报警锁定

核心流转：
1. `BOOT -> CARD_CONNECTING -> SELF_CHECK_COARSE -> READY`
2. `READY -> FINE_HOMING -> READY`
3. `READY -> JOGGING/POSITIONING -> READY`
4. `READY -> SYNC_PREPARE -> SYNC_RUNNING -> READY`
5. 任意状态异常 -> `ALARM`

## 8. 单页面UI设计（固定）

### 8.1 设备区
- `btnConnectCard`
- `lblCardStatus`
- `lblComPort`
- `diIndicator[0..3]`

### 8.2 运动区
- `btnJogPlus / btnJogMinus / btnStop / btnEStop`
- `editTargetAbsDeg / btnMoveAbs`
- `editTargetRelDeg / btnMoveRel`
- `lblCurrentDeg / lblCurrentPulse`

### 8.3 回零区
- `btnCoarseCheck`
- `btnFineHome`
- `lblHomeResult`
- `lblZeroErrorDeg`

### 8.4 Z确认区
- `lblLaserState`
- `btnConfirmZReady`
- `lblZConfirmTime`

### 8.5 相机区
- `comboCameraList / btnCameraOpen`
- `btnSnap / btnRecStart / btnRecStop`
- `sliderBrightness / sliderContrast / sliderExposure / sliderSaturation`
- `btnColorMode / btnMonoMode`

### 8.6 光机与同步投影区
- `btnProjectorPowerOn / btnProjectorPowerOff`
- `btnLedOn / btnLedOff`
- `editLedPercent / btnApplyLedPercent`
- `editImageFolder / btnBrowseImageFolder`
- `btnCheckImageFolder / lblImageCheckResult`
- `spinLoopCount / chkInfiniteLoop`
- `btnSyncStart / btnSyncStop`
- `lblSyncState / lblSyncProgress`

### 8.7 日志区
- `textLog`

## 9. 板卡API调用约定（COM_GAS_N）
主要接口（按文档）：
- 连接：`GA_Open`, `GA_Reset`, `GA_Close`
- IO：`GA_GetDiRaw`, `GA_GetExtDiBit`, `GA_SetExtDoBit`
- 点位：`GA_PrfTrap`, `GA_SetTrapPrm`, `GA_SetPos`, `GA_SetVel`, `GA_Update`
- JOG：`GA_PrfJog`, `GA_SetJogPrm`, `GA_SetVel`, `GA_Update`
- 状态：`GA_GetSts`, `GA_GetPrfPos`, `GA_ClrSts`, `GA_Stop`
- 零点/回零：`GA_ZeroPos`, `GA_HomeSetPrm`, `GA_HomeStart`, `GA_HomeGetSts`

建议：
- 外层统一封装 `BoardAdapter`，UI层不直接调用DLL API。
- 每条动作命令必须有超时和返回码检查。

## 10. 回零算法（实施版）

### 10.1 粗零点自检（上电必须）
1. 低速向预设方向旋转，等待 `DI1` 触发
2. 超时未触发 -> 报警
3. 触发后退回一定脉冲，二次慢速接近确认
4. 成功后写入粗零参考，进入 `READY`

### 10.2 精零点（激光孔中点）
1. 以低速扫描定位孔区域
2. 捕获孔位信号两个边沿位置：`P1`, `P2`
3. 计算 `P0 = (P1 + P2) / 2`
4. `GA_ZeroPos` 或写偏移，使 `P0` 对应机械零点
5. 若只抓到一个边沿或超时 -> 报警

说明：
- 该方法不依赖“只触发一次”的离散点，重复精度更高。

## 11. 默认参数（初版）
- `axis_index = 1`
- `pulse_per_motor_rev = 1000`
- `gear_ratio = 90`
- `pulse_per_deg = 250`
- `vel_max_pulse_ms = 20.0`（先低于理论上限25）
- `vel_jog_default = 2.0`
- `vel_home_coarse = 2.0`
- `vel_home_fine = 0.3`
- `acc_default = 0.5`
- `dec_default = 0.5`
- `home_timeout_ms = 30000`
- `di_debounce_ms = 10`
- `di_stable_ms = 30`
- `record_format = mp4`
- `sensor_mode = npn_nc`
- `projector_command_profile_ini = ./config/pro4710.ini`（可选，仅作命令模板参考）
- `projector_key_power = WT+PWRE`
- `projector_key_led_enable = WT+LEDE`
- `projector_key_led_brightness = WT+LEDS`
- `led_percent_default = 30`
- `image_angle_pattern = (?i).*?(\\d+(?:\\.\\d+)?)deg.*`
- `sync_loop_count_default = 1`
- `sync_infinite_default = false`
- `sync_round_time_sec_default = 60`
- `projector_com_port = ""`（空表示未绑定，运行时从枚举结果选择）
- `projector_baudrate = 115200`
- `projector_data_bits = 8`
- `projector_parity = none`
- `projector_stop_bits = 1`
- `projector_flow_control = none`
- `projector_send_with_enter = true`

## 12. 工程实现建议（Qt）
- 技术栈：Qt Widgets + QSerialPort + Qt Multimedia + QSettings/INI
- 模块划分：
  - `BoardAdapter`
  - `MotionService`
  - `HomingService`
  - `CameraService`
  - `ProjectorService`（光机串口命令发送，协议参考 `QCOM_V1.6/pro4710.ini`）
  - `ImageSequenceService`（图片目录解析与角度序列校验）
  - `SyncPlaybackService`（旋转与投影同步编排、循环控制）
  - `SafetyManager`
  - `ConfigManager`
  - `LogService`
  - `MainWindowController`

线程建议：
- 板卡轮询线程（DI/状态）
- 相机采集线程
- 同步播放线程（避免阻塞 UI）
- UI主线程仅做显示与命令发起

## 13. 分阶段开发里程碑（用于下一对话逐步落地）

### M1：项目骨架
- 建立Qt工程与单页面布局
- 加入日志系统与配置文件读写

### M2：板卡接入
- 实现连接/复位/断开
- DI状态实时显示

### M3：基础运动
- JOG与停止
- 绝对/相对运动
- 角度与脉冲换算

### M4：粗零点自检
- 上电自动执行粗零点流程
- 失败联锁

### M5：精零点
- 边沿采集与中点算法
- 精零点成功/失败闭环

### M6：相机与光机模块
- 枚举、预览、拍照、MP4录像
- 参数调节与模式切换
- 光机总开关、LED开关、亮度控制
- 图片目录导入与角度序列检查

### M7：安全与验收
- 异常联锁、日志完善、参数固化
- 旋转与投影同步循环联调（有限循环/无限循环）
- 联调与重复性测试

## 14. 验收标准（首版）
- 上电后若未通过粗零点，所有运动按钮禁用
- 精零点流程可重复执行，结果可追溯
- 角度定位误差满足工艺要求（现场标定）
- 相机可稳定拍照/录像/切换参数
- 光机总开关、LED开关、亮度百分比控制可用（可追溯）
- 图片目录检查能识别非法命名、缺失/重复角度并给出提示
- 同步运行在设定循环次数到达后自动停止
- 无限循环模式可稳定运行并可人工停止
- UI 操作文案为中文且显示无乱码
- 关键文本文件抽检为 UTF-8 编码（源码/配置/文档）
- 异常情况下可立即停机并记录日志

## 15. 待现场确认项（下一阶段第一优先）
- 粗零点方向（正向/负向）
- 回零低速最优值（`vel_home_fine`）
- DI端子实际编号与电平极性
- 定位孔机械公差与最终允许角度误差阈值
- 相机编码器与MP4封装在目标机上的兼容性
- 光机命令结束符细节（`Enter` 对应 `CR` 还是 `CRLF`）
- 图片序列“一圈完整”的判定规则（按角度跨度/按理论步距）
