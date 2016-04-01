#ifndef CUSTOMUNDOSTACK_H
#define CUSTOMUNDOSTACK_H

#include <QObject>
#include <QtUndo/undostack.h>
#include <QQmlComponent>
#include <QQuickItem>

class CustomUndoStack : public UndoStack
{
public:
    CustomUndoStack();

    Q_INVOKABLE void pushItem(QQmlComponent *component);

private:
    QVector<QQuickItem *> mItems;
};

#endif // CUSTOMUNDOSTACK_H
