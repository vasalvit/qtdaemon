#include "qabstractdaemonbackend.h"

QT_BEGIN_NAMESPACE

const int QAbstractDaemonBackend::BackendFailed = -1;

QAbstractDaemonBackend::QAbstractDaemonBackend(QCommandLineParser & prsr)
	: parser(prsr)
{
}

QAbstractDaemonBackend::~QAbstractDaemonBackend()
{
}

QT_END_NAMESPACE
