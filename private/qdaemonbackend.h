#ifndef QDAEMONBACKEND_H
#define QDAEMONBACKEND_H

#include "qdaemon-global.h"

QT_BEGIN_NAMESPACE

class Q_DAEMON_LOCAL QDaemonBackend
{
	Q_DISABLE_COPY(QDaemonBackend)

public:
	enum BackendType  {
		ControllerType, DaemonType
	};

	QDaemonBackend();
	virtual ~QDaemonBackend();

	virtual bool initialize() = 0;
	virtual bool finalize() = 0;

	virtual bool start() = 0;
	virtual bool stop() = 0;
	virtual bool install() = 0;
	virtual bool uninstall() = 0;
};

QT_END_NAMESPACE

#endif // QDAEMONBACKEND_H
