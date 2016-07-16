#include "daemonbackend_linux.h"
#include "qdaemonapplication.h"
#include "qdaemonlog.h"

#include <QtCore/qstring.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qcommandlineparser.h>

#include <QtDBus/qdbusconnection.h>
#include <QtDBus/qdbuserror.h>

QT_BEGIN_NAMESPACE

using namespace QtDaemon;

DaemonBackendLinux::DaemonBackendLinux(QCommandLineParser & arguments)
	: QAbstractDaemonBackend(arguments)
{
}

DaemonBackendLinux::~DaemonBackendLinux()
{
}

int DaemonBackendLinux::exec()
{
	QString service = serviceName();

	// Connect to the DBus infrastructure
	QDBusConnection dbus = QDBusConnection::systemBus();
	if (!dbus.isConnected())  {
		qDaemonLog(QStringLiteral("Can't connect to the D-Bus system bus: %1").arg(dbus.lastError().message()), QDaemonLog::ErrorEntry);
		return BackendFailed;
	}

	// Register the service
	if (!dbus.registerService(service))  {
		qDaemonLog(QStringLiteral("Couldn't register a service with the D-Bus system bus: %1").arg(dbus.lastError().message()), QDaemonLog::ErrorEntry);
		return BackendFailed;
	}

	// Register the object
	if (!dbus.registerObject(QStringLiteral("/"), this, QDBusConnection::ExportAllInvokables))  {
		qDaemonLog(QStringLiteral("Couldn't register an object with the D-Bus system bus. (%1)").arg(dbus.lastError().message()), QDaemonLog::ErrorEntry);
		return BackendFailed;
	}

	QStringList arguments = parser.positionalArguments();
	arguments.prepend(QDaemonApplication::applicationFilePath());

	QMetaObject::invokeMethod(qApp, "daemonized", Qt::QueuedConnection, Q_ARG(QStringList, arguments));

	int status = QCoreApplication::exec();

	// Unregister the object
	dbus.unregisterObject(QStringLiteral("/"));

	// Unregister the service
	if (!dbus.unregisterService(service))
		qDaemonLog(QStringLiteral("Can't unregister service from D-bus. (%1)").arg(dbus.lastError().message()), QDaemonLog::WarningEntry);

	return status;
}

bool DaemonBackendLinux::isRunning()
{
	return true;	// This is just for notifying the controlling process. The function is invoked over D-Bus only.
}

bool DaemonBackendLinux::stop()
{
	qApp->quit();	// This is just to respond to the controlling process. The function is invoked over D-Bus only.
	return true;
}

QString DaemonBackendLinux::serviceName()
{
	QString executable = QFileInfo(QDaemonApplication::applicationFilePath()).completeBaseName();
	QString domain = QDaemonApplication::organizationDomain();

	// Get the service name
	if (domain.isEmpty())
		return QStringLiteral("io.qt.QtDaemon.%1").arg(executable);

	QStringList elements = domain.split('.', QString::SkipEmptyParts);
	std::reverse(elements.begin(), elements.end());
	return elements.join('.') + QStringLiteral(".") + executable;
}

QT_END_NAMESPACE
