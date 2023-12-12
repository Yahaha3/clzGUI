#include "ClzDataBaseOnlineCache.h"
#include "common/clz_geomap_global.h"
#include <QJsonObject>
#include "common/CherileeCommon.h"

const QString local_name = "clz_tile_downloader";
const QString server_address = "serverAddress";
const QString tile_info = "tileInfo";

clz::ClzDataBaseOnlineCache::ClzDataBaseOnlineCache(QObject *parent) : QObject(parent)
{

}

void clz::ClzDataBaseOnlineCache::init()
{
    m_socket = std::make_shared<QLocalSocket>();
    m_socket->connectToServer(local_name);
//    std::cout << "connect to : " << local_name.toStdString() << "\n";
}

void clz::ClzDataBaseOnlineCache::update_tile_server_address(const QString &address, const QString &brand, const QString &style)
{
    QJsonObject jo;
    jo[clz::type] = server_address;
    jo[clz::address] = address;
    jo[clz::brand] = brand;
    jo[clz::style] = style;

    m_socket->write(clz::CherileeCommon::json_object_to_qstring(jo).toUtf8());
    m_socket->flush();
}

void clz::ClzDataBaseOnlineCache::request_tile(int x, int y, int z)
{
    QJsonObject jo;
    jo[clz::type] = tile_info;
    jo[clz::x] = x;
    jo[clz::y] = y;
    jo[clz::z] = z;
    m_socket->write(clz::CherileeCommon::json_object_to_qstring(jo).toUtf8());
    m_socket->flush();
}
