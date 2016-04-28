#include "qdaemonbackend_unix.h"

#include <QCoreApplication>
#include <QProcess>
#include <QThread>
#include <QDir>
#include <QFile>

#include <QDBusError>
#include <QDBusInterface>
#include <QDBusReply>

QT_BEGIN_NAMESPACE

// -------------------------------------------------------------------------------------------------------------------------------------------------------- //
// --- BackendUnix ---------------------------------------------------------------------------------------------------------------------------------------- //
// -------------------------------------------------------------------------------------------------------------------------------------------------------- //

const QString BackendUnix::dbusPrefix = QStringLiteral("dbus-prefix");
const QString BackendUnix::initdPrefix = QStringLiteral("initd-prefix");

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
		return nullptr;
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
	: dbusInterface(nullptr)
{
}

ControllerBackendUnix::~ControllerBackendUnix()
{
	delete dbusInterface;
}

void ControllerBackendUnix::setArguments(const Arguments & args)
{
	arguments = args;
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
	QString dbusPath = arguments.value(dbusPrefix), initdPath = arguments.value(initdPrefix);

	// Sanity check for the paths and permissions of the provided directories
	if (dbusPath.isEmpty() || initdPath.isEmpty())  {
		out << QStringLiteral("The provided D-Bus path and/or init.d path can't be empty");
		return false;
	}

	QFile dbusConf(QDir(dbusPath).filePath(serviceName + QStringLiteral(".conf"))), initdFile(QDir(initdPath).filePath(serviceName));
	if (dbusConf.exists())  {
		out << QStringLiteral("The provided D-Bus configuration directory already contains a configuration for this service. Uninstall first") << endl;
		return false;
	}
	if (initdFile.exists())  {
		out << QStringLiteral("The provided init.d directory already contains a script for this service. Uninstall first") << endl;
		return false;
	}

	bool status = dbusConf.open(QFile::WriteOnly | QFile::Text) && initdFile.open(QFile::WriteOnly | QFile::Text);
	if (!status)  {
		out << QStringLiteral("Couldn't open the D-Bus configuration file and/or the init.d script for writing. Check the permissions for the D-Bus configuration directory and the init.d script directory.") << endl;

		// Clean up any created files
		dbusConf.remove();
		initdFile.remove();

		return false;
	}

	// We have opened both files, read the templates
	QFile dbusTemplate(":/resources/dbus"), initdTemplate(":/resources/init");
	status = dbusTemplate.open(QFile::ReadOnly | QFile::Text) && initdTemplate.open(QFile::ReadOnly | QFile::Text);

	Q_ASSERT(status);		// We don't expect resources to be unaccessible, but who knows ...
	if (!status)  {
		out << QStringLiteral("Couldn't read the daemon's resources!");
		return false;
	}

	// Read the dbus configuration, do the substitution and write to disk
	QTextStream fin(&dbusTemplate), fout(&dbusConf);
	QString data = fin.readAll();
	data.replace(QStringLiteral("%%SERVICE_NAME%%"), serviceName);
	fout << data;

	if (fout.status() != QTextStream::Ok)  {
		out << QStringLiteral("An error occured while writing the D-Bus configuration. Installation may be broken");
		fout.resetStatus();
	}

	// Switch IO devices
	fin.setDevice(&initdTemplate);
	fout.setDevice(&initdFile);

	// Read the init.d script, do the substitution and write to disk
	data = fin.readAll();
	data.replace(QStringLiteral("%%DAEMON%%"), QCoreApplication::applicationFilePath());
	fout << data;

	if (fout.status() != QTextStream::Ok)
		out << QStringLiteral("An error occured while writing the init.d script. Installation may be broken");

	return true;
}

bool ControllerBackendUnix::uninstall()
{
	QString dbusPath = arguments.value(dbusPrefix), initdPath = arguments.value(initdPrefix);

	QFile dbusConf(QDir(dbusPath).filePath(serviceName + QStringLiteral(".conf"))), initdFile(QDir(initdPath).filePath(serviceName));
	if (dbusConf.exists() && !dbusConf.remove())  {
		out << QStringLiteral("Couldn't remove the D-Bus configuration file for this service. Check your permissions.") << endl;
		return false;
	}
	if (initdFile.exists() && !initdFile.remove())  {
		out << QStringLiteral("Couldn't remove the init.d script for this service. Check your permissions.") << endl;
		return false;
	}

	return true;
}

QDBusAbstractInterface * ControllerBackendUnix::getInterface()
{
	if (!dbusInterface)		// Acquire the D-Bus interface
		dbusInterface = new QDBusInterface(serviceName, QStringLiteral("/"), QStringLiteral(Q_DAEMON_DBUS_CONTROL_INTERFACE), dbus);

	// Wait for the daemon to notify us so we know everything is okay
	if (!dbusInterface->isValid())  {
		delete dbusInterface;
		dbusInterface = nullptr;
	}

	return dbusInterface;
}

QT_END_NAMESPACE
