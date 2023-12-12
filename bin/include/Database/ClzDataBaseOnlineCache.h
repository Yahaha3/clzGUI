#ifndef CLZDATABASEONLINECACHE_H
#define CLZDATABASEONLINECACHE_H

#include <QLocalSocket>
namespace clz {

class ClzDataBaseOnlineCache : public QObject
{
    Q_OBJECT
public:
    ClzDataBaseOnlineCache(QObject* parent = 0);
    void init();

    void update_tile_server_address(const QString& address, const QString& brand, const QString& style);
    void request_tile(int x, int y, int z);
private:
    std::shared_ptr<QLocalSocket> m_socket;
};

}

#endif // CLZDATABASEONLINECACHE_H
