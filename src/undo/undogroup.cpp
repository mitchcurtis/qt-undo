#include "undogroup.h"

#include <QtCore/private/qobject_p.h>

#include "undostack.h"
#include "undostack_p.h"

QT_BEGIN_NAMESPACE

class UndoGroupPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(UndoGroup)
public:
    UndoGroupPrivate() : active(0) {}

    UndoStack *active;
    QVector<UndoStack*> stacks;
};

/*!
    \class UndoGroup
    \brief The UndoGroup class is a group of UndoStack objects.
    \since 5.7
    \inmodule QtWidgets

    For an overview of the Qt's undo framework, see the
    \l{qundo.html}{overview}.

    An application often has multiple undo stacks, one for each opened document. At the
    same time, an application usually has one undo action and one redo action, which
    triggers an undo or redo in the active document.

    UndoGroup is a group of UndoStack objects, one of which may be active. It has
    an undo() and redo() slot, which calls UndoStack::undo() and UndoStack::redo()
    for the active stack. It also has the functions createUndoAction() and createRedoAction().
    The actions returned by these functions behave in the same way as those returned by
    UndoStack::createUndoAction() and UndoStack::createRedoAction() of the active
    stack.

    Stacks are added to a group with addStack() and removed with removeStack(). A stack
    is implicitly added to a group when it is created with the group as its parent
    QObject.

    It is the programmer's responsibility to specify which stack is active by
    calling UndoStack::setActive(), usually when the associated document window receives focus.
    The active stack may also be set with setActiveStack(), and is returned by activeStack().

    When a stack is added to a group using addStack(), the group does not take ownership
    of the stack. This means the stack has to be deleted separately from the group. When
    a stack is deleted, it is automatically removed from a group. A stack may belong to
    only one group. Adding it to another group will cause it to be removed from the previous
    group.
*/

/*!
    Creates an empty UndoGroup object with parent \a parent.

    \sa addStack()
*/

UndoGroup::UndoGroup(QObject *parent)
    : QObject(*new UndoGroupPrivate(), parent)
{
}

/*!
    Destroys the UndoGroup.
*/
UndoGroup::~UndoGroup()
{
    // Ensure all UndoStacks no longer refer to this group.
    Q_D(UndoGroup);
    QVector<UndoStack *>::iterator it = d->stacks.begin();
    QVector<UndoStack *>::iterator end = d->stacks.end();
    while (it != end) {
        (*it)->d_func()->group = 0;
        ++it;
    }
}

/*!
    Adds \a stack to this group. The group does not take ownership of the stack. Another
    way of adding a stack to a group is by specifying the group as the stack's parent
    QObject in UndoStack::UndoStack(). In this case, the stack is deleted when the
    group is deleted, in the usual manner of QObjects.

    \sa removeStack(), stacks(), UndoStack::UndoStack()
*/

void UndoGroup::addStack(UndoStack *stack)
{
    Q_D(UndoGroup);

    if (d->stacks.contains(stack))
        return;
    d->stacks.append(stack);

    if (UndoGroup *other = stack->d_func()->group)
        other->removeStack(stack);
    stack->d_func()->group = this;
}

/*!
    Removes \a stack from this group. If the stack was the active stack in the group,
    the active stack becomes null.

    \sa addStack(), stacks(), UndoStack::~UndoStack()
*/

void UndoGroup::removeStack(UndoStack *stack)
{
    Q_D(UndoGroup);

    if (d->stacks.removeAll(stack) == 0)
        return;
    if (stack == d->active)
        setActiveStack(0);
    stack->d_func()->group = 0;
}

/*!
    Returns a list of stacks in this group.

    \sa addStack(), removeStack()
*/

QVector<UndoStack*> UndoGroup::stacks() const
{
    Q_D(const UndoGroup);
    return d->stacks;
}

/*!
    Sets the active stack of this group to \a stack.

    If the stack is not a member of this group, this function does nothing.

    Equivalent to calling UndoStack::setActive() on \a stack.

    The actions returned by createUndoAction() and createRedoAction() will now behave
    in the same way as those returned by \a stack's UndoStack::createUndoAction()
    and UndoStack::createRedoAction().

    \sa UndoStack::setActive(), activeStack()
*/

void UndoGroup::setActiveStack(UndoStack *stack)
{
    Q_D(UndoGroup);
    if (d->active == stack)
        return;

    if (d->active != 0) {
        disconnect(d->active, SIGNAL(canUndoChanged(bool)),
                    this, SIGNAL(canUndoChanged(bool)));
        disconnect(d->active, SIGNAL(undoTextChanged(QString)),
                    this, SIGNAL(undoTextChanged(QString)));
        disconnect(d->active, SIGNAL(canRedoChanged(bool)),
                    this, SIGNAL(canRedoChanged(bool)));
        disconnect(d->active, SIGNAL(redoTextChanged(QString)),
                    this, SIGNAL(redoTextChanged(QString)));
        disconnect(d->active, SIGNAL(indexChanged(int)),
                    this, SIGNAL(indexChanged(int)));
        disconnect(d->active, SIGNAL(cleanChanged(bool)),
                    this, SIGNAL(cleanChanged(bool)));
    }

    d->active = stack;

    if (d->active == 0) {
        emit canUndoChanged(false);
        emit undoTextChanged(QString());
        emit canRedoChanged(false);
        emit redoTextChanged(QString());
        emit cleanChanged(true);
        emit indexChanged(0);
    } else {
        connect(d->active, SIGNAL(canUndoChanged(bool)),
                this, SIGNAL(canUndoChanged(bool)));
        connect(d->active, SIGNAL(undoTextChanged(QString)),
                this, SIGNAL(undoTextChanged(QString)));
        connect(d->active, SIGNAL(canRedoChanged(bool)),
                this, SIGNAL(canRedoChanged(bool)));
        connect(d->active, SIGNAL(redoTextChanged(QString)),
                this, SIGNAL(redoTextChanged(QString)));
        connect(d->active, SIGNAL(indexChanged(int)),
                this, SIGNAL(indexChanged(int)));
        connect(d->active, SIGNAL(cleanChanged(bool)),
                this, SIGNAL(cleanChanged(bool)));
        emit canUndoChanged(d->active->canUndo());
        emit undoTextChanged(d->active->undoText());
        emit canRedoChanged(d->active->canRedo());
        emit redoTextChanged(d->active->redoText());
        emit cleanChanged(d->active->isClean());
        emit indexChanged(d->active->index());
    }

    emit activeStackChanged(d->active);
}

