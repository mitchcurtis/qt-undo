#include <QString>
#include <QtTest>
#include <QtUndo/undocommand.h>
#include <QtUndo/undostack.h>

class InsertCommand : public UndoCommand
{
public:
    InsertCommand(QString *str, int idx, const QString &text,
                    UndoCommand *parent = 0);

    virtual void undo() override;
    virtual void redo() override;

private:
    QString *m_str;
    int m_idx;
    QString m_text;
};

class RemoveCommand : public UndoCommand
{
public:
    RemoveCommand(QString *str, int idx, int len, UndoCommand *parent = 0);

    virtual void undo() override;
    virtual void redo() override;

private:
    QString *m_str;
    int m_idx;
    QString m_text;
};

class AppendCommand : public UndoCommand
{
public:
    AppendCommand(QString *str, const QString &text, bool _fail_merge = false,
                    UndoCommand *parent = 0);
    ~AppendCommand();

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const;
    virtual bool mergeWith(const UndoCommand *other);

    bool merged;
    bool fail_merge;
    static int delete_cnt;

private:
    QString *m_str;
    QString m_text;
};

InsertCommand::InsertCommand(QString *str, int idx, const QString &text,
                            UndoCommand *parent)
    : UndoCommand(parent)
{
    QVERIFY(str->length() >= idx);

    setText("insert");

    m_str = str;
    m_idx = idx;
    m_text = text;
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

RemoveCommand::RemoveCommand(QString *str, int idx, int len, UndoCommand *parent)
    : UndoCommand(parent)
{
    QVERIFY(str->length() >= idx + len);

    setText("remove");

    m_str = str;
    m_idx = idx;
    m_text = m_str->mid(m_idx, len);
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

int AppendCommand::delete_cnt = 0;

AppendCommand::AppendCommand(QString *str, const QString &text, bool _fail_merge,
                                UndoCommand *parent)
    : UndoCommand(parent)
{
    setText("append");

    m_str = str;
    m_text = text;
    merged = false;
    fail_merge = _fail_merge;
}

AppendCommand::~AppendCommand()
{
    ++delete_cnt;
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
    if (fail_merge)
        return false;
    m_text += static_cast<const AppendCommand*>(other)->m_text;
    merged = true;
    return true;
}

struct CheckStateArgs
{
    CheckStateArgs() :
        clean(false),
        count(0),
        index(-1),
        canUndo(false),
        canRedo(false),
        cleanChanged(false),
        indexChanged(false),
        undoChanged(false),
        redoChanged(false)
    {
    }

    bool clean;
    int count;
    int index;
    bool canUndo;
    QString undoText;
    bool canRedo;
    QString redoText;
    bool cleanChanged;
    bool indexChanged;
    bool undoChanged;
    bool redoChanged;
};

class tst_Undo : public QObject
{
    Q_OBJECT
public:
    tst_Undo();

private slots:
    void cleanup();

    void undoRedo();
    void setIndex();
    void setClean();
    void clear();
    void childCommand();
    void macroBeginEnd();
    void compression();
    void undoLimit();

private:
    void checkState(const CheckStateArgs &args);

    UndoStack stack;
    QSignalSpy indexChangedSpy;
    QSignalSpy cleanChangedSpy;
    QSignalSpy canUndoChangedSpy;
    QSignalSpy undoTextChangedSpy;
    QSignalSpy canRedoChangedSpy;
    QSignalSpy redoTextChangedSpy;
};

tst_Undo::tst_Undo() :
    indexChangedSpy(&stack, SIGNAL(indexChanged(int))),
    cleanChangedSpy(&stack, SIGNAL(cleanChanged(bool))),
    canUndoChangedSpy(&stack, SIGNAL(canUndoChanged(bool))),
    undoTextChangedSpy(&stack, SIGNAL(undoTextChanged(QString))),
    canRedoChangedSpy(&stack, SIGNAL(canRedoChanged(bool))),
    redoTextChangedSpy(&stack, SIGNAL(redoTextChanged(QString)))
{
}

void tst_Undo::cleanup()
{
    stack.clear();
    indexChangedSpy.clear();
    cleanChangedSpy.clear();
    canUndoChangedSpy.clear();
    undoTextChangedSpy.clear();
    canRedoChangedSpy.clear();
    redoTextChangedSpy.clear();
}

void tst_Undo::checkState(const CheckStateArgs &args)
{
    QCOMPARE(stack.count(), args.count);
    QCOMPARE(stack.isClean(), args.clean);
    QCOMPARE(stack.index(), args.index);
    QCOMPARE(stack.canUndo(), args.canUndo);
    QCOMPARE(stack.undoText(), QString(args.undoText));
    QCOMPARE(stack.canRedo(), args.canRedo);
    QCOMPARE(stack.redoText(), QString(args.redoText));
    if (args.indexChanged) {
        QCOMPARE(indexChangedSpy.count(), 1);
        QCOMPARE(indexChangedSpy.at(0).at(0).toInt(), args.index);
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

void tst_Undo::undoRedo()
{
    QString string;

    // push, undo, redo
    CheckStateArgs args;
    args.clean = true;
    args.count = 0;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.undo(); // nothing to undo
    QCOMPARE(string, QString());
    args.clean = true;
    args.count = 0;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.push(new InsertCommand(&string, 0, "hello"));
    QCOMPARE(string, QString("hello"));
    args.clean = false;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new InsertCommand(&string, 2, "123"));
    QCOMPARE(string, QString("he123llo"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("hello"));
    args.clean = false;
    args.count = 2;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.redo();
    QCOMPARE(string, QString("he123llo"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.redo(); // nothing to redo
    QCOMPARE(string, QString("he123llo"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("hello"));
    args.clean = false;
    args.count = 2;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString());
    args.clean = true;
    args.count = 2;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo(); // nothing to undo
    QCOMPARE(string, QString());
    args.clean = true;
    args.count = 2;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    // push after undo - check that undone commands get deleted

    stack.redo();
    args.clean = false;
    args.count = 2;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new RemoveCommand(&string, 2, 2));
    QCOMPARE(string, QString("heo"));
    args.clean = false;
    args.count = 2; // - still 2, last command got deleted
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("remove");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("hello"));
    args.clean = false;
    args.count = 2;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = true;
    args.redoText = QLatin1String("remove");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString());
    args.clean = true;
    args.count = 2;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new InsertCommand(&string, 0, "goodbye"));
    QCOMPARE(string, QString("goodbye"));
    args.clean = false;
    args.count = 1; // - two commands got deleted
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
}

void tst_Undo::setIndex()
{
    UndoStack stack;
    QString string;

    stack.setIndex(10); // should do nothing
    CheckStateArgs args;
    args.clean = true;
    args.count = 0;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.setIndex(0); // should do nothing
    args.clean = true;
    args.count = 0;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.setIndex(-10); // should do nothing
    args.clean = true;
    args.count = 0;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.push(new InsertCommand(&string, 0, "hello"));
    QCOMPARE(string, QString("hello"));
    args.clean = false;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new InsertCommand(&string, 2, "123"));
    QCOMPARE(string, QString("he123llo"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setIndex(2);
    QCOMPARE(string, QString("he123llo"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.setIndex(0);
    QCOMPARE(string, QString());
    args.clean = true;
    args.count = 2;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setIndex(10); // should set index to 2
    QCOMPARE(string, QString("he123llo"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setIndex(-10); // should set index to 0
    QCOMPARE(string, QString());
    args.clean = true;
    args.count = 2;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setIndex(1);
    QCOMPARE(string, QString("hello"));
    args.clean = false;
    args.count = 2;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setIndex(2);
    QCOMPARE(string, QString("he123llo"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
}

void tst_Undo::setClean()
{
    QString string;

    QCOMPARE(stack.cleanIndex(), 0);
    stack.setClean();
    CheckStateArgs args;
    args.clean = true;
    args.count = 0;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);
    QCOMPARE(stack.cleanIndex(), 0);

    stack.push(new InsertCommand(&string, 0, "goodbye"));
    args.clean = false;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
    QCOMPARE(stack.cleanIndex(), 0);

    stack.setClean();
    QCOMPARE(string, QString("goodbye"));
    args.clean = true;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);
    QCOMPARE(stack.cleanIndex(), 1);

    stack.push(new AppendCommand(&string, " cowboy"));
    QCOMPARE(string, QString("goodbye cowboy"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
    QCOMPARE(stack.cleanIndex(), 1);

    stack.undo(); // reaching clean state from above
    QCOMPARE(string, QString("goodbye"));
    args.clean = true;
    args.count = 2;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
    QCOMPARE(stack.cleanIndex(), 1);

    stack.undo();
    QCOMPARE(string, QString());
    args.clean = false;
    args.count = 2;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
    QCOMPARE(stack.cleanIndex(), 1);

    stack.redo(); // reaching clean state from below
    QCOMPARE(string, QString("goodbye"));
    args.clean = true;
    args.count = 2;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
    QCOMPARE(stack.cleanIndex(), 1);

    stack.undo();
    QCOMPARE(string, QString());
    args.clean = false;
    args.count = 2;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
    QCOMPARE(stack.cleanIndex(), 1);

    stack.push(new InsertCommand(&string, 0, "foo")); // the clean state gets deleted!
    QCOMPARE(string, QString("foo"));
    args.clean = false;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
    QCOMPARE(stack.cleanIndex(), -1);

    stack.undo();
    QCOMPARE(string, QString());
    args.clean = false;
    args.count = 1;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
    QCOMPARE(stack.cleanIndex(), -1);
}

void tst_Undo::clear()
{
    QString string;

    CheckStateArgs args;
    args.clean = true;
    args.count = 0;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.push(new InsertCommand(&string, 0, "hello"));
    QCOMPARE(string, QString("hello"));
    args.clean = false;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new InsertCommand(&string, 2, "123"));
    QCOMPARE(string, QString("he123llo"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.clear();
    QCOMPARE(string, QString("he123llo"));
    args.clean = true;
    args.count = 0;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    string.clear();
    stack.push(new InsertCommand(&string, 0, "hello"));
    QCOMPARE(string, QString("hello"));
    args.clean = false;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new InsertCommand(&string, 2, "123"));
    QCOMPARE(string, QString("he123llo"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setIndex(0);
    QCOMPARE(string, QString());
    args.clean = true;
    args.count = 2;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.clear();
    QCOMPARE(string, QString());
    args.clean = true;
    args.count = 0;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
}

void tst_Undo::childCommand()
{
    QString string;

    stack.push(new InsertCommand(&string, 0, "hello"));
    QCOMPARE(string, QString("hello"));
    CheckStateArgs args;
    args.clean = false;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    UndoCommand *command = new UndoCommand();
    command->setText("ding");
    new InsertCommand(&string, 5, "world", command);
    new RemoveCommand(&string, 4, 1, command);
    stack.push(command);
    QCOMPARE(string, QString("hellworld"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("ding");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("hello"));
    args.clean = false;
    args.count = 2;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = true;
    args.redoText = QLatin1String("ding");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.redo();
    QCOMPARE(string, QString("hellworld"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("ding");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
}

void tst_Undo::macroBeginEnd()
{
    QString string;

    stack.beginMacro("ding");
    CheckStateArgs args;
    args.clean = false;
    args.count = 1;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setClean(); // should do nothing
    args.clean = false;
    args.count = 1;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.undo(); // should do nothing
    args.clean = false;
    args.count = 1;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.redo(); // should do nothing
    args.clean = false;
    args.count = 1;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.setIndex(0); // should do nothing
    args.clean = false;
    args.count = 1;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.endMacro();
    args.clean = false;
    args.count = 1;
    args.index = 1; // - endMacro() increments index
    args.canUndo = true;
    args.undoText = QLatin1String("ding");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new InsertCommand(&string, 0, "h"));
    QCOMPARE(string, QString("h"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new InsertCommand(&string, 1, "owdy"));
    QCOMPARE(string, QString("howdy"));
    args.clean = false;
    args.count = 3;
    args.index = 3;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setIndex(2);
    QCOMPARE(string, QString("h"));
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.beginMacro("dong"); // the "owdy" command gets deleted
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new InsertCommand(&string, 1, "ello"));
    QCOMPARE(string, QString("hello"));
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.push(new RemoveCommand(&string, 1, 2));
    QCOMPARE(string, QString("hlo"));
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.beginMacro("dong2");
    QCOMPARE(string, QString("hlo"));
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.push(new RemoveCommand(&string, 1, 1));
    QCOMPARE(string, QString("ho"));
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.endMacro();
    QCOMPARE(string, QString("ho"));
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.endMacro();
    QCOMPARE(string, QString("ho"));
    args.clean = false;
    args.count = 3;
    args.index = 3;
    args.canUndo = true;
    args.undoText = QLatin1String("dong");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("h"));
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = true;
    args.redoText = QLatin1String("dong");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString(""));
    args.clean = false;
    args.count = 3;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("ding");
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setIndex(3);
    QCOMPARE(string, QString("ho"));
    args.clean = false;
    args.count = 3;
    args.index = 3;
    args.canUndo = true;
    args.undoText = QLatin1String("dong");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setIndex(1);
    QCOMPARE(string, QString());
    args.clean = false;
    args.count = 3;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("ding");
    args.canRedo = true;
    args.redoText = QLatin1String("insert");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
}

void tst_Undo::compression()
{
    QString string;

    AppendCommand::delete_cnt = 0;

    stack.push(new InsertCommand(&string, 0, "ene"));
    QCOMPARE(string, QString("ene"));
    CheckStateArgs args;
    args.clean = false;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, " due")); // #1
    QCOMPARE(string, QString("ene due"));
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, " rike")); // #2 should merge
    QCOMPARE(string, QString("ene due rike"));
    QCOMPARE(AppendCommand::delete_cnt, 1); // #2 should be deleted
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setClean();
    args.clean = true;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.push(new AppendCommand(&string, " fake")); // #3 should NOT merge, since the stack was clean
    QCOMPARE(string, QString("ene due rike fake"));  // and we want to be able to return to this state
    QCOMPARE(AppendCommand::delete_cnt, 1); // #3 should not be deleted
    args.clean = false;
    args.count = 3;
    args.index = 3;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("ene due rike"));
    args.clean = true;
    args.count = 3;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("ene"));
    args.clean = false;
    args.count = 3;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("insert");
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, "ma", true)); // #4 clean state gets deleted!
    QCOMPARE(string, QString("enema"));
    QCOMPARE(AppendCommand::delete_cnt, 3); // #1 got deleted
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, "trix")); // #5 should NOT merge
    QCOMPARE(string, QString("enematrix"));
    QCOMPARE(AppendCommand::delete_cnt, 3);
    args.clean = false;
    args.count = 3;
    args.index = 3;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("enema"));
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    // and now for command compression inside macros

    stack.setClean();
    QCOMPARE(string, QString("enema"));
    args.clean = true;
    args.count = 3;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = true;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.beginMacro("ding");
    QCOMPARE(string, QString("enema"));
    QCOMPARE(AppendCommand::delete_cnt, 4); // #5 gets deleted
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    AppendCommand *merge_cmd = new AppendCommand(&string, "top");
    stack.push(merge_cmd); // #6
    QCOMPARE(merge_cmd->merged, false);
    QCOMPARE(string, QString("enematop"));
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.push(new AppendCommand(&string, "eja")); // #7 should merge
    QCOMPARE(string, QString("enematopeja"));
    QCOMPARE(merge_cmd->merged, true);
    QCOMPARE(AppendCommand::delete_cnt, 5); // #7 gets deleted
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);
    merge_cmd->merged = false;

    stack.push(new InsertCommand(&string, 2, "123")); // should not merge
    QCOMPARE(string, QString("en123ematopeja"));
    QCOMPARE(merge_cmd->merged, false);
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.endMacro();
    QCOMPARE(string, QString("en123ematopeja"));
    args.clean = false;
    args.count = 3;
    args.index = 3;
    args.canUndo = true;
    args.undoText = QLatin1String("ding");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("enema"));
    args.clean = true;
    args.count = 3;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = true;
    args.redoText = QLatin1String("ding");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.redo();
    QCOMPARE(string, QString("en123ematopeja"));
    args.clean = false;
    args.count = 3;
    args.index = 3;
    args.canUndo = true;
    args.undoText = QLatin1String("ding");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
}

