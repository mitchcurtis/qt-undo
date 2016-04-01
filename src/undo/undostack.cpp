#include "undostack.h"

#include <QtCore/private/qobject_p.h>

#include "undocommand.h"
#include "undocommand_p.h"
#include "undogroup.h"
#include "undostack_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class UndoStack
    \brief The UndoStack class is a stack of UndoCommand objects.
    \since 5.7

    TODO

    \sa UndoCommand
*/

/*! \internal
    Sets the current index to \a idx, emitting appropriate signals. If \a clean is true,
    makes \a idx the clean index as well.
*/

void UndoStackPrivate::setIndex(int idx, bool clean)
{
    Q_Q(UndoStack);

    bool wasClean = index == cleanIndex;

    if (idx != index) {
        index = idx;
        emit q->indexChanged(index);
        emit q->canUndoChanged(q->canUndo());
        emit q->undoTextChanged(q->undoText());
        emit q->canRedoChanged(q->canRedo());
        emit q->redoTextChanged(q->redoText());
    }

    if (clean)
        cleanIndex = index;

    bool isClean = index == cleanIndex;
    if (isClean != wasClean)
        emit q->cleanChanged(isClean);
}

/*! \internal
    If the number of commands on the stack exceedes the undo limit, deletes commands from
    the bottom of the stack.

    Returns \c true if commands were deleted.
*/

bool UndoStackPrivate::checkUndoLimit()
{
    if (undoLimit <= 0 || !macroStack.isEmpty() || undoLimit >= commandList.count())
        return false;

    int deletedCount = commandList.count() - undoLimit;

    for (int i = 0; i < deletedCount; ++i)
        delete commandList.takeFirst();

    index -= deletedCount;
    if (cleanIndex != -1) {
        if (cleanIndex < deletedCount)
            cleanIndex = -1; // we've deleted the clean command
        else
            cleanIndex -= deletedCount;
    }

    return true;
}

/*!
    Constructs an empty undo stack with the parent \a parent. The
    stack will initially be in the clean state. If \a parent is a
    UndoGroup object, the stack is automatically added to the group.

    \sa push()
*/

UndoStack::UndoStack(QObject *parent)
    : QObject(*(new UndoStackPrivate), parent)
{
    if (UndoGroup *group = qobject_cast<UndoGroup*>(parent))
        group->addStack(this);
}

/*!
    Destroys the undo stack, deleting any commands that are on it. If the
    stack is in a UndoGroup, the stack is automatically removed from the group.

    \sa UndoStack()
*/

UndoStack::~UndoStack()
{
    Q_D(UndoStack);
    if (d->group != 0)
        d->group->removeStack(this);
    clear();
}

/*!
    Clears the command stack by deleting all commands on it, and returns the stack
    to the clean state.

    Commands are not undone or redone; the state of the edited object remains
    unchanged.

    This function is usually used when the contents of the document are
    abandoned.

    \sa UndoStack()
*/

void UndoStack::clear()
{
    Q_D(UndoStack);

    if (d->commandList.isEmpty())
        return;

    bool wasClean = isClean();

    d->macroStack.clear();
    qDeleteAll(d->commandList);
    d->commandList.clear();

    d->index = 0;
    d->cleanIndex = 0;

    emit indexChanged(0);
    emit canUndoChanged(false);
    emit undoTextChanged(QString());
    emit canRedoChanged(false);
    emit redoTextChanged(QString());

    if (!wasClean)
        emit cleanChanged(true);
}

/*!
    Pushes \a cmd on the stack or merges it with the most recently executed command.
    In either case, executes \a cmd by calling its redo() function.

    If \a cmd's id is not -1, and if the id is the same as that of the
    most recently executed command, UndoStack will attempt to merge the two
    commands by calling UndoCommand::mergeWith() on the most recently executed
    command. If UndoCommand::mergeWith() returns \c true, \a cmd is deleted.

    In all other cases \a cmd is simply pushed on the stack.

    If commands were undone before \a cmd was pushed, the current command and
    all commands above it are deleted. Hence \a cmd always ends up being the
    top-most on the stack.

    Once a command is pushed, the stack takes ownership of it. There
    are no getters to return the command, since modifying it after it has
    been executed will almost always lead to corruption of the document's
    state.

    \sa UndoCommand::id(), UndoCommand::mergeWith()
*/

