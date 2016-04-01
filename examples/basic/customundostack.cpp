#include "customundostack.h"

#include "addcommand.h"
#include "deletecommand.h"

CustomUndoStack::CustomUndoStack() :
    mItemsAdded(0)
{
}

void CustomUndoStack::addItem(QQuickItem *itemParent, QQmlComponent *itemComponent)
{
    if (!itemParent || !itemComponent)
        return;

    QQuickItem *item = qobject_cast<QQuickItem*>(itemComponent->create());
    if (!item) {
        qWarning() << "Failed to create item";
        return;
    }

    item->setX(mItemsAdded * 25);
    item->setY(mItemsAdded * 25);
    push(new AddCommand(itemParent, item));
    ++mItemsAdded;
}
