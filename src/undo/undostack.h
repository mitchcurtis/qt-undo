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
    explicit UndoStack(QObject *parent = nullptr);
    ~UndoStack();

    void clear();
    void push(UndoCommand *command);

    bool canUndo() const;
    bool canRedo() const;
    QString undoText() const;
    QString redoText() const;

    int count() const;
    int index() const;
    QString text(int idx) const;

    bool isActive() const;
    bool isClean() const;
    int cleanIndex() const;

    void beginMacro(const QString &text);
    void endMacro();

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
    void indexChanged(int idx);
    void cleanChanged(bool clean);
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void undoTextChanged(const QString &undoText);
    void redoTextChanged(const QString &redoText);

private:
    Q_DISABLE_COPY(UndoStack)
    Q_DECLARE_PRIVATE(UndoStack)
    friend class UndoGroup;
};

QT_END_NAMESPACE

#endif // UNDOSTACK_H
