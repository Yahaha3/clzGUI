#include "GeoFunctionUtility.h"
#include <qmath.h>
#include "Map/Projection/Projection.h"
#include <QFile>
#include "common/clz_geomap_global.h"

#define PI 3.14159265359

bool has_decimal(double d)
{
    const double limit = 1e-10;
    return (abs(d - static_cast<int>(d)) > limit);
}

QString deprecateScientificNotation(double num, int precision)
{
    if (precision == -1) return QString::number(num);

    return QString::number(num, 'f', precision);
}

QString bool2Str(bool flag)
{
    QString res = (flag ? "True" : "False");
    return res;
}

clz::GeoFunctionUtility::GeoFunctionUtility()
{

}

int clz::GeoFunctionUtility::long2tilex(double lon, int z)
{
    return (int)(floor((lon + 180.0) / 360.0 * (1 << z)));
}

int clz::GeoFunctionUtility::lat2tiley(double lat, int z)
{
    double latrad = lat * PI/180.0;
    return (int)(floor((1.0 - asinh(tan(latrad)) / PI) / 2.0 * (1 << z)));
}

double clz::GeoFunctionUtility::tilex2long(int x, int z)
{
    return x / (double)(1 << z) * 360.0 - 180;
}

double clz::GeoFunctionUtility::tiley2lat(int y, int z)
{
    double n = PI - 2.0 * PI * y / (double)(1 << z);
    return 180.0 / PI * atan(0.5 * (exp(n) - exp(-n)));
}
// 此处操作相当于移动中心点
ImPlotPoint clz::GeoFunctionUtility::implot2scene(ImPlotPoint pt)
{
    auto nx = std::clamp(pt.x, 0.0, 1.0) - 0.5;
//    auto ny = 0.5 - pt.y;
    auto ny = std::clamp(pt.y, 0.0, 1.0) - 0.5;
    return ImPlotPoint(nx, ny);
}

ImPlotPoint clz::GeoFunctionUtility::scene2implot(ImPlotPoint pt)
{
    auto nx = pt.x + 0.5;
    auto ny = pt.y + 0.5;
    return ImPlotPoint(nx, ny);
}

ImPlotPoint clz::GeoFunctionUtility::implot2px(ImPlotPoint pt)
{
    auto pos = ImPlot::GetPlotPos();
    auto size = ImPlot::GetPlotSize();
    auto limit = ImPlot::GetPlotLimits();

    auto ratio_x = (pt.x - limit.X.Min) / limit.X.Size();
    auto ratio_y = (pt.y - limit.Y.Min) / limit.Y.Size();

    auto offset_x = size.x * ratio_x;
    auto offset_y = size.y * ratio_y;

    return ImPlotPoint(pos.x + offset_x, pos.y + offset_y);
}

ImPlotPoint clz::GeoFunctionUtility::px2implot(ImPlotPoint pt)
{
    auto pos = ImPlot::GetPlotPos();
    auto size = ImPlot::GetPlotSize();
    auto limit = ImPlot::GetPlotLimits();

    auto plot_px = ImPlotPoint(pt.x - pos.x, pt.y - pos.y);
    auto ratio_x = plot_px.x / size.x;
    auto ratio_y = plot_px.y / size.y;

    auto offset_x = limit.X.Size() * ratio_x;
    auto offset_y = limit.Y.Size() * ratio_y;

    return ImPlotPoint(limit.X.Min + offset_x, limit.Y.Min + offset_y);
}

QVector<QPointF> clz::GeoFunctionUtility::geo_list_to_proj_list(QVector<clz::GeoPos> list, Projection *projection)
{
    QVector<QPointF> ret;
    if(!projection) return ret;
    for(auto child: list)
    {
        auto proj = projection->geoToProj(child);
        ret.append(proj);
    }
    return ret;
}

QVector<clz::GeoPos> clz::GeoFunctionUtility::proj_list_to_geo_list(QVector<QPointF> list, Projection *projection)
{
    QVector<clz::GeoPos> ret;
    if(!projection) return ret;
    for(auto child: list)
    {
        auto proj = projection->projToGeo(child);
        ret.append(proj);
    }
    return ret;
}

