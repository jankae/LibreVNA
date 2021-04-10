#include "tcpserver.h"
#include <QDebug>

TCPServer::TCPServer(int port)
{
    qInfo() << "Listening on port" << port;
    socket = nullptr;
    server.listen(QHostAddress::Any, port);
    connect(&server, &QTcpServer::newConnection, [&](){
        // only one connection at a time
        delete socket;
        socket = server.nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, [=](){
            if(socket->canReadLine()) {
                auto available = socket->bytesAvailable();
                char data[available+1];
                socket->readLine(data, sizeof(data));
                auto line = QString(data);
                emit received(line.trimmed());
            }
        });
        connect(socket, &QTcpSocket::stateChanged, [&](QAbstractSocket::SocketState state){
            if (state == QAbstractSocket::UnconnectedState)
            {
                socket->deleteLater();
                socket = nullptr;
            }
        });
    });
}

bool TCPServer::send(QString line)
{
    if (socket) {
        socket->write(QByteArray::fromStdString(line.toStdString()+'\n'));
        return true;
    } else {
        return false;
    }
}
