cmake_minimum_required(VERSION 3.10)
project(app LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Ninja-friendly flags
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)

# Qt Setup
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets SerialPort)

# OpenCV Setup
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Source files
set(SOURCES
    main.cpp
    gui/gui.cpp
    meas/Measure.cpp
    gui/peripherals.cpp
    gui/CameraWindow.cpp
    system/src/DroneCtrl.cpp
    system/src/KernelComms/KernelComms.cpp
    system/src/KernelComms/BlockDevice/RamDisk.cpp
    system/src/KernelComms/CharDevice/Commander.cpp
    system/src/KernelComms/CharDevice/Watchdog.cpp
)

# Header files (optional, but nice for IDEs)
set(HEADERS
    gui/gui.h
    meas/Measure.h
    gui/CameraWindow.h
    system/include/Core.h
    system/include/Types.h
    system/include/DroneCtrl.h
    system/include/KernelComms/KernelComms.h
    system/include/KernelComms/BlockDevice/RamDisk.h
    system/include/KernelComms/CharDevice/Commander.h
    system/include/KernelComms/CharDevice/Watchdog.h
)

# Include directories
include_directories(
    gui
    meas
    system/include
    system/include/KernelComms
    system/include/KernelComms/BlockDevice
    system/include/KernelComms/CharDevice
)

# Define the executable
add_executable(app ${SOURCES} ${HEADERS})

find_package(Threads REQUIRED)

# GStreamer setup using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=1.14 gstreamer-base-1.0)

# Include and link GStreamer
include_directories(${GSTREAMER_INCLUDE_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})
add_definitions(${GSTREAMER_CFLAGS_OTHER})

# Add GStreamer to your target
target_link_libraries(app
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::SerialPort
    Threads::Threads
    ${OpenCV_LIBS}
    ${GSTREAMER_LIBRARIES}
)
