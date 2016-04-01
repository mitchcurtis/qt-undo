#include <QString>
#include <QtTest>
#include <QtUndo/undocommand.h>
#include <QtUndo/undogroup.h>
#include <QtUndo/undostack.h>

class InsertCommand : public UndoCommand
{
public:
    InsertCommand(QString *str, int idx, const QString &text, UndoCommand *parent = 0);

    virtual void undo();
    virtual void redo();

private:
    QString *m_str;
    int m_idx;
    QString m_text;
};

InsertCommand::InsertCommand(QString *str, int idx, const QString &text, UndoCommand *parent) :
    UndoCommand(parent),
    m_str(str),
    m_idx(idx),
    m_text(text)
{
    QVERIFY(str->length() >= idx);

    setText("insert");
}

void InsertCommand::redo()
{
    QVERIFY(m_str->length() >= m_idx);

    m_str->insert(m_idx, m_text);
}

void InsertCommand::undo()
{
    QCOMPARE(m_str->mid(m_idx, m_text.length()), m_text);

    m_str->remove(m_idx, m_text.length());
}

class RemoveCommand : public UndoCommand
{
public:
    RemoveCommand(QString *str, int idx, int len, UndoCommand *parent = 0);

    virtual void undo();
    virtual void redo();

private:
    QString *m_str;
    int m_idx;
    QString m_text;
};

RemoveCommand::RemoveCommand(QString *str, int idx, int len, UndoCommand *parent) :
    UndoCommand(parent),
    m_str(str),
    m_idx(idx),
    m_text(m_str->mid(m_idx, len))
{
    QVERIFY(str->length() >= idx + len);

    setText("remove");
}

void RemoveCommand::redo()
{
    QCOMPARE(m_str->mid(m_idx, m_text.length()), m_text);

    m_str->remove(m_idx, m_text.length());
}

void RemoveCommand::undo()
{
    QVERIFY(m_str->length() >= m_idx);

    m_str->insert(m_idx, m_text);
}

class AppendCommand : public UndoCommand
{
public:
    AppendCommand(QString *str, const QString &text, UndoCommand *parent = 0);

    virtual void undo();
    virtual void redo();
    virtual int id() const;
    virtual bool mergeWith(const UndoCommand *other);

    bool merged;

private:
    QString *m_str;
    QString m_text;
};

AppendCommand::AppendCommand(QString *str, const QString &text, UndoCommand *parent) :
    UndoCommand(parent),
    merged(false),
    m_str(str),
    m_text(text)
{
    setText("append");
}

void AppendCommand::redo()
{
    m_str->append(m_text);
}

void AppendCommand::undo()
{
    QCOMPARE(m_str->mid(m_str->length() - m_text.length()), m_text);

    m_str->truncate(m_str->length() - m_text.length());
}

int AppendCommand::id() const
{
    return 1;
}

bool AppendCommand::mergeWith(const UndoCommand *other)
{
    if (other->id() != id())
        return false;
    m_text += static_cast<const AppendCommand*>(other)->m_text;
    merged = true;
    return true;
}

class CheckStateArgs;

class tst_UndoGroup : public QObject
{
    Q_OBJECT

public:
    tst_UndoGroup();

private slots:
    void cleanup();

    void setActive();
    void addRemoveStack();
    void deleteStack();
    void checkSignals();
    void addStackAndDie();

private:
    void checkState(const CheckStateArgs &args);

    UndoGroup group;
    QSignalSpy indexChangedSpy;
    QSignalSpy cleanChangedSpy;
    QSignalSpy canUndoChangedSpy;
    QSignalSpy undoTextChangedSpy;
    QSignalSpy canRedoChangedSpy;
    QSignalSpy redoTextChangedSpy;
};

tst_UndoGroup::tst_UndoGroup() :
    indexChangedSpy(&group, SIGNAL(indexChanged(int))),
    cleanChangedSpy(&group, SIGNAL(cleanChanged(bool))),
    canUndoChangedSpy(&group, SIGNAL(canUndoChanged(bool))),
    undoTextChangedSpy(&group, SIGNAL(undoTextChanged(QString))),
    canRedoChangedSpy(&group, SIGNAL(canRedoChanged(bool))),
    redoTextChangedSpy(&group, SIGNAL(redoTextChanged(QString)))
{
}