void UndoStack::push(UndoCommand *command)
{
    Q_D(UndoStack);
    command->redo();

    const bool macro = !d->macroStack.isEmpty();

    UndoCommand *currentCommand = 0;
    if (macro) {
        UndoCommand *macroCommand = d->macroStack.constLast();
        if (!macroCommand->d_func()->childCommands.isEmpty())
            currentCommand = macroCommand->d_func()->childCommands.constLast();
    } else {
        if (d->index > 0)
            currentCommand = d->commandList.at(d->index - 1);
        while (d->index < d->commandList.size())
            delete d->commandList.takeLast();
        if (d->cleanIndex > d->index)
            d->cleanIndex = -1; // we've deleted the clean state
    }

    bool tryMerge = currentCommand != 0
            && currentCommand->id() != -1
            && currentCommand->id() == command->id()
            && (macro || d->index != d->cleanIndex);

    if (tryMerge && currentCommand->mergeWith(command)) {
        delete command;
        if (!macro) {
            emit indexChanged(d->index);
            emit canUndoChanged(canUndo());
            emit undoTextChanged(undoText());
            emit canRedoChanged(canRedo());
            emit redoTextChanged(redoText());
        }
    } else {
        if (macro) {
            d->macroStack.constLast()->d_func()->childCommands.append(command);
        } else {
            d->commandList.append(command);
            d->checkUndoLimit();
            d->setIndex(d->index + 1, false);
        }
    }
}

/*!
    Marks the stack as clean and emits cleanChanged() if the stack was
    not already clean.

    Whenever the stack returns to this state through the use of undo/redo
    commands, it emits the signal cleanChanged(). This signal is also
    emitted when the stack leaves the clean state.

    \sa isClean(), cleanIndex()
*/

void UndoStack::setClean()
{
    Q_D(UndoStack);
    if (Q_UNLIKELY(!d->macroStack.isEmpty())) {
        qWarning("UndoStack::setClean(): cannot set clean in the middle of a macro");
        return;
    }

    d->setIndex(d->index, true);
}

/*!
    If the stack is in the clean state, returns \c true; otherwise returns \c false.

    \sa setClean(), cleanIndex()
*/

bool UndoStack::isClean() const
{
    Q_D(const UndoStack);
    if (!d->macroStack.isEmpty())
        return false;
    return d->cleanIndex == d->index;
}

/*!
    Returns the clean index. This is the index at which setClean() was called.

    A stack may not have a clean index. This happens if a document is saved,
    some commands are undone, then a new command is pushed. Since
    push() deletes all the undone commands before pushing the new command, the stack
    can't return to the clean state again. In this case, this function returns -1.

    \sa isClean(), setClean()
*/

int UndoStack::cleanIndex() const
{
    Q_D(const UndoStack);
    return d->cleanIndex;
}

/*!
    Undoes the command below the current command by calling UndoCommand::undo().
    Decrements the current command index.

    If the stack is empty, or if the bottom command on the stack has already been
    undone, this function does nothing.

    \sa redo(), index()
*/

void UndoStack::undo()
{
    Q_D(UndoStack);
    if (d->index == 0)
        return;

    if (Q_UNLIKELY(!d->macroStack.isEmpty())) {
        qWarning("UndoStack::undo(): cannot undo in the middle of a macro");
        return;
    }

    int idx = d->index - 1;
    d->commandList.at(idx)->undo();
    d->setIndex(idx, false);
}

/*!
    Redoes the current command by calling UndoCommand::redo(). Increments the current
    command index.

    If the stack is empty, or if the top command on the stack has already been
    redone, this function does nothing.

    \sa undo(), index()
*/

void UndoStack::redo()
{
    Q_D(UndoStack);
    if (d->index == d->commandList.size())
        return;

    if (Q_UNLIKELY(!d->macroStack.isEmpty())) {
        qWarning("UndoStack::redo(): cannot redo in the middle of a macro");
        return;
    }

    d->commandList.at(d->index)->redo();
    d->setIndex(d->index + 1, false);
}

/*!
    Returns the number of commands on the stack. Macro commands are counted as
    one command.

    \sa index(), setIndex(), command()
*/

int UndoStack::count() const
{
    Q_D(const UndoStack);
    return d->commandList.size();
}

/*!
    Returns the index of the current command. This is the command that will be
    executed on the next call to redo(). It is not always the top-most command
    on the stack, since a number of commands may have been undone.

    \sa undo(), redo(), count()
*/

int UndoStack::index() const
{
    Q_D(const UndoStack);
    return d->index;
}

/*!
    Repeatedly calls undo() or redo() until the current command index reaches
    \a idx. This function can be used to roll the state of the document forwards
    of backwards. indexChanged() is emitted only once.

    \sa index(), count(), undo(), redo()
*/

