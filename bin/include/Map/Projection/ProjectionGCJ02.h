#ifndef PROJECTIONGCJ02_H
#define PROJECTIONGCJ02_H

#include "Projection.h"
#include <QRectF>
#include <armadillo>
#include <QVector>

class CLZGUI_EXPORT ProjectionGCJ02: public Projection
{
public:
    ProjectionGCJ02();
    virtual ~ProjectionGCJ02() = default;

private:
    clz::GeoRect boundaryGeoRect() const override final;
    QRectF boundaryProjRect() const override final;

    QPointF geoToProj(clz::GeoPos const& geoPos) const override final;
    clz::GeoPos projToGeo(QPointF const& projPos) const override final;
    QRectF geoToProj(clz::GeoRect const& geoRect) const override final;
    clz::GeoRect projToGeo(QRectF const& projRect) const override final;
    clz::TilePos geoToTilePos(const clz::GeoPos &geoPos, int zoom) const override;

    double geodesicMeters(const clz::GeoPos &projPos1, const clz::GeoPos &projPos2) const override;

    // 国境线检查初始化
    void init();

    void wgs84_to_gcj02(clz::GeoPos& pos) const;
    void gcj02_to_wgs84(clz::GeoPos& pos) const;

    double transfrom_lat(double x, double y) const;
    double transfrom_lon(double x, double y) const;

    bool out_of_boundary(double lon, double lat) const;

private:
    double mEarthRadius;
    double mOriginShift;

    double mMetersPerDegree;
    double mRadiansPerDegree;
    double mDegreesPerRadian;

    clz::GeoRect mGeoBoundary;
    QRectF mProjBoundary;

    double ee;
    QVector<arma::vec2> m_boundary_line;
    arma::vec2 m_reference_pnt;

    const QString m_boundary_path = "/appcontainers/eqnx_mapview_qwidget/resources/boundary.json";
};

#endif // PROJECTIONGCJ02_H
