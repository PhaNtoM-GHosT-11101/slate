QT += core gui qml quick quickcontrols2

CONFIG += c++17 release
TARGET = slate
TEMPLATE = app

HEADERS += \
    src/backend.h \
    src/bindings.h \
    src/daemon.h \
    src/device.h \
    src/keymap.h \
    src/keymonitor.h \
    src/mouseinjector.h \
    src/penmonitor.h \
    src/settings.h \
    src/theme.h \
    src/translator.h

SOURCES += \
    src/main.cpp \
    src/backend.cpp \
    src/bindings.cpp \
    src/daemon.cpp \
    src/device.cpp \
    src/keymap.cpp \
    src/keymonitor.cpp \
    src/mouseinjector.cpp \
    src/penmonitor.cpp \
    src/settings.cpp \
    src/theme.cpp \
    src/translator.cpp

RESOURCES += src/resources.qrc
