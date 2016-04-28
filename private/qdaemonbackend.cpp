#include "qdaemonbackend.h"

QT_BEGIN_NAMESPACE

QDaemonBackend::QDaemonBackend()
{
}

QDaemonBackend::~QDaemonBackend()
{
}

void QDaemonBackend::setArguments(const Arguments &)
{
	// Do nothing, subclasses should handle those themselves
}

QT_END_NAMESPACE
