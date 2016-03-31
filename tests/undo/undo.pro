QT += testlib

QT -= gui

TARGET = tst_undo
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += tst_undo.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
