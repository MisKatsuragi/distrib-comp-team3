#pragma once

#include "../common/serializerBase.h"
#include "../common/encoderBase.h"
#include "discoveryData.h"
#include "discoveryServiceBase.h"

#include <QObject>
#include <QCoreApplication>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTimer>
#include <QList>
#include <iostream>
#include <QStringList>

class UDPMulticastServer : public DiscoveryServiceBase
{
public:
    UDPMulticastServer(SerializerBase<QByteArray, QJsonObject> *serializer,
                     EncoderBase<QByteArray> *encoder,
                     QObject *parent = nullptr) : DiscoveryServiceBase(parent)
	{
     ~UDPMulticastServer();
     }
    void startMulticast(const DiscoveryData &discoveryData) {
    	socket.bind(QHostAddress::AnyIPv4, 5555, QUdpSocket::ShareAddress);
        socket.joinMulticastGroup(QHostAddress("239.255.43.21"));
        connect(&timer, &QTimer::timeout, this, &UDPMulticastServer::startMulticast);
        timer.start(500);
    	QByteArray data = getLocalIPAddress().toUtf8() + " " + m_encoder->encode(m_serializer->serialize(discoveryData));
        socket.writeDatagram(data, QHostAddress("239.255.43.21"), 5555);
    	}
    	
    void startListening() {
    	isListening = true;
    	while(isListening && socket.hasPendingDatagrams() ){
            QByteArray datagram;
            datagram.resize(socket.pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;
            socket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            qDebug() << "Recieve:" << datagram.data();
            //std::cout<<"Recieve: "<< datagram.data();
            QString peer =  QString("%1 %2").arg(sender.toString()).arg(senderPort);
            processReceivedData(peer);
            }
    	}
    void stopListening() {
	isListening = false;
    	}
    	
    void stopMulticast() {
	timer.stop();
    	}
    	
    void clearPeers() {
    	connectInfo1.clear();
    	}
    	
    QList<DiscoveryData> peers(){
    	
    	}

signals:
    void peersChanged(const QList<DiscoveryData> peers);
    
private:
    QUdpSocket socket;
    QTimer timer;
    QStringList connectInfo1;
    SerializerBase<QByteArray, QJsonObject> *m_serializer;
    EncoderBase<QByteArray> *m_encoder;
    bool isListening = false;
    
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
