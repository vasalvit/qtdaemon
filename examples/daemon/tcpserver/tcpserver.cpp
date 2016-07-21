#include "tcpserver.h"
#include "tcpsession.h"

#include <QThread>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDaemonLog>

const quint16 TcpServer::defaultPort = 5890;

TcpServer::TcpServer(QObject * parent)
    : QTcpServer(parent)
{
    qRegisterMetaType<qintptr>("qintptr");

}

void TcpServer::start(const QStringList & arguments)
{
    // Parse the command line
    const QCommandLineOption portOption(QStringList() << QStringLiteral("p") << QStringLiteral("port"), QStringLiteral("The port the server will listen to."), QStringLiteral("<port number>"));

    QCommandLineParser parser;
    parser.addOption(portOption);
    parser.parse(arguments);

    // Set the port
    quint16 port = defaultPort;
    if (parser.isSet(portOption))  {
        bool ok;
        quint16 portOptionValue = parser.value(portOption).toInt(&ok);
        if (ok)
            port = portOptionValue;
    }

    // Start the TCP server
    if (!listen(QHostAddress::Any, port))  {
        // We couldn't start the server ...
        // Log the error
        qDaemonLog(QStringLiteral("Couldn't start the server. QTcpServer::listen() failed"), QDaemonLog::ErrorEntry);
        // Give up and quit
        qApp->quit();
        return;
    }
}

void TcpServer::stop()
{
    if (!isListening())
        return;

    close();

    emit stopped();

    // Wait for the remaining session threads to finish
    threadWaitSemaphore.acquire(activeSessions.fetchAndStoreAcquire(0));
}

void TcpServer::threadStarted()
{
    activeSessions.fetchAndAddAcquire(1);
}

void TcpServer::threadFinished()
{
    if (activeSessions.fetchAndSubAcquire(1) <= 0)
        threadWaitSemaphore.release();
}

void TcpServer::incomingConnection(qintptr descriptor)
{
    QThread * thread = new QThread;
    TcpSession * session = new TcpSession;

    QObject::connect(this, &TcpServer::stopped, session, &TcpSession::close);
    QObject::connect(this, &TcpServer::stopped, thread, &QThread::quit);
    QObject::connect(session, &TcpSession::closed, thread, &QThread::quit);

    QObject::connect(thread, &QThread::started, this, &TcpServer::threadStarted, Qt::DirectConnection);
    QObject::connect(thread, &QThread::finished, this, &TcpServer::threadFinished, Qt::DirectConnection);

    QObject::connect(thread, &QThread::finished, session, &QObject::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
    session->moveToThread(thread);

    // Open the session in the worker thread
    QMetaObject::invokeMethod(session, "open", Qt::QueuedConnection, Q_ARG(qintptr, descriptor));
}
