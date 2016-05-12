#include "qdaemonlog_p.h"
#include "qdaemonlog.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QMutexLocker>

QDaemonLog * QDaemonLogPrivate::logger = NULL;

QDaemonLogPrivate::QDaemonLogPrivate()
	: logStream(stdout)
{
	// Get the log file path
	QFileInfo info(QCoreApplication::applicationFilePath());
	QString logFilePath = info.absoluteDir().filePath(info.completeBaseName() + QStringLiteral(".log"));

	logFile.setFileName(logFilePath);

	// Try opening the file
	if (logFile.open(QFile::WriteOnly | QFile::Text | QFile::Append))
		logStream.setDevice(&logFile);
	else
		write(QStringLiteral("The log file %1 couldn't be opened for writing!").arg(logFilePath), QDaemonLog::ErrorEntry);
}

QDaemonLogPrivate::~QDaemonLogPrivate()
{
	logStream.flush();
	logFile.close();
}

void QDaemonLogPrivate::write(const QString & message, QDaemonLog::EntrySeverity severity)
{
	static const QString noticeEntry = QStringLiteral("%1 > %2");
	static const QString warningEntry = QStringLiteral("%1 > Warning: %2");
	static const QString errorEntry = QStringLiteral("%1 > Error: %2");

	QString formattedMessage, date = QDateTime::currentDateTime().toString(Qt::ISODate);
	switch (severity)
	{
	case QDaemonLog::ErrorEntry:
		formattedMessage = errorEntry.arg(date).arg(message);
		break;
	case QDaemonLog::WarningEntry:
		formattedMessage = warningEntry.arg(date).arg(message);
		break;
	case QDaemonLog::NoticeEntry:
	default:
		formattedMessage = noticeEntry.arg(date).arg(message);
	}

	QMutexLocker lock(&streamMutex);	// The MS compiler doesn't get anonymous objects (error C2530: references must be initialized)
	Q_UNUSED(lock);						// Suppress warning for unused variable

	logStream << formattedMessage << endl;
}
