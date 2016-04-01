TEMPLATE = app

QT += qml quick undo
CONFIG += c++11

HEADERS += \
    customundostack.h \
    addcommand.h \
    deletecommand.h

SOURCES += main.cpp \
    customundostack.cpp \
    addcommand.cpp \
    deletecommand.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)
