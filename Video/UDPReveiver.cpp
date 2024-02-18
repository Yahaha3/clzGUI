#include "UDPReveiver.h"
#include <QNetworkInterface>

const int     ibsibdtctr_topic_cache_size = 1024*1024*8;

clz::UDPReveiver::UDPReveiver(QObject *parent): QObject(parent)
{
    m_addr = QHostAddress("226.0.1.101");
    m_port = 8000;
}

void clz::UDPReveiver::update_addr(const QString &addr, int port)
{
    m_addr = QHostAddress(addr);
    m_port = port;
}

void clz::UDPReveiver::init()
{
    if(!m_socket){
        m_socket = std::make_shared<QUdpSocket>();
    }
    m_socket->close();
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

//        connect(m_socket.get(), &QUdpSocket::readyRead, this, &clz::UDPReveiver::slot_udp_info_recv);
    }
}

QByteArray clz::UDPReveiver::data() const
{
    return m_data;
}

void clz::UDPReveiver::clear()
{
    m_socket->close();
}

int clz::UDPReveiver::read_socket(QByteArray &d, int buf_size)
{
    d.resize(static_cast<int>(m_socket->pendingDatagramSize()));
    auto len = m_socket->readDatagram(d.data(),buf_size,&m_addr);
//    auto len = m_socket->read(d.data(), buf_size);
    return len;
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
