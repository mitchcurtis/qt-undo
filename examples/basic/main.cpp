#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "customundostack.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    CustomUndoStack undoStack;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("undoStack", &undoStack);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
