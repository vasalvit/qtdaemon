#ifndef QDAEMONLOG_P_H
#define QDAEMONLOG_P_H

#include "qdaemonlog.h"

#include <QFile>
#include <QTextStream>
#include <QMutex>

QT_BEGIN_NAMESPACE

class QDaemonLogPrivate
{
	friend class QDaemonLog;
	friend QDaemonLog & qDaemonLog();
	friend void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity);

public:
	QDaemonLogPrivate();
	~QDaemonLogPrivate();

	void write(const QString &, QDaemonLog::EntrySeverity);

private:
	QString logFilePath;
	QFile logFile;
	QTextStream logStream;
	QDaemonLog::LogType logType;

	QMutex streamMutex;

	static QDaemonLog * logger;
};

QT_END_NAMESPACE

#endif // QDAEMONLOG_P_H
