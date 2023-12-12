#include "ClzLinkStatusWidget.h"
#include "GeoFunction/GeoFunctionUtility.h"

const QString link_health = "linkhealth";

clz::ClzLinkStatusWidget::ClzLinkStatusWidget(clz::ClzWidget *parent)
{

}

void clz::ClzLinkStatusWidget::init_link_card(const QJsonArray &items)
{
    // 这里定义不明确，只能写硬代码去生成了
    for(auto item_: items){
        auto item = item_.toObject();
        if(!item[clz::items].toArray().isEmpty()){
            // 先把所有参数都拿出来吧
            auto sitems = item[clz::items].toArray();
            for(auto sitem_: sitems){
                auto sitem = sitem_.toObject();
                auto sdef = sitem[clz::def].toObject();
                auto name = sdef[clz::name].toString();
#ifdef LANGUAGE_EN
                auto context = sdef[clz::name].toString();
#else
                auto context = sdef[clz::name_i18n].toString();
#endif
                auto option = sdef[clz::options].toArray();
                auto uri = sdef[clz::uri].toString();

                Context cc;
                cc.uri = uri;
                cc.name = name;
                cc.context = context;
                cc.option = option;
                cc.definition = sdef;

                m_link_uris.append(uri);
                m_link_context_names.append(name);
                m_link_context_map.insert(name, cc);
            }
            return;
        }
    }
}

bool clz::ClzLinkStatusWidget::update_card(const QString &uri, QJsonValue value)
{
    if(!m_link_uris.contains(uri)) return false;
    bool ok = false;
#ifdef USE_AOSK
    auto count = m_link_uris.count(uri);
    for(int index = m_link_uris.indexOf(uri), si = m_link_uris.size(); index < si; index++){
        // 保证所有uri相同的对象都能更新
        if(count == 0) break;
        auto name = m_link_context_names[index];
        auto option = m_link_context_map[name].option;
        auto definition = m_link_context_map[name].definition;
        auto result = clz::GeoFunctionUtility::get_string_from_json_value(value, option, definition, ok);
        // 可以在这里做个虚函数在参数内容更改的情况下更新一下自身尺寸
        auto result_before = m_link_context_map[name].value;
        if(ok && result != result_before){
            m_link_context_map[name].value = result;
        }
        count--;
    }
#endif
    return ClzStatusWidget::update_card(uri, value);
}

void clz::ClzLinkStatusWidget::init()
{
    ClzStatusWidget::init();
}

void clz::ClzLinkStatusWidget::paint()
{
    // 需要链路状态需要慢慢画，后续立项再做吧
    ClzStatusWidget::paint();
}