void tst_UndoGroup::cleanup()
{
    group.setActiveStack(nullptr);

    while (!group.stacks().isEmpty())
        group.removeStack(group.stacks().first());

    indexChangedSpy.clear();
    cleanChangedSpy.clear();
    canUndoChangedSpy.clear();
    undoTextChangedSpy.clear();
    canRedoChangedSpy.clear();
    redoTextChangedSpy.clear();
}

struct CheckStateArgs
{
    CheckStateArgs() :
        activeStack(nullptr),
        clean(false),
        canUndo(false),
        canRedo(false),
        cleanChanged(false),
        indexChanged(false),
        undoChanged(false),
        redoChanged(false)
    {
    }

    UndoStack *activeStack;
    bool clean;
    bool canUndo;
    QString undoText;
    bool canRedo;
    QString redoText;
    bool cleanChanged;
    bool indexChanged;
    bool undoChanged;
    bool redoChanged;
};

void tst_UndoGroup::setActive()
{
    UndoStack stack1(&group), stack2(&group);

    QCOMPARE(group.activeStack(), (UndoStack*)nullptr);
    QCOMPARE(stack1.isActive(), false);
    QCOMPARE(stack2.isActive(), false);

    UndoStack stack3;
    QCOMPARE(stack3.isActive(), true);

    group.addStack(&stack3);
    QCOMPARE(stack3.isActive(), false);

    stack1.setActive();
    QCOMPARE(group.activeStack(), &stack1);
    QCOMPARE(stack1.isActive(), true);
    QCOMPARE(stack2.isActive(), false);
    QCOMPARE(stack3.isActive(), false);

    group.setActiveStack(&stack2);
    QCOMPARE(group.activeStack(), &stack2);
    QCOMPARE(stack1.isActive(), false);
    QCOMPARE(stack2.isActive(), true);
    QCOMPARE(stack3.isActive(), false);

    group.removeStack(&stack2);
    QCOMPARE(group.activeStack(), (UndoStack*)nullptr);
    QCOMPARE(stack1.isActive(), false);
    QCOMPARE(stack2.isActive(), true);
    QCOMPARE(stack3.isActive(), false);

    group.removeStack(&stack2);
    QCOMPARE(group.activeStack(), (UndoStack*)nullptr);
    QCOMPARE(stack1.isActive(), false);
    QCOMPARE(stack2.isActive(), true);
    QCOMPARE(stack3.isActive(), false);
}

void tst_UndoGroup::addRemoveStack()
{
    UndoStack stack1(&group);
    QCOMPARE(group.stacks(), QVector<UndoStack*>() << &stack1);

    UndoStack stack2;
    group.addStack(&stack2);
    QCOMPARE(group.stacks(), QVector<UndoStack*>() << &stack1 << &stack2);

    group.addStack(&stack1);
    QCOMPARE(group.stacks(), QVector<UndoStack*>() << &stack1 << &stack2);

    group.removeStack(&stack1);
    QCOMPARE(group.stacks(), QVector<UndoStack*>() << &stack2);

    group.removeStack(&stack1);
    QCOMPARE(group.stacks(), QVector<UndoStack*>() << &stack2);

    group.removeStack(&stack2);
    QCOMPARE(group.stacks(), QVector<UndoStack*>());
}

void tst_UndoGroup::deleteStack()
{
    UndoStack *stack1 = new UndoStack(&group);
    QCOMPARE(group.stacks(), QVector<UndoStack*>() << stack1);
    QCOMPARE(group.activeStack(), (UndoStack*)nullptr);

    stack1->setActive();
    QCOMPARE(group.activeStack(), stack1);

    UndoStack *stack2 = new UndoStack(&group);
    QCOMPARE(group.stacks(), QVector<UndoStack*>() << stack1 << stack2);
    QCOMPARE(group.activeStack(), stack1);

    UndoStack *stack3 = new UndoStack(&group);
    QCOMPARE(group.stacks(), QVector<UndoStack*>() << stack1 << stack2 << stack3);
    QCOMPARE(group.activeStack(), stack1);

    delete stack2;
    QCOMPARE(group.stacks(), QVector<UndoStack*>() << stack1 << stack3);
    QCOMPARE(group.activeStack(), stack1);

    delete stack1;
    QCOMPARE(group.stacks(), QVector<UndoStack*>() << stack3);
    QCOMPARE(group.activeStack(), (UndoStack*)nullptr);

    stack3->setActive(false);
    QCOMPARE(group.activeStack(), (UndoStack*)nullptr);

    stack3->setActive(true);
    QCOMPARE(group.activeStack(), stack3);

    group.removeStack(stack3);
    QCOMPARE(group.stacks(), QVector<UndoStack*>());
    QCOMPARE(group.activeStack(), (UndoStack*)nullptr);

    delete stack3;
}

