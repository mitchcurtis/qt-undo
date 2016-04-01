QT += testlib undo
QT -= gui

TARGET = tst_undo
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_undostack.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
