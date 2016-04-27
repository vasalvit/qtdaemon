#include "qdaemonbackend_unix.h"

#include <QCoreApplication>
#include <QProcess>

#include <QDBusError>
#include <QDBusInterface>
#include <QDBusReply>

// -------------------------------------------------------------------------------------------------------------------------------------------------------- //
// --- BackendUnix ---------------------------------------------------------------------------------------------------------------------------------------- //
// -------------------------------------------------------------------------------------------------------------------------------------------------------- //

BackendUnix::BackendUnix()
	: QDaemonBackend(), dbus(QStringLiteral("QtDaemon")), out(stdout)
{
	// Get the service name
	QString domain = QCoreApplication::organizationDomain();
	if (!domain.isEmpty())  {
		QStringList elements = domain.split('.');
		std::reverse(elements.begin(), elements.end());
		serviceName = elements.join('.') + QStringLiteral(".");
	}

	QString path = QCoreApplication::applicationFilePath();
	serviceName += QStringLiteral("QtDaemon-") + QString::number(qChecksum(qPrintable(path), path.length()));
}

BackendUnix::~BackendUnix()
{
}

bool BackendUnix::initialize()
{
	if (dbus.isConnected())
		return true;

	dbus = QDBusConnection::systemBus();
	if (!dbus.isConnected())  {
		out << QStringLiteral("Can't connect to the D-Bus system bus");
		return false;
	}

	return true;
}

bool BackendUnix::finalize()
{
	return true;		// Nothing to do (the D-bus connection is closed automatically by Qt)
}


QDaemonBackend * BackendUnix::create(BackendType type)
{
	switch (type)
	{
	case QDaemonBackend::ControllerType:
		return new ControllerBackendUnix();
	case QDaemonBackend::DaemonType:
		return new DaemonBackendUnix();
	default:
		Q_ASSERT(false);
		return NULL;
	}
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------- //
// --- DaemonBackendUnix ---------------------------------------------------------------------------------------------------------------------------------- //
// -------------------------------------------------------------------------------------------------------------------------------------------------------- //

DaemonBackendUnix::DaemonBackendUnix()
{
}

DaemonBackendUnix::~DaemonBackendUnix()
{
}

bool DaemonBackendUnix::isRunning()
{
	return true;	// This is just for notifying the controlling process. The function is invoked over D-Bus only.
}

bool DaemonBackendUnix::initialize()
{
	if (!BackendUnix::initialize())
		return false;

	if (!dbus.registerService(serviceName))  {
		out << QStringLiteral("Couldn't register a service with the D-Bus system bus. Error: ") + dbus.lastError().message();
		return false;
	}

	// Register the object
	if (!dbus.registerObject(QStringLiteral("/"), this, QDBusConnection::ExportAllInvokables))  {
		out << QStringLiteral("Couldn't register an object with the D-Bus system bus. Error: ") + dbus.lastError().message();
		return false;
	}

	return true;
}

bool DaemonBackendUnix::finalize()
{
	if (!dbus.isConnected())
		return false;

	dbus.unregisterObject(QStringLiteral("/"));

	if (!dbus.unregisterService(serviceName))
		out << QStringLiteral("Can't unregister service from D-bus. Error: ") + dbus.lastError().message();

	return BackendUnix::finalize();
}

bool DaemonBackendUnix::start()
{
	Q_ASSERT(false);
	return false;
}

bool DaemonBackendUnix::stop()
{
	qApp->quit();
	return true;
}

bool DaemonBackendUnix::install()
{
	Q_ASSERT(false);
	return false;
}

bool DaemonBackendUnix::uninstall()
{
	Q_ASSERT(false);
	return false;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------- //
// --- ControllerBackendUnix ------------------------------------------------------------------------------------------------------------------------------ //
// -------------------------------------------------------------------------------------------------------------------------------------------------------- //

ControllerBackendUnix::ControllerBackendUnix()
	: dbusInterface(NULL)
{
}

ControllerBackendUnix::~ControllerBackendUnix()
{
	delete dbusInterface;
}

bool ControllerBackendUnix::initialize()
{
	if (!BackendUnix::initialize())
		return false;

	return true;
}

bool ControllerBackendUnix::finalize()
{
	return true;
}

bool ControllerBackendUnix::start()
{
	if (!QProcess::startDetached(QCoreApplication::applicationFilePath()))
		return false;

	// Get the D-Bus interface
	QDBusAbstractInterface * interface = getInterface();
	if (!interface)
		return false;

	QDBusReply<bool> reply = interface->call("isRunning");
	if (!reply.isValid())  {
		out << QStringLiteral("The acquired D-Bus interface replied erroneously. Error: ") + dbus.lastError().message();
		return false;
	}

	return reply.value();
}

bool ControllerBackendUnix::stop()
{
	// Get the D-Bus interface
	QDBusAbstractInterface * interface = getInterface();
	if (!interface)
		return false;

	QDBusReply<bool> reply = interface->call("stop");
	if (!reply.isValid())  {
		out << QStringLiteral("The acquired D-Bus interface replied erroneously. Error: ") + dbus.lastError().message();
		return false;
	}

	return reply.value();
}

bool ControllerBackendUnix::install()
{
	Q_ASSERT(false);		// TODO: Provide init.d script
	return false;
}

bool ControllerBackendUnix::uninstall()
{
	Q_ASSERT(false);		// TODO: Remove init.d script
	return false;
}

QDBusAbstractInterface * ControllerBackendUnix::getInterface()
{
	if (!dbusInterface)		// Acquire the D-Bus interface
		dbusInterface = new QDBusInterface(serviceName, QStringLiteral("/"), QStringLiteral(""), dbus);

	// Wait for the daemon to notify us so we know everything is okay
	if (!dbusInterface->isValid())  {
		delete dbusInterface;
		dbusInterface = NULL;

		out << QStringLiteral("Couldn't acquire the daeamon's D-Bus exposed interface. Error: ") + dbus.lastError().message();
	}

	return dbusInterface;
}
