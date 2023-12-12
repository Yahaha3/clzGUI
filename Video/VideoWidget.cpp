#include "VideoWidget.h"

clz::VideoWidget::VideoWidget(clz::ClzWidget *parent) : ClzWidget(parent)
{
    m_video = std::make_shared<clz::ImageDrawElement>();
    set_video_decoder(new clz::VideoDecode());
//    m_video_decode = std::make_shared<clz::VideoDecode>();

//    m_udp_reveiver = std::make_shared<clz::UDPReveiver>();
//    connect(m_udp_reveiver.get(), &clz::UDPReveiver::sig_video_info_decoded, this, &VideoWidget::slot_video_decode_info);
}

void clz::VideoWidget::init()
{
#if 1
//    m_video_decode->set_video_file("rtsp://127.0.0.1:8554/stream");
//    m_video_decode->quit();
    auto decode_size = size();
    if(!clz::GeoFunctionUtility::has_flag(m_window_flags, ImGuiWindowFlags_NoTitleBar)){
        decode_size.setHeight(decode_size.height() - 38);
    }
    m_video_decode->update_decode_size(decode_size.width(), decode_size.height());
    m_video_decode->ffmpeg_init();
    m_video_decode->start();
#else
    m_udp_reveiver->init();
#endif
}

void clz::VideoWidget::paint()
{
    ImGui::SetNextWindowPos(impos());
    ImGui::SetNextWindowSize(imsize());
    ImGui::Begin(name().toStdString().c_str(), 0, m_window_flags);
    m_widget_fold = ImGui::IsWindowCollapsed();
    if(!m_video) return;
    auto ID = m_video->get_image_texture_id();
    if(ID){
        ImGui::Image(ID, {(float)m_video->width(), (float)m_video->height()});
    }
    ImGui::End();
}

void clz::VideoWidget::set_video_decoder(clz::VideoDecode *decoder)
{
    m_video_decode.reset(decoder);
    connect(m_video_decode.get(), &clz::VideoDecode::sig_video_info_decoded, this, &VideoWidget::slot_video_decode_info);
}

clz::VideoDecode *clz::VideoWidget::video_decoder()
{
    return m_video_decode.get();
}

void clz::VideoWidget::resizeEvent()
{
    auto decode_size = size();
    if(!clz::GeoFunctionUtility::has_flag(m_window_flags, ImGuiWindowFlags_NoTitleBar)){
        decode_size.setHeight(decode_size.height() - 38);
    }
    m_video_decode->update_decode_size(decode_size.width(), decode_size.height());
}

void clz::VideoWidget::slot_video_decode_info(uchar *data, int w, int h)
{
    if(m_video){
        m_video->update_image(data, w, h);
    }
}
