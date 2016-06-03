#ifndef DAEMONBACKEND_WIN_H
#define DAEMONBACKEND_WIN_H

#include "qabstractdaemonbackend.h"

#ifndef UNICODE
#error Enable unicode support for your compiler.
#endif

QT_BEGIN_NAMESPACE

namespace QtDaemon
{
	class Q_DAEMON_LOCAL DaemonBackendWindows : public QAbstractDaemonBackend
	{
		Q_DISABLE_COPY(DaemonBackendWindows)

	public:
		DaemonBackendWindows(QCommandLineParser &);
		~DaemonBackendWindows() override;

		int exec() override;
	};
}

QT_END_NAMESPACE

#endif // DAEMONBACKEND_WIN_H
