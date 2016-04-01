#include <QString>
#include <QtTest>

#include <QtUndo/UndoStack>

class tst_Undo : public QObject
{
    Q_OBJECT

public:
    tst_Undo();

private Q_SLOTS:
    void undo();
};

tst_Undo::tst_Undo()
{
}

void tst_Undo::undo()
{
    UndoStack undoStack;
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(tst_Undo)

#include "tst_undo.moc"
