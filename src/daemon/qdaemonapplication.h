#ifndef QDAEMONAPPLICATION_H
#define QDAEMONAPPLICATION_H

#include "QtDaemon/qdaemon-global.h"

#include <QtCore/qcoreapplication.h>

QT_BEGIN_NAMESPACE

class QDaemonApplicationPrivate;
class Q_DAEMON_EXPORT QDaemonApplication : public QCoreApplication
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDaemonApplication)
    Q_DISABLE_COPY(QDaemonApplication)

    Q_PROPERTY(bool autoQuit READ autoQuit WRITE setAutoQuit)
    Q_PROPERTY(QString applicationDescription READ applicationDescription WRITE setApplicationDescription)

public:
	QDaemonApplication(int & argc, char ** argv);
	~QDaemonApplication() override;

	static int exec();
	static QDaemonApplication * instance();

	bool autoQuit() const;
	void setAutoQuit(bool);

	static QString applicationDescription();
	static void setApplicationDescription(const QString &);

Q_SIGNALS:
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
