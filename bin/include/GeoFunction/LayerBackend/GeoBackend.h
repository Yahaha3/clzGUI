#ifndef GEOBACKEND_H
#define GEOBACKEND_H
#include "GeoFunction/GeoMapGlobal.h"
#include "clzGUI_global.h"
#include <QObject>

class CLZGUI_EXPORT GeoBackend
{
public:
    GeoBackend();
    virtual ~GeoBackend() = default;
    virtual void init() = 0;
    virtual QString build(clz::TilePos pos) = 0;
    virtual int min_level() const = 0;
    virtual int max_level() const = 0;
    virtual QString server() const = 0;

    virtual QString label() = 0;
protected:
    QString tile_server;
};

#endif // GEOBACKEND_H
