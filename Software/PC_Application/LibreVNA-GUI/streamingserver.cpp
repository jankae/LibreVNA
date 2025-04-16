#include "streamingserver.h"

#include "json.hpp"

StreamingServer::StreamingServer(int port)
{
    this->port = port;
    server.listen(QHostAddress::Any, port);
    connect(&server, &QTcpServer::newConnection, [&](){
        auto socket = server.nextPendingConnection();
        sockets.insert(socket);

        connect(socket, &QTcpSocket::stateChanged, [this, socket](QAbstractSocket::SocketState state){
            if (state == QAbstractSocket::UnconnectedState)
            {
                sockets.erase(socket);
                socket->deleteLater();
            }
        });
    });
}

void StreamingServer::addData(const DeviceDriver::VNAMeasurement &m, bool is_zerospan)
{
    nlohmann::json j;
    j["pointNum"] = m.pointNum;
    if(is_zerospan) {
        j["time"] = m.us * 0.000001;
    } else {
        j["frequency"] = m.frequency;
        j["dBm"] = m.dBm;
    }
    j["Z0"] = m.Z0;
    nlohmann::json jp;
    for(auto const &p : m.measurements) {
        jp[QString(p.first+"_real").toStdString()] = p.second.real();
        jp[QString(p.first+"_imag").toStdString()] = p.second.imag();
    }
    j["measurements"] = jp;
    std::string toSend = j.dump();
    for(auto s : sockets) {
        if(s->isOpen()) {
            s->write(QByteArray::fromStdString(toSend+'\n'));
        }
    }
}

void StreamingServer::addData(const DeviceDriver::SAMeasurement &m, bool is_zerospan)
{
    nlohmann::json j;
    j["pointNum"] = m.pointNum;
    if(is_zerospan) {
        j["time"] = m.us * 0.000001;
    } else {
        j["frequency"] = m.frequency;
    }
    nlohmann::json jp;
    for(auto const &p : m.measurements) {
        jp[p.first.toStdString()] = p.second;
    }
    j["measurements"] = jp;
    std::string toSend = j.dump();
    for(auto s : sockets) {
        if(s->isOpen()) {
            s->write(QByteArray::fromStdString(toSend+'\n'));
        }
    }
}
