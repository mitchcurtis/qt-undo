#ifndef UNDOSTACK_P_H
#define UNDOSTACK_P_H

#include <QtCore/private/qobject_p.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qaction.h>

#include "undostack.h"

QT_BEGIN_NAMESPACE

class UndoCommand;
class UndoGroup;

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

class UndoStackPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(UndoStack)

public:
    UndoStackPrivate() :
        index(0),
        cleanIndex(0),
        group(0),
        undoLimit(0)
    {
    }

    QList<UndoCommand*> commandList;
    QList<UndoCommand*> macroStack;
    int index;
    int cleanIndex;
    UndoGroup *group;
    int undoLimit;

    void setIndex(int idx, bool clean);
    bool checkUndoLimit();
};

QT_END_NAMESPACE

#endif // UNDOSTACK_P_H
