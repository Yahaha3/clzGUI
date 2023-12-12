#ifndef GEOBACKENDGOOGLE_H
#define GEOBACKENDGOOGLE_H

#include "GeoBackend.h"

class CLZGUI_EXPORT GeoBackendGoogle: public GeoBackend
{
public:
    GeoBackendGoogle();
    void init() override;
    QString build(clz::TilePos pos) override;
    int min_level() const override;
    int max_level() const override;
    QString label() override;
    QString server() const override;
};

#endif // GEOBACKENDGOOGLE_H
