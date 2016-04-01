#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <QObject>

#include <QtUndo/undo_global.h>

QT_BEGIN_NAMESPACE

class UndoCommand;

class UndoStackPrivate;

class Q_UNDO_EXPORT UndoStack : public QObject
{
    Q_OBJECT

public:
    explicit UndoStack(QObject *parent = 0);

    bool canUndo() const;
    bool canRedo() const;
    QString undoText() const;
    QString redoText() const;

    void setUndoLimit(int limit);
    int undoLimit() const;

    const UndoCommand *command(int index) const;

public Q_SLOTS:
    void setClean();
    void setIndex(int idx);
    void undo();
    void redo();
    void setActive(bool active = true);

Q_SIGNALS:
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void undoTextChanged(const QString &undoText);
    void redoTextChanged(const QString &redoText);

private:
    Q_DISABLE_COPY(UndoStack)
    Q_DECLARE_PRIVATE(UndoStack)
};

QT_END_NAMESPACE

#endif // UNDOSTACK_H
