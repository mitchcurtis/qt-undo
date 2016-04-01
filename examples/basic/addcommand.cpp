#include "addcommand.h"

AddCommand::AddCommand(QQuickItem *itemParent, QQuickItem *item) :
    UndoCommand(nullptr),
    mItemParent(itemParent),
    mItem(item)
{
}

void AddCommand::undo()
{
    mItem->setParentItem(nullptr);
}

void AddCommand::redo()
{
    mItem->setParentItem(mItemParent);
}
