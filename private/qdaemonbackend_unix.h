#ifndef QDAEMONBACKEND_UNIX_H
#define QDAEMONBACKEND_UNIX_H

#include "qdaemonbackend.h"

#include <QObject>
#include <QTextStream>
#include <QPointer>

#include <QDBusConnection>
#include <QDBusAbstractAdaptor>
#include <QDBusAbstractInterface>

#define Q_DAEMON_DBUS_CONTROL_INTERFACE "io.qt.QtDaemon.Control"

QT_BEGIN_NAMESPACE

class Q_DAEMON_LOCAL BackendUnix : public QDaemonBackend
{
	Q_DISABLE_COPY(BackendUnix)

public:
	BackendUnix();
	virtual ~BackendUnix();

	virtual bool initialize();
	virtual bool finalize();

	static QDaemonBackend * create(BackendType);

protected:
	QString serviceName;
	QDBusConnection dbus;
	QTextStream out;
};

// TODO: Provide file based logging for the daemon backend
class Q_DAEMON_LOCAL DaemonBackendUnix : public QObject, public BackendUnix
{
	Q_OBJECT
	Q_DISABLE_COPY(DaemonBackendUnix)
	Q_CLASSINFO("D-Bus Interface", Q_DAEMON_DBUS_CONTROL_INTERFACE)

public:
	DaemonBackendUnix();
	virtual ~DaemonBackendUnix();

	Q_INVOKABLE bool isRunning();

	virtual bool initialize();
	virtual bool finalize();

	virtual bool start();
	Q_INVOKABLE virtual bool stop();
	virtual bool install();
	virtual bool uninstall();
};

class Q_DAEMON_LOCAL ControllerBackendUnix : public BackendUnix
{
	Q_DISABLE_COPY(ControllerBackendUnix)

public:
	ControllerBackendUnix();
	virtual ~ControllerBackendUnix();

	virtual bool initialize();
	virtual bool finalize();

	virtual bool start();
	virtual bool stop();
	virtual bool install();
	virtual bool uninstall();

protected:
	QDBusAbstractInterface * getInterface();

private:
	QPointer<QDBusAbstractInterface> dbusInterface;
};

QT_END_NAMESPACE

#endif // QDAEMONBACKEND_UNIX_H
