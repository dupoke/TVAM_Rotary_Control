# TVAM 旋转控制软件

基于 Qt Widgets 的 Windows 桌面程序，用于旋转台运动控制、同步投影和相机操作。

## 编译

需要 Windows、CMake、MSVC 和 Qt（Widgets、SerialPort、Multimedia、MultimediaWidgets）。

```powershell
cmake -S . -B build
cmake --build build --config Release
```

便携版打包：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package_win_portable.ps1
```
