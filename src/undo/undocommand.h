#ifndef UNDOCOMMAND_H
#define UNDOCOMMAND_H

#include <QtUndo/undo_global.h>

#include <QObject>

QT_BEGIN_NAMESPACE

class UndoCommandPrivate;

class Q_UNDO_EXPORT UndoCommand : public QObject
{
    Q_OBJECT
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
    Q_DECLARE_PRIVATE(UndoCommand)
};

QT_END_NAMESPACE

#endif // UNDOCOMMAND_H
