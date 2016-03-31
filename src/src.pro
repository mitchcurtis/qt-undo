QT -= gui

TARGET = undo
TEMPLATE = lib

DEFINES += UNDO_LIBRARY

SOURCES += undocommand.cpp

HEADERS += undocommand.h\
        undo_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
