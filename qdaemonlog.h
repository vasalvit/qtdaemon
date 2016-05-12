#ifndef QDAEMONLOG_H
#define QDAEMONLOG_H

#include <QtGlobal>

// TODO: Make the logger sync between different processes (controller and daemon)

class QDaemonLogPrivate;
class QDaemonLog
{
	Q_DISABLE_COPY(QDaemonLog)

public:
	enum EntrySeverity  {
		NoticeEntry, WarningEntry, ErrorEntry
	};

	QDaemonLog();
	~QDaemonLog();

	friend QDaemonLog & qDaemonLog();
	friend void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity);	// Thread-safe

	QDaemonLog & operator << (const QString &);	// Thread-safe

private:
	QDaemonLogPrivate * d_ptr;
};

// --- Friend declarations ---------------------------------------------------------------------------------------------- //

QDaemonLog & qDaemonLog();
void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity = QDaemonLog::NoticeEntry);


#endif // QDAEMONLOG_H
