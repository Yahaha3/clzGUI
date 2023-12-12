#include "ClzCardWidget.h"
#include "GeoFunction/GeoFunctionUtility.h"
#include <QJsonArray>
#include <QJsonObject>

clz::ClzCardWidget::ClzCardWidget(clz::ClzWidget *parent):ClzWidget(parent)
{
    set_window_flag(ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoMove);
}

bool clz::ClzCardWidget::init_card(const QStringList &context, const QStringList &uri,
                                   const QJsonArray& definitions,
                                   const QList<QJsonArray> &options)
{
    auto size = context.size();
    if(uri.size() == size && options.size() == size && definitions.size() == size){
        // first clear
        m_context_map.clear();
        m_uris.clear();
        m_context_names.clear();
        // second fill
        m_uris = uri;
        for(int i = 0, si = uri.size(); i < si; i++){
            auto def = definitions[i].toObject();
            clz::Context card;
            card.name = def[clz::name].toString();
            card.uri = uri[i];
            card.context = context[i];
            card.value = "N/A";
            card.option = options[i];
            card.definition = def;
            m_context_map.insert(card.name, card);
            m_context_names.append(card.name);
        }
        init();
        return true;
    }
    return false;
}

bool clz::ClzCardWidget::update_card(const QString &uri, QJsonValue value)
{
    if(!m_uris.contains(uri)) return false;
    bool ok = false;
#ifdef USE_AOSK
    auto count = m_uris.count(uri);
    for(int index = m_uris.indexOf(uri), si = m_uris.size(); index < si; index++){
        // 保证所有uri相同的对象都能更新
        if(count == 0) break;
        auto name = m_context_names[index];
        auto option = m_context_map[name].option;
        auto definition = m_context_map[name].definition;
        auto result = clz::GeoFunctionUtility::get_string_from_json_value(value, option, definition, ok);
        // 可以在这里做个虚函数在参数内容更改的情况下更新一下自身尺寸
        auto result_before = m_context_map[name].value;
        if(ok && result != result_before){
            m_context_map[name].value = result;
            value_updated(index);
        }
        count--;
    }
#endif
    return ok;
}

void clz::ClzCardWidget::value_updated(int index)
{
    // 参数已更新，是否需要响应 也可以抛出信号
    Q_UNUSED(index)
}

void clz::ClzCardWidget::init()
{
    resize(QSize(380, (m_context_names.size() + 1) * 31));
}

void clz::ClzCardWidget::paint()
{
    ImGui::SetNextWindowPos(impos());
    ImGui::SetNextWindowSize(imsize());
    ImVec2 alignment = ImVec2(0.5, 0.5);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, alignment);
    ImGui::Begin(name().toStdString().c_str(), 0, m_window_flags);
//    ImGui::SetWindowFontScale(1.42);
    m_widget_fold = ImGui::IsWindowCollapsed();
    auto window_width = ImGui::GetWindowSize().x;
    auto colon_width = ImGui::CalcTextSize(m_colon.toStdString().c_str()).x;
    auto center = (window_width - colon_width) * 0.5f;
    for(int i = 0, si = m_context_names.size(); i < si; i++){
        auto key = m_context_names[i];
        auto context = m_context_map[key];
        auto text = context.context;
        auto value = context.value;
        auto context_width = ImGui::CalcTextSize(text.toStdString().c_str()).x;
        ImGui::SetCursorPosX(center - context_width);
        ImGui::Text(text.toStdString().c_str(), "");
        ImGui::SameLine();
        ImGui::Text(":");
        ImGui::SameLine();
        ImGui::Text(value.toStdString().c_str(), "");
    }

    {
        // 捕获鼠标滚轮事件
        auto io = ImGui::GetIO();
        if(ImGui::IsWindowHovered()){
            if(io.MouseWheel != 0){
                emit sig_on_mouse_scroll(io.MouseWheel);
//                std::cout << "mouse wheel value: " << io.MouseWheel << std::endl;
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

QString clz::ClzCardWidget::get_real_uri_for_value(const QString &name, bool &ok)
{
    ok = false;
    if(!name.endsWith(clz::icon)) return QString();
    auto prefix = name.split(clz::icon).first();
    for(int i = 0, si = m_context_names.size(); i < si; i++){
        auto tname = m_context_names[i];
        if(name == tname) continue;
        if(tname.startsWith(prefix) && tname.endsWith(clz::val) && !m_uris[i].isEmpty()){
            ok = true;
            return m_uris[i];
        }
    }
    return QString();
}

void clz::ClzCardWidget::slot_card_data_update(const QString &uri, QJsonValue value)
{
    update_card(uri, value);
}
