#include "qabstractdaemonbackend.h"
#include "qdaemonapplication.h"
#include "qdaemonlog.h"

#include <QCommandLineParser>

QT_BEGIN_NAMESPACE

using namespace QtDaemon;

const int QAbstractDaemonBackend::BackendFailed = -1;

QAbstractDaemonBackend::QAbstractDaemonBackend(QCommandLineParser & prsr)
	: parser(prsr)
{
}

QAbstractDaemonBackend::~QAbstractDaemonBackend()
{
}

QAbstractControllerBackend::QAbstractControllerBackend(QCommandLineParser & parser, bool aq)
	: QAbstractDaemonBackend(parser), autoQuit(aq),
	  installOption(QStringList() << "i" << "install", QCoreApplication::translate("main", "Install the daemon")),
	  uninstallOption(QStringList() << "u" << "uninstall", QCoreApplication::translate("main", "Uninstall the daemon")),
	  startOption(QStringList() << "s" << "start", QCoreApplication::translate("main", "Start the daemon")),
	  stopOption(QStringList() << "t" << "stop", QCoreApplication::translate("main", "Stop the daemon")),
	  fakeOption(QStringLiteral("fake"), QCoreApplication::translate("main", "Run the daemon in fake mode (for debugging)."))
{
	parser.addOption(installOption);
	parser.addOption(uninstallOption);
	parser.addOption(startOption);
	parser.addOption(stopOption);
	parser.addOption(fakeOption);
	parser.addHelpOption();
}

int QAbstractControllerBackend::exec()
{
	bool status = true;
	if (parser.isSet(startOption))
		status = start();
	else if (parser.isSet(stopOption))
		status = stop();
	else if (parser.isSet(installOption))
		status = install();
	else if (parser.isSet(uninstallOption))
		status = uninstall();
	else if (parser.isSet(fakeOption))  {
		autoQuit = false;	// Enforce not quitting

		QStringList arguments = parser.positionalArguments();
		arguments.prepend(QDaemonApplication::applicationFilePath());

		QMetaObject::invokeMethod(qApp, "daemonized", Qt::QueuedConnection, Q_ARG(QStringList, arguments));
	}
	else  {		// Everything else + the help option, show help and enforce quit
		qDaemonLog() << parser.helpText();
		return 0;
	}

	if (!status)
		return BackendFailed;

	if (autoQuit)
		QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);

	return QCoreApplication::exec();
}

QT_END_NAMESPACE
