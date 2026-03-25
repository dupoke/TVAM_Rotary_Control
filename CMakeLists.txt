cmake_minimum_required(VERSION 3.16)
project(RotaryTableControl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialPort)

set(QT_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::SerialPort
)

add_executable(RotaryTableControl
    src/main.cpp
    src/MainWindow.cpp
    src/ConfigManager.cpp
    src/LogService.cpp
    src/BoardAdapter.cpp
    src/MotionService.cpp
    src/ProjectorService.cpp
    src/ImageSequenceService.cpp
    src/SyncPlaybackService.cpp
    src/ProjectionWindow.cpp
    include/MainWindow.h
    include/ConfigManager.h
    include/LogService.h
    include/BoardAdapter.h
    include/MotionService.h
    include/ProjectorService.h
    include/ImageSequenceService.h
    include/SyncPlaybackService.h
    include/ProjectionWindow.h
)

target_include_directories(RotaryTableControl PRIVATE include)
target_link_libraries(RotaryTableControl PRIVATE ${QT_LIBS})

# Keep source/diagnostics encoding consistent for Chinese UI text on MSVC.
if(MSVC)
    target_compile_options(RotaryTableControl PRIVATE /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(RotaryTableControl PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