void UndoStack::setIndex(int idx)
{
    Q_D(UndoStack);
    if (Q_UNLIKELY(!d->macroStack.isEmpty())) {
        qWarning("UndoStack::setIndex(): cannot set index in the middle of a macro");
        return;
    }

    if (idx < 0)
        idx = 0;
    else if (idx > d->commandList.size())
        idx = d->commandList.size();

    int i = d->index;
    while (i < idx)
        d->commandList.at(i++)->redo();
    while (i > idx)
        d->commandList.at(--i)->undo();

    d->setIndex(idx, false);
}

/*!
    Returns \c true if there is a command available for undo; otherwise returns \c false.

    This function returns \c false if the stack is empty, or if the bottom command
    on the stack has already been undone.

    Synonymous with index() == 0.

    \sa index(), canRedo()
*/

bool UndoStack::canUndo() const
{
    Q_D(const UndoStack);
    if (!d->macroStack.isEmpty())
        return false;
    return d->index > 0;
}

/*!
    Returns \c true if there is a command available for redo; otherwise returns \c false.

    This function returns \c false if the stack is empty or if the top command
    on the stack has already been redone.

    Synonymous with index() == count().

    \sa index(), canUndo()
*/

bool UndoStack::canRedo() const
{
    Q_D(const UndoStack);
    if (!d->macroStack.isEmpty())
        return false;
    return d->index < d->commandList.size();
}

/*!
    Returns the text of the command which will be undone in the next call to undo().

    \sa UndoCommand::text(), redoText()
*/

QString UndoStack::undoText() const
{
    Q_D(const UndoStack);
    if (!d->macroStack.isEmpty())
        return QString();
    if (d->index > 0)
        return d->commandList.at(d->index - 1)->text();
    return QString();
}

/*!
    Returns the text of the command which will be redone in the next call to redo().

    \sa UndoCommand::text(), undoText()
*/

QString UndoStack::redoText() const
{
    Q_D(const UndoStack);
    if (!d->macroStack.isEmpty())
        return QString();
    if (d->index < d->commandList.size())
        return d->commandList.at(d->index)->text();
    return QString();
}

/*!
    Begins composition of a macro command with the given \a text description.

    An empty command described by the specified \a text is pushed on the stack.
    Any subsequent commands pushed on the stack will be appended to the empty
    command's children until endMacro() is called.

    Calls to beginMacro() and endMacro() may be nested, but every call to
    beginMacro() must have a matching call to endMacro().

    While a macro is composed, the stack is disabled. This means that:
    \list
    \li indexChanged() and cleanChanged() are not emitted,
    \li canUndo() and canRedo() return false,
    \li calling undo() or redo() has no effect,
    \li the undo/redo actions are disabled.
    \endlist

    The stack becomes enabled and appropriate signals are emitted when endMacro()
    is called for the outermost macro.

    \snippet code/src_gui_util_qundostack.cpp 4

    This code is equivalent to:

    \snippet code/src_gui_util_qundostack.cpp 5

    \sa endMacro()
*/

void UndoStack::beginMacro(const QString &text)
{
    Q_D(UndoStack);
    UndoCommand *command = new UndoCommand();
    command->setText(text);

    if (d->macroStack.isEmpty()) {
        while (d->index < d->commandList.size())
            delete d->commandList.takeLast();
        if (d->cleanIndex > d->index)
            d->cleanIndex = -1; // we've deleted the clean state
        d->commandList.append(command);
    } else {
        d->macroStack.constLast()->d_func()->childCommands.append(command);
    }
    d->macroStack.append(command);

    if (d->macroStack.count() == 1) {
        emit canUndoChanged(false);
        emit undoTextChanged(QString());
        emit canRedoChanged(false);
        emit redoTextChanged(QString());
    }
}

/*!
    Ends composition of a macro command.

    If this is the outermost macro in a set nested macros, this function emits
    indexChanged() once for the entire macro command.

    \sa beginMacro()
*/

void UndoStack::endMacro()
{
    Q_D(UndoStack);
    if (Q_UNLIKELY(d->macroStack.isEmpty())) {
        qWarning("UndoStack::endMacro(): no matching beginMacro()");
        return;
    }

    d->macroStack.removeLast();

    if (d->macroStack.isEmpty()) {
        d->checkUndoLimit();
        d->setIndex(d->index + 1, false);
    }
}

