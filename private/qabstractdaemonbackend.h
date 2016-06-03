#ifndef QABSTRACTDAEMONBACKEND_H
#define QABSTRACTDAEMONBACKEND_H

#include "qdaemon-global.h"
#include <QStringList>
#include <QCommandLineOption>

QT_BEGIN_NAMESPACE

class QCommandLineParser;

namespace QtDaemon
{
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

	class Q_DAEMON_LOCAL QAbstractControllerBackend : public QAbstractDaemonBackend
	{
		Q_DISABLE_COPY(QAbstractControllerBackend)

	public:
		QAbstractControllerBackend(QCommandLineParser &, bool);

		int exec() override;

		virtual bool start() = 0;
		virtual bool stop() = 0;
		virtual bool install() = 0;
		virtual bool uninstall() = 0;

	protected:
		bool autoQuit;

		const QCommandLineOption installOption;
		const QCommandLineOption uninstallOption;
		const QCommandLineOption startOption;
		const QCommandLineOption stopOption;
		const QCommandLineOption fakeOption;
	};
}

QT_END_NAMESPACE

#endif // QABSTRACTDAEMONBACKEND_H
