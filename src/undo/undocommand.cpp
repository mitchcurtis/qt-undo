#include "undocommand.h"

#include <QtCore/private/qobject_p.h>
#include "undocommand_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class UndoCommand
    \brief The UndoCommand class is the base class of all commands stored on a UndoStack.
    \since 5.7

    A UndoCommand represents a single editing action on a document; for example,
    inserting or deleting a block of text in a text editor. UndoCommand can apply
    a change to the document with redo() and undo the change with undo(). The
    implementations for these functions must be provided in a derived class.

    \snippet code/src_gui_util_qundostack.cpp 0

    A UndoCommand has an associated text(). This is a short string
    describing what the command does. It is used to update the text
    properties of the stack's undo and redo actions; see
    UndoStack::createUndoAction() and UndoStack::createRedoAction().

    UndoCommand objects are owned by the stack they were pushed on.
    UndoStack deletes a command if it has been undone and a new command is pushed. For example:

    \snippet code/src_gui_util_qundostack.cpp 1

    In effect, when a command is pushed, it becomes the top-most command
    on the stack.

    To support command compression, UndoCommand has an id() and the virtual function
    mergeWith(). These functions are used by UndoStack::push().

    To support command macros, a UndoCommand object can have any number of child
    commands. Undoing or redoing the parent command will cause the child
    commands to be undone or redone. A command can be assigned
    to a parent explicitly in the constructor. In this case, the command
    will be owned by the parent.

    The parent in this case is usually an empty command, in that it doesn't
    provide its own implementation of undo() and redo(). Instead, it uses
    the base implementations of these functions, which simply call undo() or
    redo() on all its children. The parent should, however, have a meaningful
    text().

    \snippet code/src_gui_util_qundostack.cpp 2

    Another way to create macros is to use the convenience functions
    UndoStack::beginMacro() and UndoStack::endMacro().

    \sa UndoStack
*/

/*!
    Constructs a UndoCommand object with the given \a parent and \a text.

    If \a parent is not null, this command is appended to parent's child list.
    The parent command then owns this command and will delete it in its
    destructor.

    \sa ~UndoCommand()
*/

UndoCommand::UndoCommand(const QString &text, UndoCommand *parent) :
    QObject(*(new UndoCommandPrivate), parent)
{
    if (parent != 0)
        parent->d_func()->childCommands.append(this);
    setText(text);
}

/*!
    Constructs a UndoCommand object with parent \a parent.

    If \a parent is not null, this command is appended to parent's child list.
    The parent command then owns this command and will delete it in its
    destructor.

    \sa ~UndoCommand()
*/

UndoCommand::UndoCommand(UndoCommand *parent) :
    QObject(*(new UndoCommandPrivate), parent)
{
    if (parent != 0)
        parent->d_func()->childCommands.append(this);
}

/*!
    Destroys the UndoCommand object and all child commands.

    \sa UndoCommand()
*/

UndoCommand::~UndoCommand()
{
    Q_D(UndoCommand);
    qDeleteAll(d->childCommands);
}

/*!
    Returns the ID of this command.

    A command ID is used in command compression. It must be an integer unique to
    this command's class, or -1 if the command doesn't support compression.

    If the command supports compression this function must be overridden in the
    derived class to return the correct ID. The base implementation returns -1.

    UndoStack::push() will only try to merge two commands if they have the
    same ID, and the ID is not -1.

    \sa mergeWith(), UndoStack::push()
*/

int UndoCommand::id() const
{
    return -1;
}

/*!
    Attempts to merge this command with \a command. Returns \c true on
    success; otherwise returns \c false.

    If this function returns \c true, calling this command's redo() must have the same
    effect as redoing both this command and \a command.
    Similarly, calling this command's undo() must have the same effect as undoing
    \a command and this command.

    UndoStack will only try to merge two commands if they have the same id, and
    the id is not -1.

    The default implementation returns \c false.

    \snippet code/src_gui_util_qundostack.cpp 3

    \sa id(), UndoStack::push()
*/

bool UndoCommand::mergeWith(const UndoCommand *command)
{
    Q_UNUSED(command);
    return false;
}

/*!
    Applies a change to the document. This function must be implemented in
    the derived class. Calling UndoStack::push(),
    UndoStack::undo() or UndoStack::redo() from this function leads to
    undefined beahavior.

    The default implementation calls redo() on all child commands.

    \sa undo()
*/

void UndoCommand::redo()
{
    Q_D(UndoCommand);
    for (int i = 0; i < d->childCommands.size(); ++i)
        d->childCommands.at(i)->redo();
}

/*!
    Reverts a change to the document. After undo() is called, the state of
    the document should be the same as before redo() was called. This function must
    be implemented in the derived class. Calling UndoStack::push(),
    UndoStack::undo() or UndoStack::redo() from this function leads to
    undefined beahavior.

    The default implementation calls undo() on all child commands in reverse order.

    \sa redo()
*/

void UndoCommand::undo()
{
    Q_D(UndoCommand);
    for (int i = d->childCommands.size() - 1; i >= 0; --i)
        d->childCommands.at(i)->undo();
}

/*!
    Returns a short text string describing what this command does; for example,
    "insert text".

    The text is used for names of items in QUndoView.

    \sa setText(), UndoStack::createUndoAction(), UndoStack::createRedoAction()
*/

QString UndoCommand::text() const
{
    Q_D(const UndoCommand);
    return d->text;
}

/*!
    Sets the command's text to \a text.

    The specified text should be a short user-readable string describing what this
    command does.

    \sa text(), actionText(), UndoStack::createUndoAction(), UndoStack::createRedoAction()
*/

void UndoCommand::setText(const QString &text)
{
    Q_D(UndoCommand);
    if (text == d->text)
        return;

    d->text = text;
    emit textChanged();
}

/*!
    \since 4.4

    Returns the number of child commands in this command.

    \sa child()
*/

int UndoCommand::childCount() const
{
    Q_D(const UndoCommand);
    return d->childCommands.count();
}

/*!
    \since 4.4

    Returns the child command at \a index.

    \sa childCount(), UndoStack::command()
*/

const UndoCommand *UndoCommand::child(int index) const
{
    Q_D(const UndoCommand);
    if (index < 0 || index >= d->childCommands.count())
        return 0;
    return d->childCommands.at(index);
}

QT_END_NAMESPACE
