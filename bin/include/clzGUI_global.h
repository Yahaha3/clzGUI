#ifndef CLZGUI_GLOBAL_H
#define CLZGUI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CLZGUI_LIBRARY)
#  define CLZGUI_EXPORT Q_DECL_EXPORT
#else
#  define CLZGUI_EXPORT Q_DECL_IMPORT
#endif

#endif // CLZGUI_GLOBAL_H
