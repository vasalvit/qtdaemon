#ifndef QDAEMONLOG_P_H
#define QDAEMONLOG_P_H

#include "qdaemonlog.h"

#include <QFile>
#include <QTextStream>
#include <QMutex>

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
	QFile logFile;
	QTextStream logStream;
	QMutex streamMutex;

	static QDaemonLog * logger;
};

#endif // QDAEMONLOG_P_H
