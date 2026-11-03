# 在台式机上编译指南

## 修改总结

已完成以下代码修复：

###  1. 删除"回零结果"和"零点误差"功能 ✅
- 从 `include/MainWindow.h` 删除了声明
- 从 `src/MainWindow.cpp` 删除了所有引用
- 从 `ui/MainWindow.ui` 删除了控件

### 2. 异步相机操作 ✅
- 相机打开改为异步（QTimer::singleShot 50ms）
- 录像启动保持异步

### 3. 角度归一化 ✅
- 在 `positionChanged` 信号中添加 `std::fmod(cycleDeg, 360.0)` 归一化
- 添加了 `<cmath>` 头文件

### 4. LED状态检查 ✅
- 添加 `projectorLedOnBySoftware_` 标志
- 开始投影前检查：必须"光机开+LED关"
- 所有LED操作更新状态

### 5. Qt 6.8.3兼容性修复 ✅
- 修复了 `BoardAdapter.cpp` 中的函数指针转换

## 笔记本编译问题

笔记本上遇到字符串编码问题（中文弯引号导致编译错误），建议在台式机（之前成功编译的环境）上编译。

## 在台式机上的编译步骤

```powershell
# 1. 同步代码（Git或手动复制修改的文件）
git pull
# 或手动复制：
# - include/MainWindow.h
# - src/MainWindow.cpp
# - src/BoardAdapter.cpp
# - ui/MainWindow.ui

# 2. 清理build
Remove-Item build\CMakeCache.txt -Force
Remove-Item -Recurse build\CMakeFiles -Force

# 3. 重新编译
cmake --build build --config Release

# 4. 复制到portable包
Copy-Item -LiteralPath build\Release\RotaryTableControl.exe `
  -Destination dist\RotaryTableControl-win11-portable-20260429\RotaryTableControl.exe -Force

# 5. 生成zip（可选）
Compress-Archive -LiteralPath dist\RotaryTableControl-win11-portable-20260429 `
  -DestinationPath dist\RotaryTableControl-win11-portable-20260429.zip -Force
```

## 修改的文件列表

- `include/MainWindow.h` - 删除控件声明，添加LED状态标志
- `src/MainWindow.cpp` - 所有功能修复
- `src/BoardAdapter.cpp` - Qt 6.8兼容性修复
- `ui/MainWindow.ui` - 删除回零结果/零点误差控件
- `CHANGES_20260111.md` - 修改说明文档

## 测试重点

1. 角度显示是否在0-360°范围正确循环
2. 相机打开/录像是否流畅不卡顿
3. LED开着时是否阻止开始投影
4. 删除的"回零结果"/"零点误差"不再显示
