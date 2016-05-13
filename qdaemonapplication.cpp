#include "qdaemonapplication.h"
#include "private/qdaemonapplication_p.h"

QT_BEGIN_NAMESPACE

QDaemonApplication::QDaemonApplication(int & argc, char ** argv)
	: QCoreApplication(argc, argv), d_ptr(new QDaemonApplicationPrivate(this))
{
}

QDaemonApplication::~QDaemonApplication()
{
	delete d_ptr;
}

int QDaemonApplication::exec()
{
	// Just check if someone forgot to create the application object
	QDaemonApplication * app = QDaemonApplication::instance();
	if (Q_UNLIKELY(!app))  {
		qWarning("You must construct a QDaemonApplication before calling QDaemonApplication::exec.");
		return -1;
	}

	return app->d_ptr->exec();
}

QDaemonApplication * QDaemonApplication::instance()
{
	return qobject_cast<QDaemonApplication *>(QCoreApplication::instance());
}

bool QDaemonApplication::autoQuit() const
{
	Q_D(const QDaemonApplication);
	return d->autoQuit;
}

void QDaemonApplication::setAutoQuit(bool aquit)
{
	Q_D(QDaemonApplication);
	d->autoQuit = aquit;
}

QString QDaemonApplication::applicationDescription()
{
	return QDaemonApplicationPrivate::description;
}

void QDaemonApplication::setApplicationDescription(const QString & description)
{
	QDaemonApplicationPrivate::description = description;
}

QT_END_NAMESPACE
