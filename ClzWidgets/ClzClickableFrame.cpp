#pragma execution_character_set("utf-8")

#include "ClzClickableFrame.h"
#include "imgui.h"

clz::ClzClickableFrame::ClzClickableFrame(clz::ClzWidget *parent) : ClzWidget(parent)
{
    set_window_flag(ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoMove);

    m_frame_image = std::make_shared<clz::ImageDrawElement>();
    resize(QSize(430,270));
}

void clz::ClzClickableFrame::init()
{
    auto path = "C:/Users/lan19/PhugiaOS/admin/entity/image/mainImage/ENTITY_0a7jrm99mc.png";
    bool ok = false;
    auto array = clz::GeoFunctionUtility::get_image_bytearray(path, ok);
    if(ok){
        m_frame_image->load_image(array);
    }
}

void clz::ClzClickableFrame::paint()
{
    ImGui::SetNextWindowPos(impos());
    ImGui::SetNextWindowSize(imsize());
    ImGui::SetNextWindowBgAlpha(0.2f);
    ImGui::Begin(name().toStdString().c_str(), 0, m_window_flags);
//    ImGui::SetWindowFontScale(2);
    auto right_width = ImGui::CalcTextSize(m_info_right.toStdString().c_str()).x;
    auto width = ImGui::GetWindowWidth();
    if(m_frame_image){
        auto ID = m_frame_image->get_image_texture_id();
        ImGui::Text(m_info_left.toStdString().c_str(), 0);
        ImGui::SameLine();
        ImGui::SetCursorPosX(width - right_width);
        ImGui::Text(m_info_right.toStdString().c_str(), 0);
        ImGui::Image(ID, imsize());
    }

    {
        // 捕获鼠标点击事件
        auto io = ImGui::GetIO();
        if(ImGui::IsWindowFocused()){
            if(io.MouseReleased[0]){
                auto position = check_current_mouse_position(io.MousePos);
                QString info = "no in frame";
                if(position < 0){
                    info = "left";
                } else if(position > 0) {
                    info = "right";
                }
                std::cout << "left btn cilck frame release: " << info.toStdString() << std::endl;
            }
        }
    }

    ImGui::End();
}

void clz::ClzClickableFrame::update_left_info(const QString &left)
{
    m_info_left = left;
}

void clz::ClzClickableFrame::update_right_info(const QString &right)
{
    m_info_right = right;
}

void clz::ClzClickableFrame::set_background_image(const QString &path)
{
    // test
//    QString pp = clz::GeoFunctionUtility::kernel_apps_dir() + "/appcontainers/eqnx_ui/res/icon/tool_bar/aoa/RC_Warn.png";
    if(m_image_path != path){
        bool ok = false;
        auto array = clz::GeoFunctionUtility::get_image_bytearray(path, ok);
        if(ok){
            m_frame_image->load_image(array);
        }
        m_image_path = path;
    }
}

int clz::ClzClickableFrame::check_current_mouse_position(const ImVec2& position)
{
    auto contains = [](const ImVec2& tl, const ImVec2& br, const ImVec2& p){
        return p.x >= tl.x && p.y >= tl.y && p.x < br.x && p.y < br.y;
    };

    ImVec2 left_tl, left_br, right_tl, right_br;
    auto _pos = impos();
    auto _size = imsize();
    left_tl = _pos;
    left_br = ImVec2(_pos.x + _size.x / 2, _pos.y + _size.y);
    right_tl = ImVec2(_pos.x + _size.x / 2, _pos.y);
    right_br = ImVec2(_pos.x + _size.x, _pos.y + _size.y);

    int reslt = 0;
    if(contains(left_tl, left_br, position)){
        reslt = -1;
    }
    if(contains(right_tl, right_br, position)){
        reslt = 1;
    }

    return reslt;
}
