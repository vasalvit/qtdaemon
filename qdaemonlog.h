#ifndef QDAEMONLOG_H
#define QDAEMONLOG_H

#include "qdaemon-global.h"

// TODO: Possibly make the logger sync between different processes (controller and daemon)
// All methods (without the constructor and destructor) are thread-safe.

class QDaemonLogPrivate;
class Q_DAEMON_EXPORT QDaemonLog
{
	Q_DISABLE_COPY(QDaemonLog)

public:
	enum EntrySeverity  {
		NoticeEntry, WarningEntry, ErrorEntry
	};
	enum LogType { LogToStdout, LogToFile };

	QDaemonLog();
	~QDaemonLog();

	void setLogType(LogType);
	LogType logType() const;

	QDaemonLog & operator << (const QString &);

	friend Q_DAEMON_EXPORT QDaemonLog & qDaemonLog();
	friend Q_DAEMON_EXPORT void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity);

private:
	QDaemonLogPrivate * d_ptr;
};

// --- Friend declarations ---------------------------------------------------------------------------------------------- //
Q_DAEMON_EXPORT QDaemonLog & qDaemonLog();
Q_DAEMON_EXPORT void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity = QDaemonLog::NoticeEntry);
// ---------------------------------------------------------------------------------------------------------------------- //

#endif // QDAEMONLOG_H
