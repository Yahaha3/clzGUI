#ifndef CLZSTATUSWIDGET_H
#define CLZSTATUSWIDGET_H

#include <QObject>
#include <QJsonValue>
#include "ClzWidgets/ClzCardWidget.h"
#include "common/Image.h"

// status 只显示自身值，不显示标题

namespace clz {

class CLZGUI_EXPORT ClzStatusWidget : public ClzCardWidget
{
    Q_OBJECT
public:
    ClzStatusWidget(ClzWidget* parent = 0);

    bool update_card(const QString &uri, QJsonValue value) override;
    void value_updated(int index) override;

    void init() override;
    void paint() override;

protected:
    // 图片对象句柄
    QHash<QString, std::shared_ptr<ImageVk>> m_image_status;
};

}

#endif // CLZSTATUSWIDGET_H
