#ifndef CONTROLLERBACKEND_WIN_H
#define CONTROLLERBACKEND_WIN_H

#include "qabstractdaemonbackend.h"

#include <QCommandLineOption>

#ifndef UNICODE
#error Enable unicode support for your compiler.
#endif

QT_BEGIN_NAMESPACE

class Q_DAEMON_LOCAL ControllerBackendWindows : public QAbstractControllerBackend
{
	Q_DISABLE_COPY(ControllerBackendWindows)

public:
	ControllerBackendWindows(QCommandLineParser &, bool);

	bool start() override;
	bool stop() override;
	bool install() override;
	bool uninstall() override;

private:
	const QCommandLineOption updatePathOption;
};

QT_END_NAMESPACE

#endif // CONTROLLERBACKEND_WIN_H

