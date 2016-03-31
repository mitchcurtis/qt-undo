#include <QString>
#include <QtTest>

class tst_Undo : public QObject
{
    Q_OBJECT

public:
    tst_Undo();

private Q_SLOTS:
    void testCase1();
};

tst_Undo::tst_Undo()
{
}

void tst_Undo::testCase1()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(tst_Undo)

#include "tst_undo.moc"
