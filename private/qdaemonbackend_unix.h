#ifndef QDAEMONBACKEND_UNIX_H
#define QDAEMONBACKEND_UNIX_H

#include "qdaemonbackend.h"

#include <QObject>
#include <QTextStream>

#include <QDBusConnection>
#include <QDBusAbstractInterface>

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

class Q_DAEMON_LOCAL DaemonBackendUnix : public QObject, public BackendUnix
{
	Q_OBJECT
	Q_DISABLE_COPY(DaemonBackendUnix)

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
	QDBusAbstractInterface * dbusInterface;
};

QT_END_NAMESPACE

#endif // QDAEMONBACKEND_UNIX_H
