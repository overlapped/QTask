#ifndef QTASK_GLOBAL_H
#define QTASK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QTASK_LIBRARY)
#  define QTASK_EXPORT Q_DECL_EXPORT
#else
#  define QTASK_EXPORT Q_DECL_IMPORT
#endif

#endif // QTASK_GLOBAL_H