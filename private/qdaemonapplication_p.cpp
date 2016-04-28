#include "qdaemonapplication_p.h"
#include "qdaemonapplication.h"

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>

#include <csignal>

#ifdef Q_OS_WIN
#include "qdaemonbackend_win.h"
typedef BackendWindows DaemonBackend;
#else
#ifdef Q_OS_LINUX
#include "qdaemonbackend_unix.h"
typedef BackendUnix DaemonBackend;
#else
#error "This library is not supported on you platform"
#endif
#endif

QT_BEGIN_NAMESPACE

QDaemonApplicationPrivate::QDaemonApplicationPrivate(QDaemonApplication * q)
	: q_ptr(q), backend(nullptr), autoQuit(true)
{
	std::signal(SIGTERM, QDaemonApplicationPrivate::processSignalHandler);
	std::signal(SIGINT, QDaemonApplicationPrivate::processSignalHandler);
	std::signal(SIGSEGV, QDaemonApplicationPrivate::processSignalHandler);
}

QDaemonApplicationPrivate::~QDaemonApplicationPrivate()
{
	if (backend)
		backend->finalize();

	delete backend;
}

int QDaemonApplicationPrivate::exec()
{
	Q_Q(QDaemonApplication);

	// Process the command line
	if (!commandLine.process(q->arguments()))
		return -1;

	Operations op = commandLine.operations();

	// Handle the help option directly
	if (op.testFlag(HelpOperation))  {
		QTextStream out(stdout);
		out << commandLine.helpText();

		return 0;
	}
	else if (op.testFlag(FakeOperation))  {
		// Runs the daemon in fake mode. It will emit the daemonized() signal and will start the event loop, but will not detach from the controlling terminal.
		// Control will not be available, except through the debugger (or through POSIX signals).
		QMetaObject::invokeMethod(q, "daemonized", Qt::QueuedConnection);
		return QCoreApplication::exec();
	}

	// Create the backend
	backend = DaemonBackend::create(op ? QDaemonBackend::ControllerType : QDaemonBackend::DaemonType);

	// Set any additional arguments if needed
#ifdef Q_OS_LINUX
	DaemonBackend::Arguments arguments;
	arguments.insert(DaemonBackend::dbusPrefix, commandLine.value(DaemonBackend::dbusPrefix));
	arguments.insert(DaemonBackend::initdPrefix, commandLine.value(DaemonBackend::initdPrefix));

	backend->setArguments(arguments);
#endif

	if (!backend->initialize())  {
		qWarning("The daemon controller backend failed to initialize");
		return -1;
	}

	bool status = true;
	if (op.testFlag(InstallOperation))  {
		if (backend->install())
			QMetaObject::invokeMethod(q, "installed", Qt::QueuedConnection);
		else
			status = false;
	}
	else if (op.testFlag(UninstallOperation))  {
		if (backend->uninstall())
			QMetaObject::invokeMethod(q, "uninstalled", Qt::QueuedConnection);
		else
			status = false;
	}
	else if (op.testFlag(StartOperation))  {
		if (backend->start())
			QMetaObject::invokeMethod(q, "started", Qt::QueuedConnection);
		else
			status = false;
	}
	else if (op.testFlag(StopOperation))  {
		if (backend->stop())
			QMetaObject::invokeMethod(q, "stopped", Qt::QueuedConnection);
		else
			status = false;
	}
	else
		QMetaObject::invokeMethod(q, "daemonized", Qt::QueuedConnection);


	if (!status)
		return -1;
	else if (op && autoQuit)
		QMetaObject::invokeMethod(q, "quit", Qt::QueuedConnection);

	return QCoreApplication::exec();	// Finally, start the event loop
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

// --------------------------------------------------------------------------------------------------------------------------------------------------------- //



QDaemonApplicationPrivate::CommandLineOptions::CommandLineOptions()
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
		qWarning("More than one control operation was specified.");
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

QT_END_NAMESPACE
