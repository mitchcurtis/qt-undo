#ifndef UNDOCOMMAND_H
#define UNDOCOMMAND_H

#include <QObject>
#include <QtUndo/undo_global.h>

QT_BEGIN_NAMESPACE

class UndoCommandPrivate;

class Q_UNDO_EXPORT UndoCommand : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit UndoCommand(UndoCommand *parent = nullptr);
    explicit UndoCommand(const QString &text, UndoCommand *parent = nullptr);
    virtual ~UndoCommand();

    virtual void undo();
    virtual void redo();

    QString text() const;
    QString actionText() const;
    void setText(const QString &text);

    virtual int id() const;
    virtual bool mergeWith(const UndoCommand *other);

    int childCount() const;
    const UndoCommand *child(int index) const;

Q_SIGNALS:
    void textChanged();

private:
    Q_DISABLE_COPY(UndoCommand)
    Q_DECLARE_PRIVATE(UndoCommand)
    friend class UndoStack;
};

QT_END_NAMESPACE

#endif // UNDOCOMMAND_H
