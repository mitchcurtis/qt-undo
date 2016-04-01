#ifndef ADDCOMMAND_H
#define ADDCOMMAND_H

#include <QtQuick/QQuickItem>
#include <QtUndo/undocommand.h>

class AddCommand : public UndoCommand
{
public:
    AddCommand(QQuickItem *itemParent, QQuickItem *item);

    void undo() override;
    void redo() override;

private:
    QQuickItem *mItemParent;
    QQuickItem *mItem;
};

#endif // ADDCOMMAND_H
