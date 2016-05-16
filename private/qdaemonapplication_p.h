#ifndef QDAEMONAPPLICATIONPRIVATE_H
#define QDAEMONAPPLICATIONPRIVATE_H

#include "qdaemon-global.h"
#include "qdaemonlog.h"

#include <QCommandLineParser>
#include <QCommandLineOption>

QT_BEGIN_NAMESPACE

class QAbstractDaemonBackend;

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
		HelpOperation = 0x10,
		FakeOperation = 0x20
	};
	Q_DECLARE_FLAGS(Operations, Operation)

	QDaemonApplicationPrivate(QDaemonApplication *);
	~QDaemonApplicationPrivate();

private:
	int exec();

	static void processSignalHandler(int);

private:
	QAbstractDaemonBackend * createBackend(bool);

private:
	QDaemonApplication * q_ptr;
	QDaemonLog log;
	bool autoQuit;

	QCommandLineParser parser;

/*
	class CommandLineOptions
	{
	public:
		CommandLineOptions();

		bool process(const QStringList & arguments);

		QString helpText() const;
		Operations operations() const;

		QString value(const QString &) const;
	private:
		QCommandLineParser parser;
		QCommandLineOption installOption, uninstallOption, startOption, stopOption, fakeOption, helpOption;
		Operations op;
	} commandLine;*/

	static QString description;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDaemonApplicationPrivate::Operations)

QT_END_NAMESPACE

#endif // QDAEMONAPPLICATIONPRIVATE_H