void tst_Undo::undoLimit()
{
    AppendCommand::delete_cnt = 0;
    QString string;

    QCOMPARE(stack.undoLimit(), 0);
    stack.setUndoLimit(2);
    QCOMPARE(stack.undoLimit(), 2);

    stack.push(new AppendCommand(&string, "1", true));
    QCOMPARE(string, QString("1"));
    QCOMPARE(AppendCommand::delete_cnt, 0);
    CheckStateArgs args;
    args.clean = false;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, "2", true));
    QCOMPARE(string, QString("12"));
    QCOMPARE(AppendCommand::delete_cnt, 0);
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.setClean();
    QCOMPARE(string, QString("12"));
    QCOMPARE(AppendCommand::delete_cnt, 0);
    args.clean = true;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.push(new AppendCommand(&string, "3", true));
    QCOMPARE(string, QString("123"));
    QCOMPARE(AppendCommand::delete_cnt, 1);
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, "4", true));
    QCOMPARE(string, QString("1234"));
    QCOMPARE(AppendCommand::delete_cnt, 2);
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("123"));
    QCOMPARE(AppendCommand::delete_cnt, 2);
    args.clean = false;
    args.count = 2;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("12"));
    QCOMPARE(AppendCommand::delete_cnt, 2);
    args.clean = true;
    args.count = 2;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, "3", true));
    QCOMPARE(string, QString("123"));
    QCOMPARE(AppendCommand::delete_cnt, 4);
    args.clean = false;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = true;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, "4", true));
    QCOMPARE(string, QString("1234"));
    QCOMPARE(AppendCommand::delete_cnt, 4);
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, "5", true));
    QCOMPARE(string, QString("12345"));
    QCOMPARE(AppendCommand::delete_cnt, 5);
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("1234"));
    QCOMPARE(AppendCommand::delete_cnt, 5);
    args.clean = false;
    args.count = 2;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("123"));
    QCOMPARE(AppendCommand::delete_cnt, 5);
    args.clean = false;
    args.count = 2;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, "4", true));
    QCOMPARE(string, QString("1234"));
    QCOMPARE(AppendCommand::delete_cnt, 7);
    args.clean = false;
    args.count = 1;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, "5"));
    QCOMPARE(string, QString("12345"));
    QCOMPARE(AppendCommand::delete_cnt, 7);
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, "6", true)); // should be merged
    QCOMPARE(string, QString("123456"));
    QCOMPARE(AppendCommand::delete_cnt, 8);
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.beginMacro("foo");
    QCOMPARE(string, QString("123456"));
    QCOMPARE(AppendCommand::delete_cnt, 8);
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.push(new AppendCommand(&string, "7", true));
    QCOMPARE(string, QString("1234567"));
    QCOMPARE(AppendCommand::delete_cnt, 8);
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.push(new AppendCommand(&string, "8"));
    QCOMPARE(string, QString("12345678"));
    QCOMPARE(AppendCommand::delete_cnt, 8);
    args.clean = false;
    args.count = 3;
    args.index = 2;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = false;
    args.undoChanged = false;
    args.redoChanged = false;
    checkState(args);

    stack.endMacro();
    QCOMPARE(string, QString("12345678"));
    QCOMPARE(AppendCommand::delete_cnt, 9);
    args.clean = false;
    args.count = 2;
    args.index = 2;
    args.canUndo = true;
    args.undoText = QLatin1String("foo");
    args.canRedo = false;
    args.redoText = QString();
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("123456"));
    QCOMPARE(AppendCommand::delete_cnt, 9);
    args.clean = false;
    args.count = 2;
    args.index = 1;
    args.canUndo = true;
    args.undoText = QLatin1String("append");
    args.canRedo = true;
    args.redoText = QLatin1String("foo");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);

    stack.undo();
    QCOMPARE(string, QString("1234"));
    QCOMPARE(AppendCommand::delete_cnt, 9);
    args.clean = false;
    args.count = 2;
    args.index = 0;
    args.canUndo = false;
    args.undoText = QString();
    args.canRedo = true;
    args.redoText = QLatin1String("append");
    args.cleanChanged = false;
    args.indexChanged = true;
    args.undoChanged = true;
    args.redoChanged = true;
    checkState(args);
}

QTEST_APPLESS_MAIN(tst_Undo)

#include "tst_undo.moc"
