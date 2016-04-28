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
	static const QString dbusPrefix;
	static const QString initdPrefix;

	BackendUnix();
	~BackendUnix() override;

	bool initialize() override;
	bool finalize() override;

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
	~DaemonBackendUnix() override;

	Q_INVOKABLE bool isRunning();

	bool initialize() override;
	bool finalize() override;

	bool start() override;
	Q_INVOKABLE bool stop() override;
	bool install() override;
	bool uninstall() override;
};

class Q_DAEMON_LOCAL ControllerBackendUnix : public BackendUnix
{
	Q_DISABLE_COPY(ControllerBackendUnix)

public:
	ControllerBackendUnix();
	~ControllerBackendUnix() override;

	void setArguments(const Arguments &) override;

	bool initialize() override;
	bool finalize() override;

	bool start() override;
	bool stop() override;
	bool install() override;
	bool uninstall() override;

protected:
	QDBusAbstractInterface * getInterface();

private:
	QPointer<QDBusAbstractInterface> dbusInterface;
	Arguments arguments;
};

QT_END_NAMESPACE

#endif // QDAEMONBACKEND_UNIX_H
