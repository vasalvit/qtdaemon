#include "qdaemonapplication_p.h"
#include "qdaemonapplication.h"

#include <csignal>

#if defined(Q_OS_WIN)
#include "daemonbackend_win.h"
#include "controllerbackend_win.h"
typedef DaemonBackendWindows DaemonBackend;
typedef ControllerBackendWindows ControllerBackend;
#else
#include "daemonbackend_linux.h"
#include "controllerbackend_linux.h"
typedef DaemonBackendLinux DaemonBackend;
typedef ControllerBackendLinux ControllerBackend;
#if !defined(Q_OS_LINUX)
#warning This library is not supported on your platform.
#endif
#endif

QT_BEGIN_NAMESPACE

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

// --------------------------------------------------------------------------------------------------------------------------------------------------------- //

/*QDaemonApplicationPrivate::CommandLineOptions::CommandLineOptions()
	: parser(),
	  installOption(QStringList() << "i" << "install", QCoreApplication::translate("main", "Install the daemon")),
	  uninstallOption(QStringList() << "u" << "uninstall", QCoreApplication::translate("main", "Uninstall the daemon")),
	  startOption(QStringList() << "s" << "start", QCoreApplication::translate("main", "Start the daemon")),
	  stopOption(QStringList() << "t" << "stop" << "terminate", QCoreApplication::translate("main", "Stop the daemon")),
	  fakeOption(QStringList() << "fake", QCoreApplication::translate("main", "Run the daemon in fake mode (for debugging purposes). The process will not actually daemonize itself, but will run as a console application faking a running daemon.")),
	  helpOption(parser.addHelpOption())

{
	parser.addOption(installOption);
	parser.addOption(uninstallOption);
	parser.addOption(startOption);
	parser.addOption(stopOption);
	parser.addOption(fakeOption);

#ifdef Q_OS_LINUX
	const QCommandLineOption dbusPrefixOption(DaemonBackend::dbusPrefix, QCoreApplication::translate("main", "Sets the path for the installed dbus configuration file (defaults to /etc/dbus-1/system.d"), QStringLiteral("path"), QStringLiteral("/etc/dbus-1/system.d"));
	const QCommandLineOption initdPrefixOption(DaemonBackend::initdPrefix, QCoreApplication::translate("main", "Sets the path for the installed init.d script (defaults to /etc/init.d"), QStringLiteral("path"), QStringLiteral("/etc/init.d"));
	parser.addOption(dbusPrefixOption);
	parser.addOption(initdPrefixOption);
#endif
}

bool QDaemonApplicationPrivate::CommandLineOptions::process(const QStringList & arguments)
{
	parser.process(arguments);

	// Check the requested operation
	if (parser.isSet(installOption))
		op |= InstallOperation;
	if (parser.isSet(uninstallOption))
		op |= UninstallOperation;
	if (parser.isSet(startOption))
		op |= StartOperation;
	if (parser.isSet(stopOption))
		op |= StopOperation;
	if (parser.isSet(helpOption))
		op |= HelpOperation;
	if (parser.isSet(fakeOption))
		op |= FakeOperation;

	quint32 flags = op & (InstallOperation | UninstallOperation | StartOperation | StopOperation | HelpOperation | FakeOperation);
	if (qPopulationCount(flags) > 1)  {
		qDaemonLog("More than one control operation was specified.", QDaemonLog::ErrorEntry);
		return false;
	}

	return true;
}

inline QString QDaemonApplicationPrivate::CommandLineOptions::helpText() const
{
	return parser.helpText();
}

inline QDaemonApplicationPrivate::Operations QDaemonApplicationPrivate::CommandLineOptions::operations() const
{
	return op;
}

inline QString QDaemonApplicationPrivate::CommandLineOptions::value(const QString & name) const
{
	return parser.value(name);
}
*/
QT_END_NAMESPACE
