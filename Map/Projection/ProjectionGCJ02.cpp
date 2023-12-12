#include "ProjectionGCJ02.h"
#include <qmath.h>
#include <QFile>
#include <QJsonObject>
#include "common/CherileeCommon.h"

#ifdef USE_AOSK
#include "aos_kernel_gis_global.h"
#endif
ProjectionGCJ02::ProjectionGCJ02() :
    Projection("", "GCJ02", "")
{
    mEarthRadius = 6378137.0; /* meters */
    ee = 0.0669342162296594323;
    mOriginShift = 2.0 * M_PI * mEarthRadius / 2.0;
    mMetersPerDegree = 2.0 * M_PI * mEarthRadius / 360.0;
    mRadiansPerDegree = M_PI / 180.0;
    mDegreesPerRadian = 180.0 / M_PI;
    mGeoBoundary = clz::GeoRect(85, -180, -85, +180);
    // 转换为大致的地理范围,单位为m
    mProjBoundary = geoToProj(mGeoBoundary);

    init();
}

clz::GeoRect ProjectionGCJ02::boundaryGeoRect() const
{
    return mGeoBoundary;
}

QRectF ProjectionGCJ02::boundaryProjRect() const
{
    return mProjBoundary;
}

QPointF ProjectionGCJ02::geoToProj(const clz::GeoPos &geoPos) const
{
    auto pos = geoPos;
    wgs84_to_gcj02(pos);
    const double lon = pos.longitude();
    const double lat = (pos.latitude() > mGeoBoundary.topLeft().latitude()) ? mGeoBoundary.topLeft().latitude()
                                                                               : pos.latitude();
    const double x = lon * mOriginShift / 180.0;
    const double preY = -qLn(qTan((90.0 + lat) * M_PI / 360.0)) / (M_PI / 180.0);
    const double y = preY * mOriginShift / 180.0;
    return QPointF(x, y);
}

clz::GeoPos ProjectionGCJ02::projToGeo(const QPointF &projPos) const
{
    const double lon = (projPos.x() / mOriginShift) * 180.0;
    const double preLat = (-projPos.y() / mOriginShift) * 180.0;
    const double lat = 180.0 / M_PI * (2.0 * qAtan(qExp(preLat * M_PI / 180.0)) - M_PI / 2.0);
    auto pos = clz::GeoPos(lat, lon);
    gcj02_to_wgs84(pos);
    return pos;
}

QRectF ProjectionGCJ02::geoToProj(const clz::GeoRect &geoRect) const
{
    QRectF rect;
    rect.setTopLeft(geoToProj(geoRect.topLeft()));
    rect.setBottomRight(geoToProj(geoRect.bottomRight()));
    return rect;
}

clz::GeoRect ProjectionGCJ02::projToGeo(const QRectF &projRect) const
{
    return clz::GeoRect(projToGeo(projRect.topLeft()), projToGeo(projRect.bottomRight()));
}

