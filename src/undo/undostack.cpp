#include "undostack.h"

#include <QtCore/private/qobject_p.h>

#include "undocommand.h"

QT_BEGIN_NAMESPACE

class UndoStackPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(UndoStack)
public:
    UndoStackPrivate() {}

    QString text;
    QVector<UndoCommand*> commands;
};

UndoStack::UndoStack(QObject *parent) :
    QObject(*(new UndoStackPrivate), parent)
{
}

bool UndoStack::canUndo() const
{
    // TODO
    return false;
}

bool UndoStack::canRedo() const
{
    // TODO
    return false;
}

const UndoCommand *UndoStack::command(int index) const
{
    Q_D(const UndoStack);
    if (index < 0 || index >= d->commands.count())
        return nullptr;

    return d->commands.at(index);
}

QT_END_NAMESPACE
