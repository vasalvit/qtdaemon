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

#include "qdaemonapplication_p.h"
#include "qdaemonapplication.h"
#include "qdaemonlog_p.h"

#include <csignal>

#if defined(Q_OS_WIN)
#include "daemonbackend_win.h"
#include "controllerbackend_win.h"
#elif defined(Q_OS_LINUX)
#include "daemonbackend_linux.h"
#include "controllerbackend_linux.h"
#elif defined(Q_OS_OSX)
#include "daemonbackend_osx.h"
#include "controllerbackend_osx.h"
#else
#warning This library is not supported on your platform.
#endif

QT_BEGIN_NAMESPACE

using namespace QtDaemon;

#if defined(Q_OS_WIN)
typedef DaemonBackendWindows DaemonBackend;
typedef ControllerBackendWindows ControllerBackend;
#elif defined(Q_OS_LINUX)
typedef DaemonBackendLinux DaemonBackend;
typedef ControllerBackendLinux ControllerBackend;
#elif defined(Q_OS_OSX)
typedef DaemonBackendOSX DaemonBackend;
typedef ControllerBackendOSX ControllerBackend;
#endif

QString QDaemonApplicationPrivate::description;

QDaemonApplicationPrivate::QDaemonApplicationPrivate(QDaemonApplication * q)
    : q_ptr(q), log(*new QDaemonLogPrivate), autoQuit(true)
{
    std::signal(SIGTERM, QDaemonApplicationPrivate::processSignalHandler);
    std::signal(SIGINT, QDaemonApplicationPrivate::processSignalHandler);
    std::signal(SIGSEGV, QDaemonApplicationPrivate::processSignalHandler);
}

QDaemonApplicationPrivate::~QDaemonApplicationPrivate()
{
}

void QDaemonApplicationPrivate::processSignalHandler(int signalNumber)
{
    switch (signalNumber)
    {
    case SIGSEGV:
       ::exit(-1);
    case SIGTERM:
    case SIGINT:
        {
            QDaemonApplication * app = QDaemonApplication::instance();
            if (app)
                app->quit();
            else
                ::exit(-1);
        }
        break;
    default:
        return;
    }
}

QAbstractDaemonBackend * QDaemonApplicationPrivate::createBackend(bool isDaemon)
{
    if (isDaemon)  {
        log.setLogType(QDaemonLog::LogToFile);
        return new DaemonBackend(parser);
    }
    else
        return new ControllerBackend(parser, autoQuit);
}

QT_END_NAMESPACE
