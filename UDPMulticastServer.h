#pragma once

#include <QObject>
#include <QCoreApplication>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTimer>
#include <QList>
#include <iostream>
#include <QStringList>

class UDPMulticastServer : public QObject
{
    Q_OBJECT

public:
    UDPMulticastServer(QObject* parent = nullptr) : QObject(parent)
    {
        socket.bind(QHostAddress::AnyIPv4, 5555, QUdpSocket::ShareAddress);
        socket.joinMulticastGroup(QHostAddress("239.255.43.21"));
        connect(&timer, &QTimer::timeout, this, &UDPMulticastServer::sendMulticastData);
        timer.start(500);
    }

public slots:
    void sendMulticastData()
    {
        QByteArray data = getLocalIPAddress().toUtf8() + " 5555";
        socket.writeDatagram(data, QHostAddress("239.255.43.21"), 5555);
        //receiveData();
    }
    void receiveData()
    {
        while (socket.hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(socket.pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;
            socket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            qDebug() << "Recieve:" << datagram.data();
            std::cout<<"Recieve: "<< datagram.data();
            QString peer =  QString("%1 %2").arg(sender.toString()).arg(senderPort);
            processReceivedData(peer);
            }
     }

            //QString ip = datagram.section(" ", 0, 0).trimmed();
            //int port = datagram.section(" ", 1, 1).toInt();
    void clearConnectInfo(){
    	connectInfo1.clear();
    }
    	
    void stopSend(){
    	timer.stop();
    }


private:
    QUdpSocket socket;
    QTimer timer;
    QStringList connectInfo1;
    
    void processReceivedData(const QString& peer) {
    	bool isNewPeer = true;
    	for (const auto& existingPeer : connectInfo1) {
        if (existingPeer == peer) {
            isNewPeer = false;
            break;
            }
    	}

    if (isNewPeer) {
        connectInfo1.append(peer);
        qDebug() << "New peer:" << peer;
    	}
    	
    }

    

    QString getLocalIPAddress()
    {
        QList<QHostAddress> list = QNetworkInterface::allAddresses();
        for (const QHostAddress& address : list) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
                return address.toString();
        }
        return "";
    }
};
