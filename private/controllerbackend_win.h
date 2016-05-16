#ifndef CONTROLLERBACKEND_WIN_H
#define CONTROLLERBACKEND_WIN_H

#include "qabstractdaemonbackend.h"

#include <QCommandLineOption>

#ifndef UNICODE
#error Enable unicode support for your compiler.
#endif

QT_BEGIN_NAMESPACE

class Q_DAEMON_LOCAL ControllerBackendWindows : public QAbstractDaemonBackend
{
	Q_DISABLE_COPY(ControllerBackendWindows)

public:
	ControllerBackendWindows(QCommandLineParser &, bool);
	~ControllerBackendWindows() override;

	int exec() override;

	bool start();
	bool stop();
	bool install();
	bool uninstall();

private:
	bool autoQuit;

	const QCommandLineOption installOption;
	const QCommandLineOption uninstallOption;
	const QCommandLineOption startOption;
	const QCommandLineOption stopOption;
	const QCommandLineOption fakeOption;
	const QCommandLineOption updatePathOption;
};

QT_END_NAMESPACE

#endif // CONTROLLERBACKEND_WIN_H

