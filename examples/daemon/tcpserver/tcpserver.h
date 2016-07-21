#ifndef TCPSERVERDAEMON_H
#define TCPSERVERDAEMON_H

#include <QTcpServer>
#include <QAtomicInteger>
#include <QSemaphore>

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpServer(QObject * parent = Q_NULLPTR);

signals:
    void stopped();

public slots:
    void start(const QStringList &);
    void stop();

private slots:
    void threadStarted();
    void threadFinished();

protected:
    void incomingConnection(qintptr) Q_DECL_OVERRIDE;

private:
    QAtomicInteger<qint8> activeSessions;
    QSemaphore threadWaitSemaphore;

    static const quint16 defaultPort;
};

Q_DECLARE_METATYPE(qintptr)

#endif // TCPSERVERDAEMON_H
