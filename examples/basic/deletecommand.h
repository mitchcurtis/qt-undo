#ifndef DELETECOMMAND_H
#define DELETECOMMAND_H

#include <QtQuick/QQuickItem>
#include <QtUndo/undocommand.h>

class DeleteCommand : public UndoCommand
{
public:
    DeleteCommand(QQuickItem *itemParent, QQuickItem *item);

    void undo() override;
    void redo() override;

private:
    QQuickItem *mItemParent;
    QQuickItem *mItem;
};

#endif // DELETECOMMAND_H
