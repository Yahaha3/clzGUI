#ifndef CLZCARDWIDGET_H
#define CLZCARDWIDGET_H

#include <QObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include "ClzWidgets/ClzWidget.h"

// card 为左右,左右的形式显示数据

namespace clz {

struct Context {
    // 名称 需要保证唯一性
    QString name;
    // 参数路径
    QString uri;
    // 目录
    QString context;
    // 值
    QString value;
    // 值枚举
    QJsonArray option;
    // 卡片定义
    QJsonObject definition;
};

class CLZGUI_EXPORT ClzCardWidget: public ClzWidget
{
    Q_OBJECT
public:
    ClzCardWidget(ClzWidget* parent = 0);

    bool init_card(const QStringList& context,
                   const QStringList& uri,
                   const QJsonArray& definitions,
                   const QList<QJsonArray>& options);
    virtual bool update_card(const QString& uri, QJsonValue value);
    virtual void value_updated(int index);

    void init() override;
    void paint() override;

    // 获取实际对应的uri
    QString get_real_uri_for_value(const QString& name, bool& ok);

public slots:
    void slot_card_data_update(const QString& uri, QJsonValue value);

protected:
    // 路径索引
    QStringList m_uris;
    // 名称索引
    QStringList m_context_names;
    // 名称到卡片的映射
    QHash<QString, Context> m_context_map;

    const QString m_colon = ":";
};
}

#endif // CLZCARDWIDGET_H