QPointF clz::GeoFunctionUtility::get_polygon_center(QVector<QPointF> vertices)
{
    //计算多边形形心
    if(vertices.size() < 1) return QPointF();
    if(vertices.size() < 2) return vertices.first();
    double cx = 0,cy = 0;
    double A = 0;
    vertices.append(vertices[0]);//首尾相连
    for(int i = 0; i < vertices.size() - 1; i++)
    {
        auto pti = vertices[i];
        auto ptip = vertices[i + 1];
        cx += ((pti.x() + ptip.x()) * (pti.x() * ptip.y() - ptip.x() * pti.y()));
        cy += ((pti.y() + ptip.y()) * (pti.x() * ptip.y() - ptip.x() * pti.y()));
        A += (pti.x() * ptip.y() - ptip.x() * pti.y());
    }
    A = A / 2;
    cx = cx / (6 * A);
    cy = cy / (6 * A);
    return QPointF(cx, cy);
}

QPointF clz::GeoFunctionUtility::get_polyline_center(QVector<QPointF> vertices)
{
    if(vertices.size() <= 0) return QPointF();
    double tx = vertices.front().x(), ty = vertices.front().y();
    if(vertices.size() > 1)
    {
        for(int i = 1; i < vertices.size(); i++)
        {
            tx += vertices.at(i).x();
            ty += vertices.at(i).y();
        }
    }
    return QPointF(tx / vertices.size(), ty/ vertices.size());
}

QByteArray clz::GeoFunctionUtility::get_image_bytearray(const QString &path, bool &ok)
{
    ok = false;
    if(!QFile::exists(path)) return QByteArray();
    QFile file(path);
    if(file.open(QFile::ReadOnly)){
        ok = true;
        auto data = file.readAll();
        return data;
    }
    return QByteArray();
}
#ifdef USE_AOSK
//所有内核相关的业务都放这里来转吧
#include "AosKernelCommon.h"
#include "algorithm_utilities.h"
#include "aos_kernel_vardef_formatter_global.h"
#include "aos_kernel_variable_global.h"

QString clz::GeoFunctionUtility::kernel_apps_dir()
{
    return aos::AosKernelCommon::get_apps_dir();
}

QJsonObject clz::GeoFunctionUtility::kernel_read_json_file(const QString &path)
{
    return aos::AosKernelCommon::read_json_file_object(path);
}

clz::GeoPos clz::GeoFunctionUtility::get_target_lonlat_with_distance(const clz::GeoPos &p0, const clz::GeoPos &p1, double distance)
{
    double target_lon, target_lat;
    aos::algo_utils::lonlat_distancing(p1.longitude(), p1.latitude(),
                                       p0.longitude(), p0.latitude(),
                                       distance,
                                       target_lon, target_lat);
    return GeoPos(target_lat, target_lon);
}

QString clz::GeoFunctionUtility::cpsgr_icon_path(const QString &icon)
{
    auto pre = aos::AosKernelCommon::get_installer_dir();
    auto path = pre + QString("/UX/csgrs/%1_64x64.png").arg(icon);
    return path;
}

QString clz::GeoFunctionUtility::parse_format(const QJsonValue &value, const QString &format, bool& ok)
{
    QString result;
    if(format.isEmpty()) {
        ok = false;
        return QString();
    }
    ok = true;
    auto vv = aos::VarDefFormatterPluginTypeHelper::parse_format(value, format);
    if(vv.type() == QJsonValue::Double){
        result = QString::number(vv.toDouble());
    } else {
        result = vv.toString();
    }
    return result;
}

QString clz::GeoFunctionUtility::parse_enum(const QJsonValue &value, const QJsonArray &option, bool& ok)
{
    if(option.isEmpty()){
        ok = false;
        return QString();
    }
    QString result;
    if(value.isDouble()){
        // 有可能并不是0-1-2-3-4-5-6....的值枚举
        auto jv = value.toDouble();
        for(auto option_ :option){
            auto opt = option_.toObject();
            auto ov = opt[clz::value].toDouble();
            if(abs(ov - jv) < 1e-5){
                ok = true;
#ifdef LANGUAGE_EN
                result = opt[clz::note].toString();
#else
                result = opt[clz::note_i18n].toString();
#endif
                break;
            }
        }
    }
    return result;
}