void tst_UndoGroup::checkState(const CheckStateArgs &args)
{
    QCOMPARE(group.activeStack(), (UndoStack*)args.activeStack);
    QCOMPARE(group.isClean(), args.clean);
    QCOMPARE(group.canUndo(), args.canUndo);
    QCOMPARE(group.undoText(), QString(args.undoText));
    QCOMPARE(group.canRedo(), args.canRedo);
    QCOMPARE(group.redoText(), QString(args.redoText));
    if (args.indexChanged) {
        QCOMPARE(indexChangedSpy.count(), 1);
        indexChangedSpy.clear();
    } else {
        QCOMPARE(indexChangedSpy.count(), 0);
    }
    if (args.cleanChanged) {
        QCOMPARE(cleanChangedSpy.count(), 1);
        QCOMPARE(cleanChangedSpy.at(0).at(0).toBool(), args.clean);
        cleanChangedSpy.clear();
    } else {
        QCOMPARE(cleanChangedSpy.count(), 0);
    }
    if (args.undoChanged) {
        QCOMPARE(canUndoChangedSpy.count(), 1);
        QCOMPARE(canUndoChangedSpy.at(0).at(0).toBool(), args.canUndo);
        QCOMPARE(undoTextChangedSpy.count(), 1);
        QCOMPARE(undoTextChangedSpy.at(0).at(0).toString(), QString(args.undoText));
        canUndoChangedSpy.clear();
        undoTextChangedSpy.clear();
    } else {
        QCOMPARE(canUndoChangedSpy.count(), 0);
        QCOMPARE(undoTextChangedSpy.count(), 0);
    }
    if (args.redoChanged) {
        QCOMPARE(canRedoChangedSpy.count(), 1);
        QCOMPARE(canRedoChangedSpy.at(0).at(0).toBool(), args.canRedo);
        QCOMPARE(redoTextChangedSpy.count(), 1);
        QCOMPARE(redoTextChangedSpy.at(0).at(0).toString(), QString(args.redoText));
        canRedoChangedSpy.clear();
        redoTextChangedSpy.clear();
    } else {
        QCOMPARE(canRedoChangedSpy.count(), 0);
        QCOMPARE(redoTextChangedSpy.count(), 0);
    }
}

void tst_UndoGroup::checkSignals()
{
    QString string;

    CheckStateArgs args;
    args.activeStack = nullptr;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    group.undo();
    args.activeStack = nullptr;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    group.redo();
    args.activeStack = nullptr;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    UndoStack *stack1 = new UndoStack(&group);
    args.activeStack = nullptr;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack1->push(new AppendCommand(&string, "foo"));
    args.activeStack = nullptr;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack1->setActive();
    args.activeStack = stack1;
    args.clean = false;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack1->push(new InsertCommand(&string, 0, "bar"));
    args.activeStack = stack1;
    args.clean = false;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack1->undo();
    args.activeStack = stack1;
    args.clean = false;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack1->undo();
    args.activeStack = stack1;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack1->undo();
    args.activeStack = stack1;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    group.undo();
    args.activeStack = stack1;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    group.redo();
    args.activeStack = stack1;
    args.clean = false;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack1->setActive(false);
    args.activeStack = nullptr;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    UndoStack *stack2 = new UndoStack(&group);
    args.activeStack = nullptr;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack2->setActive();
    args.activeStack = stack2;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack1->setActive();
    args.activeStack = stack1;
    args.clean = false;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    delete stack1;
    args.activeStack = nullptr;
    args.clean = true;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
}

void tst_UndoGroup::addStackAndDie()
{
    // Test that UndoStack doesn't keep a reference to UndoGroup after the
    // group is deleted.
    UndoStack *stack = new UndoStack;
    UndoGroup *group = new UndoGroup;
    group->addStack(stack);
    delete group;
    stack->setActive(true);
    delete stack;
}

QTEST_MAIN(tst_UndoGroup)

#include "tst_undogroup.moc"

