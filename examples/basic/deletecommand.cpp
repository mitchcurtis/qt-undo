#include "deletecommand.h"

DeleteCommand::DeleteCommand(QQuickItem *itemParent, QQuickItem *item) :
    UndoCommand(nullptr),
    mItemParent(itemParent),
    mItem(item)
{
}

void DeleteCommand::undo()
{
    mItem->setParentItem(mItemParent);
}

void DeleteCommand::redo()
{
    mItem->setParentItem(nullptr);
}
