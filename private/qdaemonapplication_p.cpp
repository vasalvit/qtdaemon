#include "qdaemonapplication_p.h"
#include "qdaemonapplication.h"

#include <csignal>

#if defined(Q_OS_WIN)
#include "daemonbackend_win.h"
#include "controllerbackend_win.h"
#else
#include "daemonbackend_linux.h"
#include "controllerbackend_linux.h"
#if !defined(Q_OS_LINUX)
#warning This library is not supported on your platform.
#endif
#endif

QT_BEGIN_NAMESPACE

#if defined(Q_OS_WIN)
typedef DaemonBackendWindows DaemonBackend;
typedef ControllerBackendWindows ControllerBackend;
#else
typedef DaemonBackendLinux DaemonBackend;
typedef ControllerBackendLinux ControllerBackend;
#endif

QString QDaemonApplicationPrivate::description;

QDaemonApplicationPrivate::QDaemonApplicationPrivate(QDaemonApplication * q)
	: q_ptr(q), autoQuit(true)
{
	std::signal(SIGTERM, QDaemonApplicationPrivate::processSignalHandler);
	std::signal(SIGINT, QDaemonApplicationPrivate::processSignalHandler);
	std::signal(SIGSEGV, QDaemonApplicationPrivate::processSignalHandler);
}

QDaemonApplicationPrivate::~QDaemonApplicationPrivate()
{
}

void QDaemonApplicationPrivate::processSignalHandler(int signalNumber)
{
	switch (signalNumber)
	{
	case SIGSEGV:
	   ::exit(-1);
	case SIGTERM:
	case SIGINT:
		{
			QDaemonApplication * app = QDaemonApplication::instance();
			if (app)
				app->quit();
			else
				::exit(-1);
		}
		break;
	default:
		return;
	}
}

QAbstractDaemonBackend * QDaemonApplicationPrivate::createBackend(bool isDaemon)
{
	if (isDaemon)  {
		log.setLogType(QDaemonLog::LogToFile);
		return new DaemonBackend(parser);
	}
	else
		return new ControllerBackend(parser, autoQuit);
}

QT_END_NAMESPACE
