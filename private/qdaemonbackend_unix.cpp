#include "qdaemonbackend_unix.h"

#include <QCoreApplication>
#include <QProcess>
#include <QThread>

#include <QDBusError>
#include <QDBusInterface>
#include <QDBusReply>

QT_BEGIN_NAMESPACE

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
	else
		serviceName = QStringLiteral("io.qt");

	QString path = QCoreApplication::applicationFilePath();
	serviceName += QStringLiteral(".QtDaemon-") + QString::number(qChecksum(qPrintable(path), path.length()));
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
		out << QStringLiteral("Can't connect to the D-Bus system bus") << endl;
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
		out << QStringLiteral("Couldn't register a service with the D-Bus system bus. Error: ") + dbus.lastError().message() << endl;
		return false;
	}

	// Register the object
	if (!dbus.registerObject(QStringLiteral("/"), this, QDBusConnection::ExportAllInvokables))  {
		out << QStringLiteral("Couldn't register an object with the D-Bus system bus. Error: ") + dbus.lastError().message() << endl;
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
		out << QStringLiteral("Can't unregister service from D-bus. Error: ") + dbus.lastError().message() << endl;

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
	// First check if the daemon is already running
	QDBusAbstractInterface * interface = getInterface();
	if (interface)  {
		QDBusReply<bool> reply = interface->call("isRunning");
		if (reply.isValid() && reply.value())
			out << QStringLiteral("The daemon is already running") << endl;
		else
			out << QStringLiteral("The daemon is not responding.") << endl;

		return false;
	}

	// The daemon is (most probably) not running, so start it
	if (!QProcess::startDetached(QCoreApplication::applicationFilePath()))
		return false;

	QThread::sleep(1);		// Give the daemon a second to start it's D-Bus service

	// Repeat the call to make sure the communication is Ok
	interface = getInterface();
	if (!interface)  {
		out << QStringLiteral("Connection with the daemon couldn't be established. Error: ") + dbus.lastError().message() << endl;
		return false;
	}

	QDBusReply<bool> reply = interface->call("isRunning");
	if (!reply.isValid())  {
		out << QStringLiteral("The acquired D-Bus interface replied erroneously. Error: ") + dbus.lastError().message() << endl;
		return false;
	}

	return reply.value();
}

bool ControllerBackendUnix::stop()
{
	// Get the D-Bus interface
	QDBusAbstractInterface * interface = getInterface();
	if (!interface)  {
		out << QStringLiteral("Daemon is not responding. Is it running?") << endl;
		return false;
	}

	QDBusReply<bool> reply = interface->call("stop");
	if (!reply.isValid())  {
		out << QStringLiteral("The acquired D-Bus interface replied erroneously. Error: ") + dbus.lastError().message() << endl;
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
		dbusInterface = new QDBusInterface(serviceName, QStringLiteral("/"), QStringLiteral(Q_DAEMON_DBUS_CONTROL_INTERFACE), dbus);

	// Wait for the daemon to notify us so we know everything is okay
	if (!dbusInterface->isValid())  {
		delete dbusInterface;
		dbusInterface = NULL;
	}

	return dbusInterface;
}

QT_END_NAMESPACE
