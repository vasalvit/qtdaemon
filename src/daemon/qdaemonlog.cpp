#include "qdaemonlog.h"
#include "private/qdaemonlog_p.h"

#include <QtCore/QMutexLocker>

QT_BEGIN_NAMESPACE

/*!
    \class QDaemonLog
    \inmodule QDaemon

    \brief The \l{QDaemonLog} class provides basic logging facilities for
    daemon applications.

    It's set up to output on the standard stream when the application is run as controlling terminal,
    and to a file when the application is ran as daemon/service.

    \threadsafe
*/

/*!
    \enum QDaemonLog::EntrySeverity

    This enum is used to specify the severity of the log message.

    \value NoticeEntry  The entry is a notice.
    \value WarningEntry The entry is a warning. Usually used when non-critical errors occur.
    \value ErrorEntry   The entry is an error. Usually used with critical errors.

    \sa qDaemonLog(const QString &, QDaemonLog::EntrySeverity severity)
*/

/*!
    \enum QDaemonLog::LogType

    This enum specifies the type of the log.

    \value LogToStdout  The messages are written to the standard output stream.
    \value LogToFile    The messages are written to a regular file.
                        The file is created in the application's directory if it doesn't exist.
                        The name of the file is constructed from the base name of the executable by appending a .log extension.
*/

/*!
    \internal
*/
QDaemonLog::QDaemonLog(QDaemonLogPrivate & d)
    : d_ptr(&d)
{
    Q_ASSERT(!QDaemonLogPrivate::logger);
    QDaemonLogPrivate::logger = this;
}

/*!
    \internal
*/
QDaemonLog::~QDaemonLog()
{
    QDaemonLogPrivate::logger = nullptr;
    delete d_ptr;
}

/*!
    Sets the log type to \a type.

    \sa logType(), QDaemonLog::LogType
*/
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

/*!
    Retrieves the currently used log type.

    \sa setLogType(), QDaemonLog::LogType
*/
QDaemonLog::LogType QDaemonLog::logType() const
{
    QMutexLocker lock(&d_ptr->streamMutex);	// The MS compiler doesn't get anonymous objects (error C2530: references must be initialized)
    Q_UNUSED(lock);							// Suppress warning for unused variable

    return d_ptr->logType;
}

/*!
    Writes the message specified by \a message to the log.

    \sa qDaemonLog(const QString &, QDaemonLog::EntrySeverity)
*/
QDaemonLog & QDaemonLog::operator << (const QString & message)
{
    QMutexLocker lock(&d_ptr->streamMutex);	// The MS compiler doesn't get anonymous objects (error C2530: references must be initialized)
    Q_UNUSED(lock);                         // Suppress warning for unused variable

    d_ptr->write(message, QDaemonLog::NoticeEntry);
    return *this;
}

/*!
    \fn QDaemonLog & qDaemonLog()

    Retrieves the logger instance.

    \warning The daemon log can be retrieved only after the QDaemonApplication instance has been created.

    \sa qDaemonLog(), operator << (const QString &)
*/
QDaemonLog & qDaemonLog()
{
    Q_ASSERT(QDaemonLogPrivate::logger);
    return *QDaemonLogPrivate::logger;
}

/*!
    \fn void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity)

    \overload qDaemonLog()

    Writes a message specified by \a message to the log with a severity given by \a severity.

    \sa qDaemonLog(), operator << (const QString &)
*/
void qDaemonLog(const QString & message, QDaemonLog::EntrySeverity severity)
{
    Q_ASSERT(QDaemonLogPrivate::logger);

    QDaemonLogPrivate * const d = QDaemonLogPrivate::logger->d_ptr;

    QMutexLocker lock(&d->streamMutex); // The MS compiler doesn't get anonymous objects (error C2530: references must be initialized)
    Q_UNUSED(lock);                     // Suppress warning for unused variable

    d->write(message, severity);
}

QT_END_NAMESPACE
