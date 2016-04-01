TARGET = QtUndo
MODULE = undo

QT += core-private
QT -= gui

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

DEFINES += UNDO_LIBRARY

HEADERS += undo_global.h \
    undocommand.h \
    undocommand_p.h \
    undostack.h \
    undostack_p.h \
    undogroup.h

SOURCES += undocommand.cpp \
    undostack.cpp \
    undogroup.cpp

load(qt_module)
