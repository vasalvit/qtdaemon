#ifndef DAEMONBACKEND_LINUX_H
#define DAEMONBACKEND_LINUX_H

#include "QtDaemon/qabstractdaemonbackend.h"

#include <QtCore/qobject.h>

#define Q_DAEMON_DBUS_CONTROL_INTERFACE "io.qt.QtDaemon.Control"

QT_BEGIN_NAMESPACE

namespace QtDaemon
{
    class Q_DAEMON_LOCAL DaemonBackendLinux : public QObject, public QAbstractDaemonBackend
    {
        Q_OBJECT
        Q_DISABLE_COPY(DaemonBackendLinux)
        Q_CLASSINFO("D-Bus Interface", Q_DAEMON_DBUS_CONTROL_INTERFACE)

    public:
        DaemonBackendLinux(QCommandLineParser &);
        ~DaemonBackendLinux() override;

        int exec() override;

        Q_INVOKABLE bool isRunning();
        Q_INVOKABLE bool stop();

        static QString serviceName();
    };
}

QT_END_NAMESPACE

#endif // DAEMONBACKEND_LINUX_H
