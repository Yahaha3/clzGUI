#ifndef CLZ_DATABASE_COMMON_H
#define CLZ_DATABASE_COMMON_H
#ifdef USE_AOSK
#include "aos_kernel_global_base.h"
namespace clz {

DECLARE_CONSTANT_QSTRING(check_db_exist,        "SELECT name from sqlite_master WHERE name='map-data'")
DECLARE_CONSTANT_QSTRING(create_map_cache_db,   "CREATE TABLE \"map-data\" ("
                                                "  \"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT,"
                                                "  \"mapBrand\" text NOT NULL,"
                                                "  \"mapShowStyle\" text NOT NULL,"
                                                "  \"x\" integer NOT NULL,"
                                                "  \"y\" integer NOT NULL,"
                                                "  \"z\" integer NOT NULL,"
                                                "  \"mapData\" blob,"
                                                "  \"updateTime\" text"
                                                ");")

DECLARE_CONSTANT_QSTRING(query_map_cache_db,    "SELECT mapData, updateTime FROM \"map-data\" WHERE mapBrand='%1' "
                                                "AND mapShowStyle='%2' AND x='%3' AND y='%4' AND z='%5'")
DECLARE_CONSTANT_QSTRING(query_map_cache_db_v2, "SELECT mapData, updateTime FROM \"map-data-google\" WHERE mapShowStyle='%1' "
                                                "AND x='%2' AND y='%3' AND z='%4'")
DECLARE_CONSTANT_QSTRING(update_map_cache_db,   "UPDATE \"map-data-google\" SET mapData='%1', updateTime='%2' WHERE mapBrand='%3' "
                                                "AND mapShowStyle='%4' AND x='%5' AND y='%6' AND z='%7' AND language='%8'")
DECLARE_CONSTANT_QSTRING(insert_map_cache_db,   "INSERT INTO \"map-data-google\" "
                                                "(mapBrand, mapShowStyle, x, y, z, mapData, updateTime, language)"
                                                " VALUES('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8')")
DECLARE_CONSTANT_QSTRING(delete_map_cache_db,   "DELETE FROM \"map-data-google\" WHERE mapBrand='%1' AND "
                                                "mapShowStyle='%2' AND x='%3' AND y='%4' AND z='%5'")


}
#else
#include <QString>
namespace clz {
const QString check_db_exist =        "SELECT name from sqlite_master WHERE name='map-data'";
const QString create_map_cache_db =   "CREATE TABLE \"map-data\" ("
                                    "  \"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT,"
                                    "  \"mapBrand\" text NOT NULL,"
                                    "  \"mapShowStyle\" text NOT NULL,"
                                    "  \"x\" integer NOT NULL,"
                                    "  \"y\" integer NOT NULL,"
                                    "  \"z\" integer NOT NULL,"
                                    "  \"mapData\" blob,"
                                    "  \"updateTime\" text"
                                    ");";

const QString query_map_cache_db =    "SELECT mapData, updateTime FROM \"map-data\" WHERE mapBrand='%1' "
                        "AND mapShowStyle='%2' AND x='%3' AND y='%4' AND z='%5'";
const QString query_map_cache_db_v2 = "SELECT mapData, updateTime FROM \"map-data-google\" WHERE mapShowStyle='%1' "
                        "AND x='%2' AND y='%3' AND z='%4'";
const QString update_map_cache_db =   "UPDATE \"map-data-google\" SET mapData='%1', updateTime='%2' WHERE mapBrand='%3' "
                        "AND mapShowStyle='%4' AND x='%5' AND y='%6' AND z='%7' AND language='%8'";
const QString insert_map_cache_db =   "INSERT INTO \"map-data-google\" "
                       "(mapBrand, mapShowStyle, x, y, z, mapData, updateTime, language)"
                       " VALUES('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8')";
const QString delete_map_cache_db =   "DELETE FROM \"map-data-google\" WHERE mapBrand='%1' AND "
                                                "mapShowStyle='%2' AND x='%3' AND y='%4' AND z='%5'";


}
#endif
#endif // CLZ_DATABASE_COMMON_H
