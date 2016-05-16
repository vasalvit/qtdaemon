#ifndef QABSTRACTDAEMONBACKEND_H
#define QABSTRACTDAEMONBACKEND_H

#include "qdaemon-global.h"
#include <QStringList>

QT_BEGIN_NAMESPACE

class QCommandLineParser;
class Q_DAEMON_LOCAL QAbstractDaemonBackend
{
	Q_DISABLE_COPY(QAbstractDaemonBackend)

public:
	static const int BackendFailed;

public:
	QAbstractDaemonBackend(QCommandLineParser &);
	virtual ~QAbstractDaemonBackend();

	virtual int exec() = 0;

protected:
	QCommandLineParser & parser;
};

QT_END_NAMESPACE

#endif // QABSTRACTDAEMONBACKEND_H
