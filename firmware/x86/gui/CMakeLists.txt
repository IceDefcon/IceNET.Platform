cmake_minimum_required(VERSION 3.10)
project(mainControl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network)

find_package(OpenCV REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)

set(SOURCES
    main.cpp
    mainCtrl.cpp
    kernel/KernelCtrl.cpp
    stream/StreamReceiver.cpp
)

set(HEADERS
    mainCtrl.h
    kernel/KernelCtrl.h
    stream/StreamReceiver.h
)

add_executable(mainControl ${SOURCES} ${HEADERS})

target_include_directories(mainControl PRIVATE
    kernel
    stream
    ${OpenCV_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
)

target_link_libraries(mainControl PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    ${OpenCV_LIBS}
    ${GSTREAMER_LIBRARIES}
)
