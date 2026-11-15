QT += core gui testlib
CONFIG += c++17 console testcase
CONFIG -= app_bundle
TEMPLATE = app
TARGET = slate_tests

INCLUDEPATH += $$PWD/../src

HEADERS += \
    $$PWD/../src/bindings.h \
    $$PWD/../src/keymap.h \
    $$PWD/../src/settings.h \
    $$PWD/../src/theme.h

SOURCES += \
    test_main.cpp \
    $$PWD/../src/bindings.cpp \
    $$PWD/../src/keymap.cpp \
    $$PWD/../src/settings.cpp \
    $$PWD/../src/theme.cpp
