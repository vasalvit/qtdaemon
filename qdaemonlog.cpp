#include "qdaemonlog.h"
#include "private/qdaemonlog_p.h"

#include <QMutexLocker>

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
	QDaemonLogPrivate::logger = nullptr;
	delete d_ptr;
}

void QDaemonLog::setLogType(LogType type)
{
	QMutexLocker lock(&d_ptr->streamMutex);
	Q_UNUSED(lock);

	if (type == d_ptr->logType)
		return;

	bool failed = false;
	switch (type)
	{
	case LogToFile:
		d_ptr->logFile.close();
		d_ptr->logFile.setFileName(d_ptr->logFilePath);
		// Try opening the file
		if (d_ptr->logFile.open(QFile::WriteOnly | QFile::Text | QFile::Append))  {
			d_ptr->logType = LogToFile;
			break;
		}

		// File couldn't be open. Try to fall back to the standard output
		failed = true;
	case LogToStdout:
	default:
		d_ptr->logFile.close();
		if (Q_UNLIKELY(!d_ptr->logFile.open(stdout, QFile::WriteOnly | QFile::Text)))  {
			qWarning("Error while trying to open the standard output. Giving up!");
			break;
		}

		d_ptr->logType = LogToStdout;
		if (failed)  // Report that a file couldn't be opened
			d_ptr->write(QStringLiteral("The log file %1 couldn't be opened for writing! Switched to stdout.").arg(d_ptr->logFilePath), WarningEntry);
	}
}

QDaemonLog::LogType QDaemonLog::logType() const
{
	QMutexLocker lock(&d_ptr->streamMutex);	// The MS compiler doesn't get anonymous objects (error C2530: references must be initialized)
	Q_UNUSED(lock);							// Suppress warning for unused variable

	return d_ptr->logType;
}

QDaemonLog & QDaemonLog::operator << (const QString & message)
{
	QMutexLocker lock(&d_ptr->streamMutex);	// The MS compiler doesn't get anonymous objects (error C2530: references must be initialized)
	Q_UNUSED(lock);						// Suppress warning for unused variable

	d_ptr->write(message, QDaemonLog::NoticeEntry);
	return *this;
}

QDaemonLog & qDaemonLog()
{
	Q_ASSERT(QDaemonLogPrivate::logger);
	return *QDaemonLogPrivate::logger;
}

void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity)
{
	Q_ASSERT(QDaemonLogPrivate::logger);

	QDaemonLogPrivate * const d = QDaemonLogPrivate::logger->d_ptr;

	QMutexLocker lock(&d->streamMutex);	// The MS compiler doesn't get anonymous objects (error C2530: references must be initialized)
	Q_UNUSED(lock);							// Suppress warning for unused variable

	d->write(message, severity);
}
