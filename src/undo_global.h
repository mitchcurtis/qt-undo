#ifndef UNDO_GLOBAL_H
#define UNDO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(UNDO_LIBRARY)
#  define UNDOSHARED_EXPORT Q_DECL_EXPORT
#else
#  define UNDOSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // UNDO_GLOBAL_H
