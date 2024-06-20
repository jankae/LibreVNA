#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class TCPServer : public QObject
{
    Q_OBJECT
public:
    TCPServer(int port);

    int getPort() {return port;}

public slots:
    bool send(QString line);
signals:
    void received(QString line);
private:
    int port;
    QTcpServer server;
    QTcpSocket *socket;
};

#endif // TCPSERVER_H
