QT += core gui widgets

TARGET = app
TEMPLATE = app

SOURCES += main.cpp \
           gui/gui.cpp \
           system/src/DroneCtrl.cpp \
           system/src/KernelComms/KernelComms.cpp \
           system/src/KernelComms/BlockDevice/RamDisk.cpp \
           system/src/KernelComms/CharDevice/Commander.cpp \
           system/src/KernelComms/CharDevice/Watchdog.cpp \

HEADERS += gui/gui.h \
           system/include/Core.h \
           system/include/Types.h \
           system/include/DroneCtrl.h \
           system/include/KernelComms/KernelComms.h \
           system/include/KernelComms/BlockDevice/RamDisk.h \
           system/include/KernelComms/CharDevice/Commander.h \
           system/include/KernelComms/CharDevice/Watchdog.h \

INCLUDEPATH += gui \
               system/include \
               system/include/KernelComms \
               system/include/KernelComms/BlockDevice \
               system/include/KernelComms/CharDevice \
