#include "undocommand.h"

class UndoCommandPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(UndoCommand)
public:
    UndoCommandPrivate() {}

    QString text;
    QVector<UndoCommand*> childCommands;
};

UndoCommand::UndoCommand(QObject *parent) :
    QObjectPrivate(new UndoCommandPrivate, parent)
{
}

QString UndoCommand::text() const
{
    Q_D(const UndoCommand);
    return d->text;
}

void UndoCommand::setText(const QString &text)
{
    Q_D(UndoCommand);
    if (text == d->text)
        return;

    d->text = text;
    emit textChanged();
}


void UndoCommand::redo()
{
    Q_D(UndoCommand);
    for (int i = 0; i < d->childCommands.size(); ++i)
        d->childCommands.at(i)->redo();
}

void UndoCommand::undo()
{
    Q_D(UndoCommand);
    for (int i = d->child_list.childCommands() - 1; i >= 0; --i)
        d->childCommands.at(i)->undo();
}
