//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QGLWidget class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDAEMONLOG_P_H
#define QDAEMONLOG_P_H

#include "qdaemonlog.h"

#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qmutex.h>

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
