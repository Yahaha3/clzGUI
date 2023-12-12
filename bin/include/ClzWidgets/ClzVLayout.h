#ifndef CLZVLAYOUT_H
#define CLZVLAYOUT_H

#include "ClzLayout.h"

namespace clz {
class CLZGUI_EXPORT ClzVLayout : public ClzLayout
{
public:
    ClzVLayout(ClzLayout* parent = 0);

    void relayout() override;
    
private:
    void slots_response_widget_mouse_wheel(float delta) override;

private:
    int m_bar_height = 15;
};
}

#endif // CLZVLAYOUT_H