QString clz::GeoFunctionUtility::parse_value(const QJsonValue &value, const int &to_si, const int &type, const int &precision, bool &ok)
{
    QString result = "N/A";
    auto type1 = static_cast<aos::VariableType>(type);
    if(value.isNull()) return result;

    switch (type1)
    {
    default:
        return result;
    case aos::VARIABLE_STRING:
        result = value.toString();
        break;
    case aos::VARIABLE_INTEGER:
        if (has_decimal(value.toDouble()*to_si))
        {
            result = deprecateScientificNotation(value.toDouble()*to_si, precision);
        }
        else
        {
            result = QString::number(static_cast<qint64>(value.toDouble()));
        }
        break;
    case aos::VARIABLE_NUMBER:
        result = deprecateScientificNotation(value.toDouble()*to_si, precision);
        break;
    case aos::VARIABLE_BOOL:
        result = bool2Str(value.toBool());
        break;
    }
    ok = true;
    return result;
}


bool clz::GeoFunctionUtility::out_of_target_area(const QVector<arma::vec2> &target_area, const arma::vec2 &target)
{
    return !aos::algo_utils::is_in_polygon(target_area, target);
}

arma::vec2 clz::GeoFunctionUtility::transfrom_to_reference_coordinate(const arma::vec2 &pnt, const arma::vec2 &pntr)
{
    double lon, lat;
    aos::algo_utils::lonlat2xy(pnt(0), pnt(1), pntr(0), pntr(1), lon, lat);
    arma::vec2 result{lon, lat};
    return result;
}

const QString icon_pre = "iconanim://";
const QString lock_unlock = "LockUnlock";
const QString wind_Vane = "WindVane";
const QString icon_path = "/appcontainers/eqnx_ui/res/icon/";
const QString lock_icon = "tool_bar/xc/white/locked_40x40.png";
const QString unlock_icon = "tool_bar/xc/white/unlocked_40x40.png";
const QString tool_bar_aoa = "tool_bar/aoa/";
const QString png_suffix = ".png";

QString clz::GeoFunctionUtility::identify_icon_with_definition_and_value(const QString &icon, const QJsonValue &value, bool &ok)
{
    QString result = clz::empty;
    ok = false;
    if(icon.isNull()) return result;

    auto path = aos::AosKernelCommon::get_apps_dir() + icon_path;
    if(icon.startsWith(icon_pre)){
        // 为动画类图标
        auto type = icon.split('/').last();
        if(type == lock_unlock && value.isDouble()){
            // 加解锁图标
            ok = true;
            auto icon_after = value.toDouble() == 0 ? lock_icon : unlock_icon;
            result = path + icon_after;
        } else if(type == wind_Vane){
            // 暂时不知道风向的图标
            ok = true;
            result = path + tool_bar_aoa + "upstream-silent_normal_40x40" + png_suffix;
        }
    } else {
        auto icon_enum = icon.split(';');
        for(auto item: icon_enum){
            auto enu = item.split(':');
            if(enu.size() != 2) continue;
            auto key = enu.first().toInt();
            if(abs(value.toDouble() - key) < 1e-7){
                ok = true;
                auto value = enu.last();
                result = path + tool_bar_aoa + value + png_suffix;
                break;
            }
        }
    }
    return result;
}

QString clz::GeoFunctionUtility::get_string_from_json_value(const QJsonValue &value, const QJsonArray &option, const QJsonObject &definition, bool &ok)
{
    ok = false;
    QString result = clz::GeoFunctionUtility::parse_enum(value, option, ok);
    if(!ok){
        // 每次都要解析json，会不会比较慢呢？
        auto format = definition[clz::format].toString();
        if(!format.isEmpty()){
            result = clz::GeoFunctionUtility::parse_format(value, format, ok);
        } else {
            auto to_si = definition[clz::to_si].toInt();
            auto type = definition[clz::type].toInt();
            auto precision = definition[clz::precision].toInt();
            auto unit = definition[clz::unit].toString();
            result = clz::GeoFunctionUtility::parse_value(value, to_si, type, precision, ok);
            result += unit;
        }
    }
    return result;
}

QString clz::GeoFunctionUtility::json_2_qstring(const QJsonObject &jo)
{
    return aos::AosKernelCommon::json_object_to_string(jo);
}

QJsonObject clz::GeoFunctionUtility::qstring_2_json(const QString &str)
{
    return aos::AosKernelCommon::string_to_json_object(str);
}
#endif