clz::TilePos ProjectionGCJ02::geoToTilePos(const clz::GeoPos &geoPos, int zoom) const
{
    const double lon = geoPos.longitude();
    const double lat = geoPos.latitude();
    const double x = floor((lon + 180.0) / 360.0 * pow(2.0, zoom));
    const double y =
            floor((1.0 - log(tan(lat * M_PI / 180.0) + 1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * pow(2.0, zoom));
    return clz::TilePos(static_cast<int>(x), static_cast<int>(y), zoom);
}

double ProjectionGCJ02::geodesicMeters(const clz::GeoPos &projPos1, const clz::GeoPos &projPos2) const
{
    const double latitudeArc = (projPos1.latitude() - projPos2.latitude()) * M_PI / 180.0;
    const double longitudeArc = (projPos1.longitude() - projPos2.longitude()) * M_PI / 180.0;
    const double latitudeH = qPow(sin(latitudeArc * 0.5), 2);
    const double lontitudeH = qPow(sin(longitudeArc * 0.5), 2);
    const double lonFactor = cos(projPos1.latitude() * M_PI / 180.0) * cos(projPos2.latitude() * M_PI / 180.0);
    const double arcInRadians = 2.0 * asin(sqrt(latitudeH + lonFactor * lontitudeH));
    return mEarthRadius * arcInRadians;
}

// 考虑优化，剔除kernel相关性
void ProjectionGCJ02::init()
{
#ifdef USE_AOSK
    const QString path = clz::CherileeCommon::get_root_dir() + m_boundary_path;
    if(!QFile::exists(path)) return;
    auto jo = clz::CherileeCommon::read_json_object_file(path);
    auto features = jo[aos::geojson_features].toArray();
    if(features.isEmpty()) return;
    auto feature = features.first().toObject();
    auto geometry = feature[aos::geojson_geometry].toObject();
    auto type = GET_STRUCT_INSTANCE(aos::geojson_geom_name_to_type)[geometry[aos::geojson_type].toString()];
    QVector<arma::vec2> boundary;
    if(type == aos::GeoJsonGeometryType::GeoPolygon){
        auto coordinates_ja = geometry[aos::geojson_geometry_coordinates].toArray();
        if(coordinates_ja.size() <= 0) return;
        auto coordinate = coordinates_ja.first().toArray();
        for(int i = 0, si = coordinate.size(); i < si; i++){
            auto pnt = coordinate[i].toArray();
            if(pnt.size() < 2) continue;
            auto pp = arma::vec2{pnt.at(0).toDouble(), pnt.at(1).toDouble()};
            boundary.append(pp);
        }
    }
    {
        if(boundary.size() == 0) return;
        m_boundary_line.clear();
        m_reference_pnt = boundary.front();
        for(int i = 0, si = boundary.size(); i < si; ++i){
            auto pnt = boundary.at(i);
            auto rpnt = clz::GeoFunctionUtility::transfrom_to_reference_coordinate(pnt, m_reference_pnt);
            m_boundary_line.append(rpnt);
        }
    }
#endif
}

void ProjectionGCJ02::wgs84_to_gcj02(clz::GeoPos &pos) const
{
    if(out_of_boundary(pos.longitude(), pos.latitude())) return;

    double dLat = transfrom_lat(pos.longitude() - 105.0, pos.latitude() - 35.0);
    double dLon = transfrom_lon(pos.longitude() - 105.0, pos.latitude() - 35.0);
    double radLat = pos.latitude() / 180.0 * M_PI;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((mEarthRadius * (1 - ee)) / (magic * sqrtMagic) * M_PI);
    dLon = (dLon * 180.0) / (mEarthRadius / sqrtMagic * cos(radLat) * M_PI);
    pos.setLat(pos.latitude() + dLat);
    pos.setLon(pos.longitude() + dLon);
}

void ProjectionGCJ02::gcj02_to_wgs84(clz::GeoPos &pos) const
{
    if(out_of_boundary(pos.longitude(), pos.latitude())) return;

    double dlat = transfrom_lat(pos.longitude() - 105.0, pos.latitude() - 35.0);
    double dlng = transfrom_lon(pos.longitude() - 105.0, pos.latitude() - 35.0);
    double radlat = pos.latitude() / 180.0 * M_PI;
    double magic = sin(radlat);
    magic = 1 - ee * magic * magic;
    double sqrtmagic = sqrt(magic);
    dlat = (dlat * 180.0) / ((mEarthRadius * (1 - ee)) / (magic * sqrtmagic) * M_PI);
    dlng = (dlng * 180.0) / (mEarthRadius / sqrtmagic * cos(radlat) * M_PI);
    double mglat = pos.latitude() + dlat;
    double mglng = pos.longitude() + dlng;
    pos.setLat(pos.latitude() * 2 - mglat);
    pos.setLon(pos.longitude() * 2 - mglng);
}

double ProjectionGCJ02::transfrom_lat(double x, double y) const
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * M_PI) + 40.0 * sin(y / 3.0 * M_PI)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * M_PI) + 320 * sin(y * M_PI / 30.0)) * 2.0 / 3.0;
    return ret;
}

double ProjectionGCJ02::transfrom_lon(double x, double y) const
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * M_PI) + 40.0 * sin(x / 3.0 * M_PI)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * M_PI) + 300.0 * sin(x / 30.0 * M_PI)) * 2.0 / 3.0;
    return ret;
}

bool ProjectionGCJ02::out_of_boundary(double lon, double lat) const
{
#ifdef USE_AOSK
    if(m_boundary_line.isEmpty()) return true;
    auto pnt = clz::GeoFunctionUtility::transfrom_to_reference_coordinate(arma::vec2{lon, lat}, m_reference_pnt);
    return clz::GeoFunctionUtility::out_of_target_area(m_boundary_line, pnt);
#else
    return false;
#endif
}
