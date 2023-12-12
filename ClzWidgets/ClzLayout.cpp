#include "ClzLayout.h"
#include "ClzCore/ClzObjectManager.h"
#include "GeoFunction/GeoFunctionUtility.h"

clz::ClzLayout::ClzLayout(clz::ClzObject *parent) : ClzObject(parent)
{
    m_margin_left       = 1;
    m_margin_right      = 1;
    m_margin_top        = 1;
    m_margin_bottom     = 1;
}

void clz::ClzLayout::set_contents_margins(int left, int top, int right, int bottom)
{
    m_margin_left       = left;
    m_margin_right      = right;
    m_margin_top        = top;
    m_margin_bottom     = bottom;
}

void clz::ClzLayout::add_widget(clz::ClzWidget *widget)
{
    if(contain_widget(widget)) return;
    {
        // connect signal
        connect(widget, &clz::ClzWidget::sig_on_mouse_scroll, this, &clz::ClzLayout::slots_response_widget_mouse_wheel);
    }
    add_child(widget);
}

void clz::ClzLayout::remove_widget(clz::ClzWidget *widget)
{
    if(!contain_widget(widget)) return;
    remove_child(widget);
}

bool clz::ClzLayout::contain_widget(clz::ClzWidget *widget)
{
    return contain_widget(widget->get_object_id());
}

bool clz::ClzLayout::contain_widget(const QString &id)
{
    return has_child(id);
}

void clz::ClzLayout::resize_event(QSize psize)
{
    QPoint position(0,0);
    if(clz::GeoFunctionUtility::has_flag(layout_anchor(), LayoutAnchor::Left)){
        position.setX(10);
    }
    if(clz::GeoFunctionUtility::has_flag(layout_anchor(), LayoutAnchor::Right)){
        position.setX(psize.width() - size().width() - 10);
    }
    if(clz::GeoFunctionUtility::has_flag(layout_anchor(), LayoutAnchor::HCenter)){
        position.setX(psize.width() / 2 - size().width() / 2);
    }

    if(clz::GeoFunctionUtility::has_flag(layout_anchor(), LayoutAnchor::Top)){
        position.setY(0);
    }
    if(clz::GeoFunctionUtility::has_flag(layout_anchor(), LayoutAnchor::Bottom)){
        position.setY(psize.height() - size().height() - 10);
    }
    if(clz::GeoFunctionUtility::has_flag(layout_anchor(), LayoutAnchor::VCenter)){
        position.setY(psize.height() / 2 - size().height() / 2);
    }
    move(position);
}

void clz::ClzLayout::reset()
{
    resize_event(m_max_size);
}

QPoint clz::ClzLayout::pos()
{
    return m_position;
}

QSize clz::ClzLayout::size()
{
    return m_size;
}

void clz::ClzLayout::move(QPoint position)
{
    m_position = position;
}

void clz::ClzLayout::resize(QSize size)
{
    m_size = size;
}

clz::LayoutAnchor clz::ClzLayout::layout_anchor() const
{
    return m_anchor;
}

void clz::ClzLayout::set_layout_anchor(clz::LayoutAnchor anchor)
{
    m_anchor = anchor;
}

void clz::ClzLayout::set_sensitive_width_fold(bool sensitive)
{
    m_sensitive_with_fold = sensitive;
}

void clz::ClzLayout::set_max_size(QSize size)
{
    m_max_size = size;
}

void clz::ClzLayout::set_enable_max_size(bool value)
{
    m_enable_max_size = value;
}
