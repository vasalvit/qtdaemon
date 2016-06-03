#ifndef CONTROLLERBACKEND_LINUX_H
#define CONTROLLERBACKEND_LINUX_H

#include "qabstractdaemonbackend.h"

QT_BEGIN_NAMESPACE

class QDBusAbstractInterface;

namespace QtDaemon
{
	class Q_DAEMON_LOCAL ControllerBackendLinux : public QAbstractControllerBackend
	{
		Q_DISABLE_COPY(ControllerBackendLinux)

	public:
		ControllerBackendLinux(QCommandLineParser &, bool);

		bool start() override;
		bool stop() override;
		bool install() override;
		bool uninstall() override;

	private:
		QDBusAbstractInterface * getDBusInterface();

		const QCommandLineOption dbusPrefixOption;
		const QCommandLineOption initdPrefixOption;

		static const QString initdPrefix;
		static const QString dbusPrefix;
		static const QString defaultInitPath;
		static const QString defaultDBusPath;
	};
}

QT_END_NAMESPACE

#endif // CONTROLLERBACKEND_LINUX_H
