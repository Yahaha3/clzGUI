#ifndef CLZBUTTON_H
#define CLZBUTTON_H

#include <QObject>
#include "ClzWidgets/ClzWidget.h"
#include "common/Image.h"

namespace clz {

class CLZGUI_EXPORT ClzImageButton : public ClzWidget
{
    Q_OBJECT
public:
    ClzImageButton(ClzWidget* parent = 0);
    ClzImageButton(const QString& image, const QString& name, ClzWidget* parent = 0);

    void set_image_path(const QString& image);
    void init() override;
    void paint() override;

signals:
    void sig_image_btn_clicked(const QString& id);

protected:
    // 这里就不由地图来维护了
    std::shared_ptr<ImageVk> m_btn_image;
    QString m_image_path;
};

}

#endif // CLZBUTTON_H
