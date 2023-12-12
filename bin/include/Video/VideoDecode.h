#ifndef VIDEODECODE_H
#define VIDEODECODE_H

#include <QObject>
#include <QThread>
#include <QTime>
#include <QTimer>
#include "clzGUI_global.h"


class AVPacket;
class AVFrame;
class AVCodecContext;
class AVFormatContext;
class SwsContext;
class AVDictionary;
class AVBufferRef;
class AVCodec;

namespace clz {

class UDPReveiver;

enum VideoInitState {
    NOTREADY,
    IDLE,
    OPTION_BUILD,
    INPUT_OPEN,
    STREAM_DECODE_FIND,
    MATCH_HARDWARE,
    ALLOC_PACKET,
    TRANSCODE,
    FINISH,
};

class CLZGUI_EXPORT VideoDecode : public QThread
{
    Q_OBJECT
public:
    VideoDecode(QObject* parent = 0);
    void run() override;

    void set_video_file(const QString& path);

    void ffmpeg_init();
    // 中途更换播放源
    void reset();

    void close();

    void update_decode_size(int w, int h);

signals:
    void sig_video_info_decoded(uchar* data, int w, int h);

private slots:
    void slot_global_timeout();

private:
    bool _0_build_options();
    bool _1_open_input();
    bool _2_find_stream_and_decode();
    bool _3_match_hardware();
    bool _6_alloc_pack_and_frame();
    bool _7_do_transcode_initialization();

    bool _3_1_init_hardware_device_qsv();
    bool _3_2_init_hardware_device_other(const QString& hardware);
private:
    struct DecodeContext {
        AVBufferRef *hw_device_ref;
    };

    QString m_video_path;

    AVCodecContext* m_avcodec_context = NULL;
    AVFormatContext* m_avformat_context = NULL;
    SwsContext* m_sws_context = NULL;
    AVDictionary* m_options = NULL;
    // 帧缓存数据
    AVPacket* m_av_packet = NULL;
    // 帧输入
    AVFrame* m_frame_in = NULL;
    // 帧输出
    AVFrame* m_frame_rgb = NULL;
    // 视频帧
    AVFrame* m_video_frame = NULL;

    // 视频流索引
    int m_video_stream_index = -1;
    // 缓冲区
    uint8_t* m_outbuffer;
    // 输出尺寸
    int m_owidth, m_oheight;
    // time
    QTime m_cur_time;
    // 硬解码组件
    QString m_hardware_name = "none";
    DecodeContext m_hw_decode_ctx;

    QTimer* m_timeout;
    VideoInitState m_init_state = NOTREADY;
    double m_fps;
};

}

#endif // VIDEODECODE_H
