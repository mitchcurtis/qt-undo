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

target.path = $$[QT_INSTALL_EXAMPLES]/qtundo/basic
INSTALLS += target
