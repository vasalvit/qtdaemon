#ifndef QDAEMONAPPLICATIONPRIVATE_H
#define QDAEMONAPPLICATIONPRIVATE_H

#include "qdaemon-global.h"
#include "qdaemonbackend.h"

#include <QCommandLineParser>
#include <QCommandLineOption>

QT_BEGIN_NAMESPACE

class QDaemonApplication;
class Q_DAEMON_EXPORT QDaemonApplicationPrivate
{
	Q_DECLARE_PUBLIC(QDaemonApplication)
public:
	enum Operation  {
		StartOperation = 0x01,
		StopOperation = 0x02,
		InstallOperation = 0x04,
		UninstallOperation = 0x08,
		HelpOperation = 0x10
	};
	Q_DECLARE_FLAGS(Operations, Operation)

	QDaemonApplicationPrivate(QDaemonApplication *);
	~QDaemonApplicationPrivate();

private:
	int exec();

	static void processSignalHandler(int);

private:
	QDaemonApplication * q_ptr;
	QDaemonBackend * backend;
	bool autoQuit;

	class CommandLineOptions
	{
		// TODO: Support prefix paths for installation of dbus conf file and init.d template script
	public:
		CommandLineOptions();

		bool process(const QStringList & arguments);

		QString helpText() const;
		Operations operations() const;

		QString value(const QString &) const;
	private:
		QCommandLineParser parser;
		QCommandLineOption installOption, uninstallOption, startOption, stopOption, helpOption;
		Operations op;
	} commandLine;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDaemonApplicationPrivate::Operations)

QT_END_NAMESPACE

#endif // QDAEMONAPPLICATIONPRIVATE_H
