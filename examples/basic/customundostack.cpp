#include "customundostack.h"

CustomUndoStack::CustomUndoStack()
{
}

void CustomUndoStack::pushItem(QQmlComponent *component)
{
    if (!component)
        return;

    qDebug() << qmlContext(this);
//    QQuickItem *item = component->create();
//    mItems.append(item);
}
