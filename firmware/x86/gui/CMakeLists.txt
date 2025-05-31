cmake_minimum_required(VERSION 3.10)
project(receiver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt Meta-Object Compiler
set(CMAKE_AUTOMOC ON)

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Find GStreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)

include_directories(
    ${GSTREAMER_INCLUDE_DIRS}
)

link_directories(
    ${GSTREAMER_LIBRARY_DIRS}
)

# Add source files
set(SOURCES
    main.cpp
    StreamReceiver.cpp
    mainCtrl.cpp         # <-- Added your GUI implementation file
)

set(HEADERS
    StreamReceiver.h
    mainCtrl.h           # <-- Added your GUI header
)

add_executable(receiver ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(receiver
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    ${OpenCV_LIBS}
    ${GSTREAMER_LIBRARIES}
)
