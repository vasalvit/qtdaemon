#include "tcpserver.h"

#include <QDaemonApplication>

int main(int argc, char ** argv)
{
    QDaemonApplication app(argc, argv);

    QDaemonApplication::setApplicationName("TcpServerDaemon example");
    QDaemonApplication::setApplicationDescription("The TcpServerDaemon example shows the capabilities of QtDaemon module");
    QDaemonApplication::setOrganizationDomain("qtdaemon.exmaples");

    TcpServer tcpServer(&app);

    QObject::connect(&app, &QDaemonApplication::daemonized, &tcpServer, &TcpServer::start);
    QObject::connect(&app, &QDaemonApplication::aboutToQuit, &tcpServer, &TcpServer::stop);

    return QDaemonApplication::exec();
}
