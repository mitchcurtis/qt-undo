#ifndef UNDOCOMMAND_H
#define UNDOCOMMAND_H

#include "undo_global.h"

#include <QObject>

class UNDOSHARED_EXPORT UndoCommand : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(UndoCommand)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    UndoCommand(QObject *parent = nullptr);

    QString text() const;
    void setText(const QString &text);

    virtual void undo();
    virtual void redo();

Q_SIGNALS:
    void textChanged();

private:
    Q_DISABLE_COPY(UndoCommand)
};

#endif // UNDOCOMMAND_H
