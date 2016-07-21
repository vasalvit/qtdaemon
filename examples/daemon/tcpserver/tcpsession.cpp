#include "tcpsession.h"

#include <QDaemonApplication>
#include <QDaemonLog>

TcpSession::TcpSession(QObject * parent)
    : QObject(parent), socket(Q_NULLPTR)
{
}

void TcpSession::open(qintptr descriptor)
{
    socket = new QTcpSocket(this);

    QObject::connect(socket, &QTcpSocket::connected, this, &TcpSession::opened);
    QObject::connect(socket, &QTcpSocket::disconnected, this, &TcpSession::closed);
    QObject::connect(socket, &QTcpSocket::readyRead, this, &TcpSession::readSocketData);

    if (!socket->setSocketDescriptor(descriptor))  {
        QMetaObject::invokeMethod(this, "closed", Qt::QueuedConnection);    // Give up on the session
        return;
    }
}

void TcpSession::close()
{
    socket->disconnectFromHost();   // Will emit closed() when the socket connection is dropped
}

void TcpSession::readSocketData()
{
    QString message = socket->readAll();
    if (message.trimmed() == "quit")
        qApp->quit();
    else if (message.trimmed() == "close")
        close();
    else
        qDaemonLog() << message;
}
