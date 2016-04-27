#ifndef QDAEMONBACKEND_WIN_H
#define QDAEMONBACKEND_WIN_H

#include "qdaemonbackend.h"

#ifdef Q_OS_WIN

QT_BEGIN_NAMESPACE

class Q_DAEMON_LOCAL BackendWindows : public QDaemonBackend
{
	Q_DISABLE_COPY(BackendWindows)

public:
	BackendWindows();
	virtual ~BackendWindows();

	virtual bool initialize();
	virtual bool finalize();

	virtual bool start();
	virtual bool stop();
	virtual bool install();
	virtual bool uninstall();
};


QT_END_NAMESPACE

#endif
#endif // QDAEMONBACKEND_WIN_H