/*!
    Returns the active stack of this group.

    If none of the stacks are active, or if the group is empty, this function
    returns 0.

    \sa setActiveStack(), UndoStack::setActive()
*/

UndoStack *UndoGroup::activeStack() const
{
    Q_D(const UndoGroup);
    return d->active;
}

/*!
    Calls UndoStack::undo() on the active stack.

    If none of the stacks are active, or if the group is empty, this function
    does nothing.

    \sa redo(), canUndo(), setActiveStack()
*/

void UndoGroup::undo()
{
    Q_D(UndoGroup);
    if (d->active != 0)
        d->active->undo();
}

/*!
    Calls UndoStack::redo() on the active stack.

    If none of the stacks are active, or if the group is empty, this function
    does nothing.

    \sa undo(), canRedo(), setActiveStack()
*/


void UndoGroup::redo()
{
    Q_D(UndoGroup);
    if (d->active != 0)
        d->active->redo();
}

/*!
    Returns the value of the active stack's UndoStack::canUndo().

    If none of the stacks are active, or if the group is empty, this function
    returns \c false.

    \sa canRedo(), setActiveStack()
*/

bool UndoGroup::canUndo() const
{
    Q_D(const UndoGroup);
    return d->active != 0 && d->active->canUndo();
}

/*!
    Returns the value of the active stack's UndoStack::canRedo().

    If none of the stacks are active, or if the group is empty, this function
    returns \c false.

    \sa canUndo(), setActiveStack()
*/

bool UndoGroup::canRedo() const
{
    Q_D(const UndoGroup);
    return d->active != 0 && d->active->canRedo();
}

/*!
    Returns the value of the active stack's UndoStack::undoText().

    If none of the stacks are active, or if the group is empty, this function
    returns an empty string.

    \sa redoText(), setActiveStack()
*/

QString UndoGroup::undoText() const
{
    Q_D(const UndoGroup);
    return d->active == 0 ? QString() : d->active->undoText();
}

/*!
    Returns the value of the active stack's UndoStack::redoText().

    If none of the stacks are active, or if the group is empty, this function
    returns an empty string.

    \sa undoText(), setActiveStack()
*/

QString UndoGroup::redoText() const
{
    Q_D(const UndoGroup);
    return d->active == 0 ? QString() : d->active->redoText();
}

/*!
    Returns the value of the active stack's UndoStack::isClean().

    If none of the stacks are active, or if the group is empty, this function
    returns \c true.

    \sa setActiveStack()
*/

bool UndoGroup::isClean() const
{
    Q_D(const UndoGroup);
    return d->active == 0 || d->active->isClean();
}

/*! \fn void UndoGroup::activeStackChanged(UndoStack *stack)

    This signal is emitted whenever the active stack of the group changes. This can happen
    when setActiveStack() or UndoStack::setActive() is called, or when the active stack
    is removed form the group. \a stack is the new active stack. If no stack is active,
    \a stack is 0.

    \sa setActiveStack(), UndoStack::setActive()
*/

/*! \fn void UndoGroup::indexChanged(int idx)

    This signal is emitted whenever the active stack emits UndoStack::indexChanged()
    or the active stack changes.

    \a idx is the new current index, or 0 if the active stack is 0.

    \sa UndoStack::indexChanged(), setActiveStack()
*/

/*! \fn void UndoGroup::cleanChanged(bool clean)

    This signal is emitted whenever the active stack emits UndoStack::cleanChanged()
    or the active stack changes.

    \a clean is the new state, or true if the active stack is 0.

    \sa UndoStack::cleanChanged(), setActiveStack()
*/

/*! \fn void UndoGroup::canUndoChanged(bool canUndo)

    This signal is emitted whenever the active stack emits UndoStack::canUndoChanged()
    or the active stack changes.

    \a canUndo is the new state, or false if the active stack is 0.

    \sa UndoStack::canUndoChanged(), setActiveStack()
*/

/*! \fn void UndoGroup::canRedoChanged(bool canRedo)

    This signal is emitted whenever the active stack emits UndoStack::canRedoChanged()
    or the active stack changes.

    \a canRedo is the new state, or false if the active stack is 0.

    \sa UndoStack::canRedoChanged(), setActiveStack()
*/

/*! \fn void UndoGroup::undoTextChanged(const QString &undoText)

    This signal is emitted whenever the active stack emits UndoStack::undoTextChanged()
    or the active stack changes.

    \a undoText is the new state, or an empty string if the active stack is 0.

    \sa UndoStack::undoTextChanged(), setActiveStack()
*/

/*! \fn void UndoGroup::redoTextChanged(const QString &redoText)

    This signal is emitted whenever the active stack emits UndoStack::redoTextChanged()
    or the active stack changes.

    \a redoText is the new state, or an empty string if the active stack is 0.

    \sa UndoStack::redoTextChanged(), setActiveStack()
*/

QT_END_NAMESPACE
