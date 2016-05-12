#include "qdaemonlog.h"
#include "private/qdaemonlog_p.h"

QDaemonLog::QDaemonLog()
	: d_ptr(new QDaemonLogPrivate)
{
	if (Q_LIKELY(!QDaemonLogPrivate::logger))
		QDaemonLogPrivate::logger = this;
	else
		qWarning("More than one instance of QDaemonLog has been created!");
}

QDaemonLog::~QDaemonLog()
{
	delete d_ptr;
}

QDaemonLog & qDaemonLog()
{
	Q_ASSERT(QDaemonLogPrivate::logger);
	return *QDaemonLogPrivate::logger;
}

void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity)
{
	Q_ASSERT(QDaemonLogPrivate::logger);
	QDaemonLogPrivate::logger->d_ptr->write(message, severity);
}

QDaemonLog & QDaemonLog::operator << (const QString & message)
{
	Q_ASSERT(QDaemonLogPrivate::logger);
	QDaemonLogPrivate::logger->d_ptr->write(message, QDaemonLog::NoticeEntry);
	return *QDaemonLogPrivate::logger;
}
