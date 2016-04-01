#include "addcommand.h"

AddCommand::AddCommand(QQuickItem *itemParent, QQuickItem *item) :
    UndoCommand(nullptr),
    mItemParent(itemParent),
    mItem(item)
{
    connect(mItemParent, &QObject::destroyed, this, &AddCommand::cleanUp);
}

AddCommand::~AddCommand()
{
    if (mItem)
        delete mItem;
}

void AddCommand::undo()
{
    mItem->setParentItem(nullptr);
}

void AddCommand::redo()
{
    mItem->setParentItem(mItemParent);
}

void AddCommand::cleanUp()
{
    mItemParent = nullptr;
    mItem = nullptr;
}