/*!
  \since 4.4

  Returns a const pointer to the command at \a index.

  This function returns a const pointer, because modifying a command,
  once it has been pushed onto the stack and executed, almost always
  causes corruption of the state of the document, if the command is
  later undone or redone.

  \sa UndoCommand::child()
*/
const UndoCommand *UndoStack::command(int index) const
{
    Q_D(const UndoStack);

    if (index < 0 || index >= d->commandList.count())
        return 0;
    return d->commandList.at(index);
}

/*!
    Returns the text of the command at index \a idx.

    \sa beginMacro()
*/

QString UndoStack::text(int idx) const
{
    Q_D(const UndoStack);

    if (idx < 0 || idx >= d->commandList.size())
        return QString();
    return d->commandList.at(idx)->text();
}

/*!
    \property UndoStack::undoLimit
    \brief the maximum number of commands on this stack.
    \since 4.3

    When the number of commands on a stack exceedes the stack's undoLimit, commands are
    deleted from the bottom of the stack. Macro commands (commands with child commands)
    are treated as one command. The default value is 0, which means that there is no
    limit.

    This property may only be set when the undo stack is empty, since setting it on a
    non-empty stack might delete the command at the current index. Calling setUndoLimit()
    on a non-empty stack prints a warning and does nothing.
*/

void UndoStack::setUndoLimit(int limit)
{
    Q_D(UndoStack);

    if (Q_UNLIKELY(!d->commandList.isEmpty())) {
        qWarning("UndoStack::setUndoLimit(): an undo limit can only be set when the stack is empty");
        return;
    }

    if (limit == d->undoLimit)
        return;
    d->undoLimit = limit;
    d->checkUndoLimit();
}

int UndoStack::undoLimit() const
{
    Q_D(const UndoStack);

    return d->undoLimit;
}

/*!
    \property UndoStack::active
    \brief the active status of this stack.

    An application often has multiple undo stacks, one for each opened document. The active
    stack is the one associated with the currently active document. If the stack belongs
    to a UndoGroup, calls to UndoGroup::undo() or UndoGroup::redo() will be forwarded
    to this stack when it is active. If the UndoGroup is watched by a QUndoView, the view
    will display the contents of this stack when it is active. If the stack does not belong to
    a UndoGroup, making it active has no effect.

    It is the programmer's responsibility to specify which stack is active by
    calling setActive(), usually when the associated document window receives focus.

    \sa UndoGroup
*/

void UndoStack::setActive(bool active)
{
#ifdef QT_NO_UNDOGROUP
    Q_UNUSED(active);
#else
    Q_D(UndoStack);

    if (d->group != 0) {
        if (active)
            d->group->setActiveStack(this);
        else if (d->group->activeStack() == this)
            d->group->setActiveStack(0);
    }
#endif
}

bool UndoStack::isActive() const
{
#ifdef QT_NO_UNDOGROUP
    return true;
#else
    Q_D(const UndoStack);
    return d->group == 0 || d->group->activeStack() == this;
#endif
}

/*!
    \fn void UndoStack::indexChanged(int idx)

    This signal is emitted whenever a command modifies the state of the document.
    This happens when a command is undone or redone. When a macro
    command is undone or redone, or setIndex() is called, this signal
    is emitted only once.

    \a idx specifies the index of the current command, ie. the command which will be
    executed on the next call to redo().

    \sa index(), setIndex()
*/

/*!
    \fn void UndoStack::cleanChanged(bool clean)

    This signal is emitted whenever the stack enters or leaves the clean state.
    If \a clean is true, the stack is in a clean state; otherwise this signal
    indicates that it has left the clean state.

    \sa isClean(), setClean()
*/

/*!
    \fn void UndoStack::undoTextChanged(const QString &undoText)

    This signal is emitted whenever the value of undoText() changes. It is
    used to update the text property of the undo action returned by createUndoAction().
    \a undoText specifies the new text.
*/

/*!
    \fn void UndoStack::canUndoChanged(bool canUndo)

    This signal is emitted whenever the value of canUndo() changes. It is
    used to enable or disable the undo action returned by createUndoAction().
    \a canUndo specifies the new value.
*/

/*!
    \fn void UndoStack::redoTextChanged(const QString &redoText)

    This signal is emitted whenever the value of redoText() changes. It is
    used to update the text property of the redo action returned by createRedoAction().
    \a redoText specifies the new text.
*/

/*!
    \fn void UndoStack::canRedoChanged(bool canRedo)

    This signal is emitted whenever the value of canRedo() changes. It is
    used to enable or disable the redo action returned by createRedoAction().
    \a canRedo specifies the new value.
*/

QT_END_NAMESPACE
