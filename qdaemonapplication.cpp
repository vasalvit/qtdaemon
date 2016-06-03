#include "qdaemonapplication.h"
#include "private/qdaemonapplication_p.h"
#include "private/qabstractdaemonbackend.h"

#include <QScopedPointer>

QT_BEGIN_NAMESPACE

using namespace QtDaemon;

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

	QStringList arguments = QDaemonApplication::arguments();
	QDaemonApplicationPrivate * const d = app->d_ptr;

	QCommandLineOption daemonOption(QStringLiteral("d"));
	daemonOption.setHidden(true);

	d->parser.addOption(daemonOption);
	d->parser.parse(arguments);

	// Create the appropriate backend
	bool isDaemon = d->parser.isSet(daemonOption);
	QScopedPointer<QAbstractDaemonBackend> backend(d->createBackend(isDaemon));

	// Reparse with the options that the backends may have added in their constructors
	d->parser.parse(arguments);

	return backend->exec();
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
