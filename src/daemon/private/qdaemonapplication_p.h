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

#ifndef QDAEMONAPPLICATIONPRIVATE_H
#define QDAEMONAPPLICATIONPRIVATE_H

#include "qdaemon-global.h"
#include "qdaemonlog.h"

#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcommandlineoption.h>

QT_BEGIN_NAMESPACE

namespace QtDaemon
{
    class QAbstractDaemonBackend;
}

class QDaemonApplication;
class Q_DAEMON_EXPORT QDaemonApplicationPrivate
{
    Q_DECLARE_PUBLIC(QDaemonApplication)
public:
    QDaemonApplicationPrivate(QDaemonApplication *);
    ~QDaemonApplicationPrivate();

private:
    int exec();

    static void processSignalHandler(int);

private:
    QtDaemon::QAbstractDaemonBackend * createBackend(bool);

private:
    QDaemonApplication * q_ptr;
    QDaemonLog log;
    bool autoQuit;
    QCommandLineParser parser;

    static QString description;
};

QT_END_NAMESPACE

#endif // QDAEMONAPPLICATIONPRIVATE_H
