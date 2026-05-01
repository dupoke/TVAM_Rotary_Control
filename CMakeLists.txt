cmake_minimum_required(VERSION 3.16)
project(RotaryTableControl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/ui")

if(MSVC)
    # Conda Qt5 ships release runtime libraries only; use /MD in all configs
    # to avoid cross-CRT heap assertions in Debug.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets SerialPort Multimedia MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialPort Multimedia MultimediaWidgets)
set(_LOCAL_OPENCV_HINTS
    "${CMAKE_SOURCE_DIR}/third_party/opencv/build/x64/vc16/lib"
    "${CMAKE_SOURCE_DIR}/third_party/opencv/build"
)
foreach(_hint IN LISTS _LOCAL_OPENCV_HINTS)
    if(EXISTS "${_hint}")
        list(APPEND CMAKE_PREFIX_PATH "${_hint}")
        set(OpenCV_DIR "${_hint}" CACHE PATH "OpenCV config path" FORCE)
        break()
    endif()
endforeach()
find_package(OpenCV QUIET)

set(QT_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)

add_executable(RotaryTableControl WIN32
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
    src/CameraService.cpp
    include/MainWindow.h
    include/ConfigManager.h
    include/LogService.h
    include/BoardAdapter.h
    include/MotionService.h
    include/ProjectorService.h
    include/ImageSequenceService.h
    include/SyncPlaybackService.h
    include/ProjectionWindow.h
    include/CameraService.h
    ui/MainWindow.ui
)

target_include_directories(RotaryTableControl PRIVATE include)
target_link_libraries(RotaryTableControl PRIVATE ${QT_LIBS})

if(OpenCV_FOUND)
    target_include_directories(RotaryTableControl PRIVATE ${OpenCV_INCLUDE_DIRS})

    set(_opencv_linked FALSE)
    if(MSVC)
        file(GLOB _opencv_world_release_libs "${OpenCV_DIR}/opencv_world[0-9]*.lib")
        list(FILTER _opencv_world_release_libs EXCLUDE REGEX ".*d\\.lib$")
        if(_opencv_world_release_libs)
            list(GET _opencv_world_release_libs 0 _opencv_world_release_lib)
            target_link_libraries(RotaryTableControl PRIVATE "${_opencv_world_release_lib}")
            message(STATUS "OpenCV world release lib (all configs): ${_opencv_world_release_lib}")
            set(_opencv_linked TRUE)
        endif()
    endif()
    if(NOT _opencv_linked)
        target_link_libraries(RotaryTableControl PRIVATE ${OpenCV_LIBS})
    endif()

    target_compile_definitions(RotaryTableControl PRIVATE RTC_HAS_OPENCV=1)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")

    if(WIN32)
        set(_opencv_dll_candidates
            "${CMAKE_SOURCE_DIR}/third_party/opencv/build/x64/vc16/bin/opencv_world[0-9]*.dll"
            "${CMAKE_SOURCE_DIR}/third_party/opencv/build/bin/opencv_world[0-9]*.dll"
        )
        set(_opencv_runtime_dlls)
        foreach(_glob IN LISTS _opencv_dll_candidates)
            file(GLOB _found_dlls "${_glob}")
            if(_found_dlls)
                list(FILTER _found_dlls EXCLUDE REGEX ".*d\\.dll$")
                list(APPEND _opencv_runtime_dlls ${_found_dlls})
            endif()
        endforeach()
        list(REMOVE_DUPLICATES _opencv_runtime_dlls)
        foreach(_dll IN LISTS _opencv_runtime_dlls)
            add_custom_command(TARGET RotaryTableControl POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_dll}"
                    "$<TARGET_FILE_DIR:RotaryTableControl>"
                VERBATIM
            )
        endforeach()
    endif()
else()
    message(STATUS "OpenCV not found, fallback recorder disabled.")
endif()

# Bundle board runtime DLL into executable folder (no runtime dependency on ref/).
if(WIN32)
    set(BOARD_GAS_DLL "${CMAKE_SOURCE_DIR}/third_party/board/x64/GAS.dll")
    if(EXISTS "${BOARD_GAS_DLL}")
        add_custom_command(TARGET RotaryTableControl POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${BOARD_GAS_DLL}"
                "$<TARGET_FILE_DIR:RotaryTableControl>/GAS.dll"
            VERBATIM
        )
    endif()
endif()

# Keep source/diagnostics encoding consistent for Chinese UI text on MSVC.
if(MSVC)
    target_compile_options(RotaryTableControl PRIVATE /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(RotaryTableControl PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
