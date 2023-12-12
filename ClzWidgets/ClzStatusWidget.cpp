#include "ClzStatusWidget.h"
#include "GeoFunction/GeoFunctionUtility.h"

clz::ClzStatusWidget::ClzStatusWidget(clz::ClzWidget *parent) : ClzCardWidget(parent)
{
    set_window_flag(ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove);
}

bool clz::ClzStatusWidget::update_card(const QString &uri, QJsonValue value)
{
#ifdef USE_AOSK
    if(m_uris.contains(uri)){
        // 更新图片对象的状态
        for(auto name: m_context_names){
            if(m_context_map[name].uri == uri && m_image_status.keys().contains(name)){
                // 找到当前uri对应的name image
                bool ok = false;
                auto definition = m_context_map[name].definition;
                auto icon = definition[clz::icon].toString();
                auto ipath = clz::GeoFunctionUtility::identify_icon_with_definition_and_value(icon, value, ok);
                if(!ok) continue;
                if(m_image_status[name] && m_image_status[name]->get_image_path() != ipath){
                    auto byte = clz::GeoFunctionUtility::get_image_bytearray(ipath, ok);
                    if(!ok) continue;
                    // 后期可以优化为更新的形式
                    m_image_status[name]->set_image_path(ipath);
                    m_image_status[name]->set_origin_data(byte.toStdString());
                    m_image_status[name]->loadFromOrigin();
               }
            }
        }
    }
#endif
    return ClzCardWidget::update_card(uri, value);
}

void clz::ClzStatusWidget::value_updated(int index)
{
    // 内容发生更改时要根据情况调整自身尺寸
    /*
     * 1，遍历当前参数
     * 2，判断当前项是否为图片，图片则固定大小
     * 2，若不为图片，则根据文字大小来调整尺寸
    */
    Q_UNUSED(index)
    int total_width = 0;
    for(auto item: m_context_map){
        if(m_image_status.contains(item.name)){
            // 图片
            total_width += 50;
        } else {
            // 文字
            auto value = item.value;
            auto width = ImGui::CalcTextSize(value.toStdString().c_str()).x + 10;
            total_width += width;
        }
    }
    auto osize = size();
    resize(QSize(total_width, osize.height()));
}

void clz::ClzStatusWidget::init()
{
    resize(QSize((m_context_map.size() + 0) * 43, 31));

    // 有些状态需要用图片来描述
    for(int i = 0, si = m_context_names.size(); i < si; i++){
        auto key = m_context_names[i];
        auto definition = m_context_map[key].definition;
        auto icon = definition[clz::icon].toString();
        if(!icon.isEmpty()){
            if(m_uris[i].isEmpty()){
                // 找到该参数对应的uri
                bool ok = false;
                auto uri = get_real_uri_for_value(key, ok);
                if(ok){
                    m_uris[i] = uri;
                    m_context_map[key].uri= uri;
                }
            }
            auto image = std::make_shared<ImageVk>();
            m_image_status.insert(key, image);
        }
    }
}

void clz::ClzStatusWidget::paint()
{
    ImGui::SetNextWindowPos(impos());
    ImGui::SetNextWindowSize(imsize());
    ImGui::Begin(name().toStdString().c_str(), 0, m_window_flags);
    // status有点特殊，当uri为空时，表示受到其他值的影响,status可能携带有图片
    for(int i = 0, si = m_context_names.size(); i < si; i++){
        auto name = m_context_names[i];
        auto context = m_context_map[name].context;
        auto value = m_context_map[name].value;
        bool ok = false;
        if(m_image_status.contains(name)){
            auto image = m_image_status[name];
            auto ID = image->get_image_texture(ok);
            if(ok){
                ImGui::Image(ID, {(float)image->Width, (float)image->Height});
                if(ImGui::IsWindowHovered()){
                    ImGui::SetTooltip(context.toStdString().c_str(), 0);
                }
            }
        } else {
            ImGui::Text(value.toStdString().c_str(), "");
            if(ImGui::IsWindowHovered()){
                ImGui::SetTooltip(context.toStdString().c_str(), 0);
            }
        }
        ImGui::SameLine();
    }

    ImGui::End();
}
