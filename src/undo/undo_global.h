#ifndef UNDO_GLOBAL_H
#define UNDO_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#  if defined(QT_BUILD_QUICKTEMPLATES_LIB)
#    define Q_UNDO_EXPORT Q_DECL_EXPORT
#  else
#    define Q_UNDO_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define Q_UNDO_EXPORT
#endif

QT_END_NAMESPACE

#endif // UNDO_GLOBAL_H
