#ifndef TCPSESSION_H
#define TCPSESSION_H

#include <QObject>
#include <QTcpSocket>

class TcpSession : public QObject
{
    Q_OBJECT

public:
    explicit TcpSession(QObject * parent = Q_NULLPTR);

public slots:
    void open(qintptr);
    void close();

private slots:
    void readSocketData();

signals:
    void opened();
    void closed();

private:
    QTcpSocket * socket;
};

#endif // TCPSESSION_H
