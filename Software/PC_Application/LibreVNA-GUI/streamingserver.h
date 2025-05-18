#ifndef STREAMINGSERVER_H
#define STREAMINGSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <set>

#include "Device/devicedriver.h"

class StreamingServer : public QObject
{
    Q_OBJECT
public:
    StreamingServer(int port);

    void addData(const DeviceDriver::VNAMeasurement &m, bool is_zerospan);
    void addData(const DeviceDriver::SAMeasurement &m, bool is_zerospan);

    int getPort() {return port;}

private:
    int port;
    QTcpServer server;
    std::set<QTcpSocket*> sockets;
};

#endif // STREAMINGSERVER_H
