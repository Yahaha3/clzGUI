#ifndef CLZLINKSTATUSWIDGET_H
#define CLZLINKSTATUSWIDGET_H

#include <QObject>
#include <QJsonValue>
#include "ClzStatusWidget.h"

// link card是一种可扩展的状态显示，在折叠时显示为主要信息，展开则显示全部信息
// 直接继承吧，目前业务暂时不需要抽象层级那么高，后续可抽象为按布局生成的扩展状态栏
namespace clz {

class CLZGUI_EXPORT ClzLinkStatusWidget : public ClzStatusWidget
{
    Q_OBJECT
public:
    ClzLinkStatusWidget(ClzWidget* parent = 0);

    void init_link_card(const QJsonArray& items);

    bool update_card(const QString &uri, QJsonValue value) override;

    void init() override;
    void paint() override;

private:
    // 路径索引
    QStringList m_link_uris;
    // 名称索引
    QStringList m_link_context_names;
    // 名称到卡片的映射
    QHash<QString, Context> m_link_context_map;
};

}


#endif // CLZLINKSTATUSWIDGET_H
