#ifndef CUSTOMUNDOSTACK_H
#define CUSTOMUNDOSTACK_H

#include <QObject>
#include <QtUndo/undostack.h>
#include <QQmlComponent>
#include <QQuickItem>

class CustomUndoStack : public UndoStack
{
    Q_OBJECT

public:
    CustomUndoStack();

    Q_INVOKABLE void addItem(QQuickItem *itemParent, QQmlComponent *component);

private:
    int mItemsAdded;
};

#endif // CUSTOMUNDOSTACK_H
