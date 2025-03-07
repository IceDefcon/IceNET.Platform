QT += core gui widgets

TARGET = app
TEMPLATE = app

SOURCES += main.cpp \
           gui/gui.cpp \
           worker/worker.cpp \
           system/src/DroneCtrl.cpp \
           system/src/KernelComms/KernelComms.cpp \
           system/src/KernelComms/BlockDevice/RamDisk.cpp \
           system/src/KernelComms/CharDevice/Commander.cpp \
           system/src/KernelComms/CharDevice/Watchdog.cpp \
           system/src/Network/Network.cpp \
           system/src/Network/ServerTCP.cpp

HEADERS += gui/gui.h \
           worker/worker.h \
           system/include/Core.h \
           system/include/DroneCtrl.h \
           system/include/Types.h \
           system/include/KernelComms/KernelComms.h \
           system/include/KernelComms/BlockDevice/RamDisk.h \
           system/include/KernelComms/CharDevice/Commander.h \
           system/include/KernelComms/CharDevice/Watchdog.h \
           system/include/Network/Network.h \
           system/include/Network/ServerTCP.h \

INCLUDEPATH += gui \
               worker \
               system/include \
               system/include/KernelComms \
               system/include/KernelComms/BlockDevice \
               system/include/KernelComms/CharDevice \
               system/include/Network
