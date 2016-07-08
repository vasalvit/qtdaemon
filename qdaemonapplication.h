#ifndef QDAEMONAPPLICATION_H
#define QDAEMONAPPLICATION_H

#include "qdaemon-global.h"
#include <QCoreApplication>

QT_BEGIN_NAMESPACE

class QDaemonApplicationPrivate;
class Q_DAEMON_EXPORT QDaemonApplication : public QCoreApplication
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(QDaemonApplication)
	Q_DISABLE_COPY(QDaemonApplication)

public:
	QDaemonApplication(int & argc, char ** argv);
	~QDaemonApplication() override;

	static int exec();
	static QDaemonApplication * instance();

	bool autoQuit() const;
	void setAutoQuit(bool);

	static QString applicationDescription();
	static void setApplicationDescription(const QString &);

signals:
	void daemonized(const QStringList &);

	void started();
	void stopped();
	void installed();
	void uninstalled();

private:
	QDaemonApplicationPrivate * d_ptr;
};

QT_END_NAMESPACE

#endif // QDAEMONAPPLICATION_H
