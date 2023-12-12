#include "UDPReveiver.h"
#include <QNetworkInterface>

const int     ibsibdtctr_topic_cache_size = 1024*1024*8;

clz::UDPReveiver::UDPReveiver(QObject *parent): QObject(parent)
{
    m_addr = QHostAddress("226.0.1.101");
    m_port = 8000;
}

void clz::UDPReveiver::init()
{
    if(!m_socket){
        m_socket = std::make_shared<QUdpSocket>();
    }
    auto bind = m_socket->bind(QHostAddress::AnyIPv4,
                               m_port,
                               QUdpSocket::ShareAddress);

    if(bind){
        auto interfaces = QNetworkInterface::allInterfaces();
        auto interface = interfaces[7];
        m_socket->setMulticastInterface(interface);
        m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
        m_socket->joinMulticastGroup(m_addr, interface);
        m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption,
                                  ibsibdtctr_topic_cache_size);

        connect(m_socket.get(), &QUdpSocket::readyRead, this, &clz::UDPReveiver::slot_udp_info_recv);
    }
}

QByteArray clz::UDPReveiver::data() const
{
    return m_data;
}
#include "common/Image.h"
void clz::UDPReveiver::slot_udp_info_recv()
{
    while (m_socket->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        QHostAddress *host = new QHostAddress;
        m_socket->readDatagram(datagram.data(),datagram.size(),host);
        m_data = datagram;
        int w, h;
        auto dd = ImageVk::get_image_data(data(), w, h);
        emit sig_video_info_decoded(dd, w, h);
    }
}
