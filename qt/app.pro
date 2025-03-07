QT += core gui widgets

TARGET = app
TEMPLATE = app

SOURCES += main.cpp \
           gui/Gui.cpp \
           worker/worker.cpp

HEADERS += gui/Gui.h \
           worker/worker.h
