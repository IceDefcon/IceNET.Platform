cmake_minimum_required(VERSION 3.10)
project(app LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Ninja-friendly flags
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)

# Qt Setup
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets SerialPort)

# Source files
set(SOURCES
    main.cpp
    gui/gui.cpp
    meas/Measure.cpp
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

# Link Qt
target_link_libraries(app
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::SerialPort
    Threads::Threads
)
