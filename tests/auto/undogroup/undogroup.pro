QT += testlib undo
QT -= gui

TARGET = tst_undogroup
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_undogroup.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
