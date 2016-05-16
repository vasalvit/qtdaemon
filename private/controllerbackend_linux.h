#ifndef CONTROLLERBACKEND_LINUX_H
#define CONTROLLERBACKEND_LINUX_H

#include "qabstractdaemonbackend.h"

#include <QCommandLineOption>

QT_BEGIN_NAMESPACE

class QDBusAbstractInterface;
class Q_DAEMON_LOCAL ControllerBackendLinux : public QAbstractDaemonBackend
{
	Q_DISABLE_COPY(ControllerBackendLinux)

public:
	ControllerBackendLinux(QCommandLineParser &, bool);
	~ControllerBackendLinux() override;

	int exec() override;

	bool start();
	bool stop();
	bool install();
	bool uninstall();

private:
	bool autoQuit;

	QDBusAbstractInterface * getDBusInterface();

	const QCommandLineOption installOption;
	const QCommandLineOption uninstallOption;
	const QCommandLineOption startOption;
	const QCommandLineOption stopOption;
	const QCommandLineOption fakeOption;
	const QCommandLineOption dbusPrefixOption;
	const QCommandLineOption initdPrefixOption;

	static const QString initdPrefix;
	static const QString dbusPrefix;
	static const QString defaultInitPath;
	static const QString defaultDBusPath;
};

QT_END_NAMESPACE

#endif // CONTROLLERBACKEND_LINUX_H
