#ifndef UNDOCOMMAND_P_H
#define UNDOCOMMAND_P_H

#include <QtCore/private/qobject_p.h>
#include <QtCore/qvector.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class UndoCommand;

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

class UndoCommandPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(UndoCommand)

public:
    UndoCommandPrivate() :
        id(-1)
    {
    }

    QVector<UndoCommand*> childCommands;
    QString text;
    QString actionText;
    int id;
};


QT_END_NAMESPACE

#endif // UNDOCOMMAND_P_H
