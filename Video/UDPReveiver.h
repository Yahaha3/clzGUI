#ifndef UDPREVEIVER_H
#define UDPREVEIVER_H

#include <QObject>
#include <QUdpSocket>

namespace clz {

class UDPReveiver : public QObject
{
    Q_OBJECT
public:
    UDPReveiver(QObject* parent = 0);

    void update_addr(const QString& addr, int port);
    void init();
    QByteArray data() const;
    void clear();
    int read_socket(QByteArray& d, int buf_size);

signals:
    void sig_video_info_decoded(uchar* data, int w, int h);

private slots:
    void slot_udp_info_recv();

private:
    std::shared_ptr<QUdpSocket> m_socket;
    QHostAddress m_addr;
    quint16 m_port;
    QByteArray m_data = QByteArray();
};

}

#endif // UDPREVEIVER_H
