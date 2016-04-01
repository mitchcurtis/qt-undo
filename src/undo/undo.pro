TARGET = QtUndo
MODULE = undo
CONFIG += internal_module

QT += core-private
QT -= gui

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

DEFINES += UNDO_LIBRARY

HEADERS += undocommand.h\
    undo_global.h \
    undostack.h

SOURCES += undocommand.cpp \
    undostack.cpp

load(qt_module)
