# CameraService Qt6 迁移说明

## 版本信息
- 迁移日期: 2026-11-02
- Qt版本: Qt5 → Qt6.8.3
- 编译器: MinGW 13.1.0

## 主要改进

### 1. API 现代化
- **替换过时的 Qt5 API**
  - `QCameraInfo` → `QMediaDevices` + `QCameraDevice`
  - `QCameraViewfinder` → `QVideoWidget`
  - `QCameraImageCapture` → `QImageCapture`
  - 引入 `QMediaCaptureSession` 统一管理相机会话

### 2. 新增功能

#### 分辨率管理
```cpp
// 查询支持的分辨率
QList<CameraResolution> supportedResolutions() const;

// 获取当前分辨率
CameraResolution currentResolution() const;

// 设置分辨率
bool setResolution(int width, int height, QString* error = nullptr);
```

#### 帧率控制
```cpp
// 设置目标帧率
bool setFrameRate(double fps, QString* error = nullptr);
```

#### 增强的预览控件
- 更现代的 `QVideoWidget` 替代旧的 `QCameraViewfinder`
- 更好的性能和GPU加速支持
- 最小尺寸从 480x270 提升到 640x480

### 3. 录制改进

#### Qt6 原生录制
- 使用 `QMediaFormat` 设置输出格式
- 支持 MPEG4 容器 + H.264 编码
- 高质量预设 (`QMediaRecorder::HighQuality`)

#### OpenCV 录制（可选）
- 保留了 OpenCV 后备录制路径
- 支持 MJPEG (AVI) 和 MP4V 编码
- 智能后端选择 (MSMF/DSHOW/ANY)
- 当前版本默认禁用（MinGW 与 MSVC 编译的 OpenCV 不兼容）

### 4. 架构改进

#### 会话管理
```cpp
QMediaCaptureSession* captureSession_;  // 统一管理相机、拍照、录制
```

#### 时长追踪
- 使用独立的 `QTimer` 追踪录制时长
- 100ms 精度更新
- 支持 OpenCV 和 Qt 两种录制模式

#### 设备枚举
```cpp
cameraDevices_;  // 缓存 QCameraDevice 列表
```

### 5. 已知限制

#### 图像调整功能
Qt6 移除了 `QCameraImageProcessing`，以下功能暂不支持：
- 亮度调整
- 对比度调整
- 饱和度调整
- 曝光补偿
- 灰度模式

这些功能在 Qt6 中需要通过手动实现图像处理管线或使用 GPU 着色器。

## 编译配置

### CMake 选项
```cmake
set(ENABLE_OPENCV OFF CACHE BOOL "Enable OpenCV support")
```

当前默认关闭 OpenCV 以避免 MinGW/MSVC 链接器不兼容问题。

### 依赖的 Qt6 模块
- `Qt6::Widgets`
- `Qt6::Multimedia`
- `Qt6::MultimediaWidgets`
- `Qt6::SerialPort`

## 测试建议

1. **基本功能**
   - 检测相机列表
   - 打开/关闭相机
   - 预览显示

2. **拍照功能**
   - 拍照并保存 JPG
   - 检查文件命名 (cam_yyyyMMdd_HHmmss_zzz.jpg)

3. **录制功能**
   - 录制视频（MP4）
   - 检查录制时长更新
   - 验证停止录制后文件完整性

4. **分辨率切换**
   - 列出支持的分辨率
   - 切换到不同分辨率
   - 验证预览和录制质量

## 后续改进方向

1. **重新启用 OpenCV**
   - 使用 MinGW 编译的 OpenCV
   - 或切换到 MSVC 编译器

2. **图像调整**
   - 使用 QVideoSink 访问原始帧
   - 通过 OpenCV 或自定义着色器实现调整

3. **高级功能**
   - 自动对焦控制
   - 白平衡调整
   - 曝光锁定

## 文件清单

- `include/CameraService.h` - 头文件，新增分辨率和帧率API
- `src/CameraService.cpp` - Qt6 实现
- `src/CameraService.cpp.qt5backup` - Qt5 原始实现备份
