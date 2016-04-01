#ifndef UNDOGROUP_H
#define UNDOGROUP_H

#include <QObject>
#include <QtUndo/undo_global.h>

QT_BEGIN_NAMESPACE

class UndoGroupPrivate;
class UndoStack;

class Q_UNDO_EXPORT UndoGroup : public QObject
{
    Q_OBJECT
public:
    explicit UndoGroup(QObject *parent = 0);
    ~UndoGroup();

    void addStack(UndoStack *stack);
    void removeStack(UndoStack *stack);
    QVector<UndoStack*> stacks() const;
    UndoStack *activeStack() const;

    bool canUndo() const;
    bool canRedo() const;
    QString undoText() const;
    QString redoText() const;
    bool isClean() const;

public Q_SLOTS:
    void undo();
    void redo();
    void setActiveStack(UndoStack *stack);

Q_SIGNALS:
    void activeStackChanged(UndoStack *stack);
    void indexChanged(int idx);
    void cleanChanged(bool clean);
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void undoTextChanged(const QString &undoText);
    void redoTextChanged(const QString &redoText);

private:
    Q_DISABLE_COPY(UndoGroup)
    Q_DECLARE_PRIVATE(UndoGroup)
};

QT_END_NAMESPACE

#endif // UNDOGROUP_H
