#ifndef CLZLAYOUT_H
#define CLZLAYOUT_H

#include <QObject>
#include "ClzCore/ClzObject.h"
#include "ClzWidget.h"
#include "clzGUI_global.h"
/*
 * 用于实现简单的网格布局
 * 自动铺满,根据当前内容停靠
 * 一个layout相当于一个大的imgui
*/

namespace clz {

enum LayoutAnchor {
    None    = 0,
    Left    = 1 << 0,
    Top     = 1 << 1,
    Right   = 1 << 2,
    Bottom  = 1 << 3,
    HCenter = Left | Right,
    VCenter = Top | Bottom
};

class CLZGUI_EXPORT ClzLayout : public ClzObject
{
    Q_OBJECT
public:
    ClzLayout(ClzObject* parent = 0);

    void set_contents_margins(int left, int top, int right, int bottom);

    void add_widget(ClzWidget* widget);
    void remove_widget(ClzWidget* widget);
    bool contain_widget(ClzWidget* widget);
    bool contain_widget(const QString& id);

    // 暂时不让调整顺序吧
    // todo: readjust order
    virtual void relayout() = 0;
    virtual void resize_event(QSize psize);
    // 重置当前layout
    virtual void reset();

    QPoint pos();
    QSize size();

    void move(QPoint position);
    void resize(QSize size);

    LayoutAnchor layout_anchor() const;
    void set_layout_anchor(LayoutAnchor anchor);

    void set_sensitive_width_fold(bool sensitive);

    // 最大可用宽高，是可用并不是只能用
    void set_max_size(QSize);
    void set_enable_max_size(bool);

protected slots:
    virtual void slots_response_widget_mouse_wheel(float delta) = 0;

protected:
    int m_margin_left,m_margin_right,m_margin_top,m_margin_bottom;

    QPoint m_position;
    QSize m_size;

    LayoutAnchor m_anchor = None;

    bool m_sensitive_with_fold = true;

    QSize m_max_size = QSize(40,40);
    bool m_enable_max_size = false;

    // 滚动速度
    int m_scroll_speed = 27;
};

}

#endif // CLZLAYOUT_H
