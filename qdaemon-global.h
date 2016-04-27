#ifndef QDAEMON_GLOBAL_H
#define QDAEMON_GLOBAL_H

#include <QtGlobal>

#if defined(QDAEMON_LIBRARY)
	#define Q_DAEMON_EXPORT Q_DECL_EXPORT
	#define Q_DAEMON_LOCAL Q_DECL_HIDDEN
#else
	#define Q_DAEMON_EXPORT Q_DECL_IMPORT
#endif

#endif // QDAEMON_GLOBAL_H
