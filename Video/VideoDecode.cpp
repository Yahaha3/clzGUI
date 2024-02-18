#include "VideoDecode.h"
#include <iostream>
#include <QImage>
#include <QTimer>
#include "UDPReveiver.h"
#include "common/CherileeCommon.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavdevice/avdevice.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/dict.h>
    #include <libavutil/hwcontext_qsv.h>
}

static AVPixelFormat m_hw_pix_format = AV_PIX_FMT_CUDA;
static qint64 frontDataLen = 3*512*1024;
static qint64 frontRdata = 0;
static bool   b_already_throwdata = false;

void check_ffmpeg_error(int result, size_t size){
    char* error_info = new char[32];
    av_strerror(result, error_info, size);
    std::cout << "[FFMPEG] open error: " << error_info << std::endl;
}

int read_udp_packet(void *opaque, uint8_t *buf, int buf_size){

//    clz::UDPReveiver *rev = reinterpret_cast<clz::UDPReveiver*>(opaque);
//    if(!rev) return -1;
    static QUdpSocket udpsocket;
//    if(!udpsocket)
    {
//        udpsocket = new QUdpSocket();
        auto bind = udpsocket.bind(QHostAddress::AnyIPv4, 8000, QUdpSocket::ShareAddress);
        if(bind){
            udpsocket.joinMulticastGroup(QHostAddress("226.0.1.101"));
        }
    }

    int rd_len = -1;

    do {
        QByteArray datagram;
//        rd_len = rev->read_socket(datagram, buf_size);
        rd_len = udpsocket.readDatagram(datagram.data(), buf_size);
        qDebug() << "rd len: " << rd_len;
        buf = (unsigned char*)datagram.data();
        if(rd_len > 0){
            if(frontRdata < frontDataLen){
                frontRdata += rd_len;
            } else {
                break;
            }
        }
    } while (frontRdata < frontDataLen && rd_len > 0 && !b_already_throwdata);

    if(rd_len > 0 && !b_already_throwdata && frontRdata >= frontDataLen){
        b_already_throwdata = true;
        return -1;
    }

    if(rd_len > 0){
        return rd_len;
    } else{
        frontRdata = 0;
        b_already_throwdata = false;
    }

    return -1;
}

static AVPixelFormat _3_1_1_get_qsv_format(AVCodecContext* avctx, const enum AVPixelFormat *pix_fmts);
static enum AVPixelFormat find_fmt_by_hw_type(const enum AVHWDeviceType type);
static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);

static int interrupt_callback(void* context){
    QTime *start_time = (QTime *)(context);
    if(start_time == NULL) return 0;
    if(QTime::currentTime() > *start_time){
        // timeout
        return 1;
    }
    return 0;
}

clz::VideoDecode::VideoDecode(QObject *parent) : QThread(parent)
{
    m_owidth = 800;
    m_oheight = 600;
    m_timeout = new QTimer();
    connect(m_timeout, &QTimer::timeout, this, &clz::VideoDecode::slot_global_timeout);
    m_udp_reveiver = std::make_shared<clz::UDPReveiver>();
    std::cout << "initialization ffmpeg [version]:" << avcodec_version() << std::endl;
}

void clz::VideoDecode::run()
{
    int count = 0;
    while (true) {
        if(m_init_state != FINISH) {
            msleep(100);
            continue;
        }
        auto ret = av_read_frame(m_avformat_context.get(), m_av_packet);
        if( ret >= 0
        && m_av_packet->stream_index == AVMEDIA_TYPE_VIDEO){
            // 匹配视频流
            count ++;
            int ret = avcodec_send_packet(m_avcodec_context, m_av_packet);
            ret = avcodec_receive_frame(m_avcodec_context, m_frame_in);
            if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) continue;
            if(ret < 0) continue;
#ifdef HARDWARE_SPEED
            if(m_frame_in->format == m_hw_pix_format) {
                // 执行硬解码
                ret = av_hwframe_transfer_data(m_video_frame, m_frame_in, 0);
                if(ret < 0) continue;
                sws_scale(m_sws_context, (const unsigned char* const*)m_video_frame->data, m_video_frame->linesize,
                          0,
                          m_avcodec_context->height,
                          m_frame_rgb->data, m_frame_rgb->linesize);
            }
#else
            {
                // 软解码
                sws_scale(m_sws_context, (const unsigned char* const*)m_frame_in->data, m_frame_in->linesize,
                          0,
                          m_avcodec_context->height,
                          m_frame_rgb->data, m_frame_rgb->linesize);
            }
#endif
//            QImage image(output, m_avcodec_context->width, m_avcodec_context->height, QImage::Format_RGB32);
//            image.save(QString("D:/TEST/video%1.png").arg(count), "PNG", 100);
            emit sig_video_info_decoded((unsigned char*)m_outbuffer, m_owidth, m_oheight);
            msleep(1000.0f / m_fps);
        } else {
            check_ffmpeg_error(ret, 1024);
            msleep(1);
        }
        av_packet_unref(m_av_packet);
    }
}

void clz::VideoDecode::set_video_file(const QString &path)
{
    m_video_path = path;
}

void clz::VideoDecode::ffmpeg_init()
{

    avdevice_register_all();
    avformat_network_init();

    m_init_state = VideoInitState::IDLE;

    bool result = false;
    result = _0_build_options();
    if(!result) goto error;
    m_init_state = VideoInitState::OPTION_BUILD;
//    m_avformat_context->avio_flags |= AVIO_FLAG_NONBLOCK;
//    m_cur_time = QTime::currentTime().addSecs(3);
    result = _1_open_input();
    if(!result) goto error;
    m_init_state = VideoInitState::INPUT_OPEN;

    result = _2_find_stream_and_decode();
    if(!result) goto error;
    m_init_state = VideoInitState::STREAM_DECODE_FIND;

    result = _3_match_hardware();
    if(!result) goto error;
    m_init_state = VideoInitState::MATCH_HARDWARE;

    result = _6_alloc_pack_and_frame();
    if(!result) goto error;
    m_init_state = VideoInitState::ALLOC_PACKET;

    result = _7_do_transcode_initialization();
    if(!result) goto error;
    m_init_state = VideoInitState::TRANSCODE;
    m_timeout->start(10);
    return;
error:
    m_timeout->start(100);
}

void clz::VideoDecode::reset()
{

}

void clz::VideoDecode::close()
{
    av_packet_free(&m_av_packet);
    avcodec_close(m_avcodec_context);
    avformat_free_context(m_avformat_context.get());
}

void clz::VideoDecode::update_decode_size(int w, int h)
{
    m_owidth = w;
    m_oheight = h;
    // 尺寸更改后需要重新初始化转码
    _7_do_transcode_initialization();
}

void clz::VideoDecode::slot_global_timeout()
{
    if(m_init_state != VideoInitState::NOTREADY){
        switch (m_init_state) {
        default:
            return;
        case FINISH:
            m_timeout->stop();
//            this->start();
            return;
        case OPTION_BUILD:
        {
            if(_1_open_input()){
                m_init_state = INPUT_OPEN;
            }
            break;
        }
        case INPUT_OPEN:
        {
            if(_2_find_stream_and_decode()){
                m_init_state = STREAM_DECODE_FIND;
            }
            break;
        }
        case STREAM_DECODE_FIND:
        {
            if(_3_match_hardware()){
                m_init_state = MATCH_HARDWARE;
            }
            break;
        }
        case MATCH_HARDWARE:
        {
            if(_6_alloc_pack_and_frame()){
                m_init_state = ALLOC_PACKET;
            }
            break;
        }
        case ALLOC_PACKET:
        {
            if(_7_do_transcode_initialization()){
                m_init_state = TRANSCODE;
            }
            break;
        }
        case TRANSCODE:
        {
            m_init_state = FINISH;
            break;
        }
        }
    }
}

bool clz::VideoDecode::_0_build_options()
{
    {
        // 0, build options
        m_options = NULL;
        av_dict_set(&m_options, "buffer_size", "8192000", 0);
        av_dict_set(&m_options, "rtsp_transport", "tcp", 0);
        av_dict_set(&m_options, "stimeout", "3000000", 0);
        av_dict_set(&m_options, "max_delay", "1000000", 0);
        av_dict_set(&m_options, "threads", "auto", 0);
//        av_dict_set_int(&m_options, "multiple_requests", 1, 0);
//        av_dict_set_int(&m_options, "read_ahead_limit", INT_MAX, 0);
        av_dict_set(&m_options, "listen_timeout", "3", 0);
    }
    return true;
}

bool clz::VideoDecode::_1_open_input()
{
    m_avformat_context.reset(avformat_alloc_context());
//    m_avformat_context->interrupt_callback.callback = interrupt_callback;
//    m_avformat_context->interrupt_callback.opaque = (void*)(&m_cur_time);
    {
        // 1, open video m_video_path.toStdString().c_str()
#if 1
        int avformat_open_result;
        if(m_video_path.startsWith("tcp")){
            int bufsize = 1024*400;

            unsigned char* buffer = (unsigned char*)av_malloc(bufsize);
            m_udp_reveiver->init();
            m_avformat_context->pb = avio_alloc_context(buffer, bufsize, 0, m_udp_reveiver.get(), read_udp_packet, NULL, NULL);
            if(m_avformat_context->pb == 0){
                return false;
            }
            auto ctx = m_avformat_context.get();
            avformat_open_result = avformat_open_input(&ctx, 0, NULL, &m_options);
        }
        else
        {
            const AVInputFormat *ifmt = av_find_input_format("h264");
            auto uu = m_video_path.toUtf8();
            auto ctx = m_avformat_context.get();
            avformat_open_result = avformat_open_input(&ctx, uu.data(), ifmt, &m_options);
        }
        if(avformat_open_result < 0){
            //error
            check_ffmpeg_error(avformat_open_result, 1024);
            return false;
        }

        if (m_options != NULL) {
            av_dict_free(&m_options);
        }
#else
        int bufsize =1024 * 400;
        unsigned char* buffer = (unsigned char*)av_malloc(bufsize);
        m_reveiver = new clz::UDPReveiver();
        m_reveiver->init();
        m_avformat_context->pb = avio_alloc_context(buffer, bufsize, 0, m_reveiver, read_udp_packet, NULL, NULL);
        int avformat_open_result = avformat_open_input(&m_avformat_context, 0, NULL, &m_options);
        if(avformat_open_result != 0){
            //error
            check_ffmpeg_error(avformat_open_result, 1024);

        }
#endif
    }
    return true;
}

bool clz::VideoDecode::_2_find_stream_and_decode()
{
    {
        // 2, find video stream info

        int avformat_find_stream_info_result = avformat_find_stream_info(m_avformat_context.get(), NULL);
        if(avformat_find_stream_info_result < 0){
            check_ffmpeg_error(avformat_find_stream_info_result, 1024);
            return false;
        }

        // 3, find video decoder
//        int av_stream_index = -1;
//        const AVCodec* codec;
//        m_video_stream_index = av_find_best_stream(m_avformat_context, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
//        for(int i = 0, si = m_avformat_context->nb_streams; i < si; i++){
//            if(m_avformat_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
//                m_video_stream_index = i;
//                break;
//            }
//        }
        m_video_stream_index = av_find_best_stream(m_avformat_context.get(), AVMEDIA_TYPE_VIDEO, -1, -1, &m_video_codec, 0);
        if(m_video_stream_index == -1) {
            std::cout << "find video stream failed! " << std::endl;
            return false;
        }
    }
    return true;
}

bool clz::VideoDecode::_3_match_hardware()
{
    auto stream = m_avformat_context->streams[m_video_stream_index];
    m_fps = av_q2d(stream->avg_frame_rate);
    auto avcodec_par = stream->codecpar;
    m_video_codec = avcodec_find_decoder(avcodec_par->codec_id);
    if(m_video_codec == NULL){
        std::cout << "find video decoder failed! " << std::endl;
        return false;
    }
#ifdef HARDWARE_SPEED
    enum AVHWDeviceType type;
    auto hardwares = clz::CherileeCommon::get_hardware_names();
    m_hardware_name = "none";
    for(auto hardware: hardwares){
        type = av_hwdevice_find_type_by_name(hardware.toStdString().c_str());
        if(type == AV_HWDEVICE_TYPE_NONE) continue;
        for(int i = 0 ;; i++){
            auto config = avcodec_get_hw_config(m_video_codec, i);
            if(!config) break;

            if(config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == type){
                m_hw_pix_format = config->pix_fmt;
                m_hardware_name = hardware;
                break;
            }
        }
    }
    // 因为没有硬解码可以不管，所以这里返回true，如果后续需要强制硬解码，可以考虑修改一下返回
    if(m_hardware_name == "none") return true;

    if(m_hardware_name == "qsv"){
        return _3_1_init_hardware_device_qsv();
    } else {
        return _3_2_init_hardware_device_other(m_hardware_name);
    }
#else
    m_avcodec_context = avcodec_alloc_context3(avcodec);

    // 4, synchronization avcodecparameters
    avcodec_parameters_to_context(m_avcodec_context, avcodec_par);
    // 5, open decoder
    int avcodec_open2_result = avcodec_open2(m_avcodec_context, avcodec, NULL);
    if(avcodec_open2_result != 0){
        check_ffmpeg_error(avcodec_open2_result, 1024);
    }
    return true;
#endif
}

bool clz::VideoDecode::_6_alloc_pack_and_frame()
{
    // 6, print dump infomation when exit
    av_dump_format(m_avformat_context.get(), 0, m_video_path.toStdString().c_str(), 0);

    m_av_packet = av_packet_alloc();
    m_frame_in = av_frame_alloc();
    m_frame_rgb = av_frame_alloc();
    m_video_frame = av_frame_alloc();
    return true;
}

bool clz::VideoDecode::_7_do_transcode_initialization()
{
    if(!m_avcodec_context) return false;
    AVPixelFormat format = AV_PIX_FMT_BGR32;
#ifdef HARDWARE_SPEED
    AVPixelFormat srcformat = AV_PIX_FMT_NV12;
#else
    AVPixelFormat srcformat = m_avcodec_context->pix_fmt;
#endif
    m_outbuffer = (uint8_t*)av_malloc(av_image_get_buffer_size(
                                      format,m_owidth, m_oheight,1));
    av_image_fill_arrays(m_frame_rgb->data, m_frame_rgb->linesize,
                         m_outbuffer, format, m_owidth, m_oheight, 1);

    // 转码初始化
    m_sws_context = sws_getContext(m_avcodec_context->width,
                                    m_avcodec_context->height,
                                   srcformat,
                                    m_owidth, m_oheight,
                                   format, SWS_BICUBIC, NULL, NULL, NULL);
    return true;
}

bool clz::VideoDecode::_3_1_init_hardware_device_qsv()
{
#ifdef HARDWARE_SPEED
    int result = av_hwdevice_ctx_create(&m_hw_decode_ctx.hw_device_ref, AV_HWDEVICE_TYPE_QSV, "auto", NULL, 0);
    if(result < 0){
        check_ffmpeg_error(result, 1024);
        return false;
    }

    auto avcodec = avcodec_find_decoder_by_name("h264_qsv");
    if(avcodec == NULL) {
        return false;
    }

    auto stream = m_avformat_context->streams[m_video_stream_index];
    m_avcodec_context = avcodec_alloc_context3(avcodec);
    m_avcodec_context->codec_id = AV_CODEC_ID_H264;

    m_avcodec_context->flags  |= AV_CODEC_FLAG_GLOBAL_HEADER;
    m_avcodec_context->flags  |= AV_CODEC_FLAG_LOW_DELAY;
    m_avcodec_context->flags2 |= AV_CODEC_FLAG2_FAST;

    if(stream->codecpar->extradata_size){
        m_avcodec_context->extradata = (uint8_t *)av_mallocz(stream->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if(!m_avcodec_context->extradata) return false;

        memcpy(m_avcodec_context->extradata, stream->codecpar->extradata, stream->codecpar->extradata_size);
        m_avcodec_context->extradata_size = stream->codecpar->extradata_size;
    }

    m_avcodec_context->opaque = &m_hw_decode_ctx;
    m_avcodec_context->get_format = _3_1_1_get_qsv_format;

    // 5, open decoder
    int avcodec_open2_result = avcodec_open2(m_avcodec_context, avcodec, NULL);
    if(avcodec_open2_result != 0){
        check_ffmpeg_error(avcodec_open2_result, 1024);
        return false;
    }

#endif
    return true;
}

bool clz::VideoDecode::_3_2_init_hardware_device_other(const QString& hardware)
{
#ifdef HARDWARE_SPEED
    QByteArray hwd = hardware.toUtf8();
    enum AVHWDeviceType type = av_hwdevice_find_type_by_name(hwd.data());
    m_hw_pix_format = find_fmt_by_hw_type(type);
    if(m_hw_pix_format == -1) return false;

    auto stream = m_avformat_context->streams[m_video_stream_index];
    auto avcodec_par = stream->codecpar;
    m_video_codec = avcodec_find_decoder(avcodec_par->codec_id);
    m_avcodec_context = avcodec_alloc_context3(m_video_codec);

    // 4, synchronization avcodecparameters
    int result = avcodec_parameters_to_context(m_avcodec_context, avcodec_par);
    if(result < 0){
        check_ffmpeg_error(result, 1024);
        return false;
    }
    m_avcodec_context->get_format = get_hw_format;
    AVBufferRef *hw_device_ref;
    result = av_hwdevice_ctx_create(&hw_device_ref, type, NULL, NULL, 0);
    if(result < 0){
        check_ffmpeg_error(result, 1024);
        return false;
    }
    m_avcodec_context->hw_device_ctx = av_buffer_ref(hw_device_ref);
    av_buffer_unref(&hw_device_ref);

    m_avcodec_context->flags  |= AV_CODEC_FLAG_GLOBAL_HEADER;
    m_avcodec_context->flags  |= AV_CODEC_FLAG_LOW_DELAY;
    m_avcodec_context->flags2 |= AV_CODEC_FLAG2_FAST;

    // 5, open decoder
    int avcodec_open2_result = avcodec_open2(m_avcodec_context, m_video_codec, NULL);
    if(avcodec_open2_result != 0){
        check_ffmpeg_error(avcodec_open2_result, 1024);
        return false;
    }

    m_owidth = stream->codecpar->width;
    m_oheight = stream->codecpar->height;

    if(m_owidth == 0 || m_oheight == 0){
        return false;
    }

#endif
    return true;
}

AVPixelFormat _3_1_1_get_qsv_format(AVCodecContext *avctx, const AVPixelFormat *pix_fmts)
{
#ifndef gcc45
    while (*pix_fmts != AV_PIX_FMT_NONE) {
        if (*pix_fmts == AV_PIX_FMT_QSV) {
            struct DecodeContext {
                AVBufferRef *hw_device_ref;
            };

            DecodeContext *decode = (DecodeContext *)avctx->opaque;
            avctx->hw_frames_ctx = av_hwframe_ctx_alloc(decode->hw_device_ref);
            av_buffer_unref(&decode->hw_device_ref);

            if (!avctx->hw_frames_ctx) {
                return AV_PIX_FMT_NONE;
            }

            AVHWFramesContext *frames_ctx = (AVHWFramesContext *)avctx->hw_frames_ctx->data;
            AVQSVFramesContext *frames_hwctx = (AVQSVFramesContext *)frames_ctx->hwctx;

            frames_ctx->format = AV_PIX_FMT_QSV;
            frames_ctx->sw_format = avctx->sw_pix_fmt;
            frames_ctx->width = FFALIGN(avctx->coded_width, 32);
            frames_ctx->height = FFALIGN(avctx->coded_height, 32);
            frames_ctx->initial_pool_size = 32;
            frames_hwctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

            int ret = av_hwframe_ctx_init(avctx->hw_frames_ctx);
            if (ret < 0) {
                return AV_PIX_FMT_NONE;
            }

            //qDebug() << TIMEMS << "get_qsv_format ok";
            return AV_PIX_FMT_QSV;
        }

        pix_fmts++;
    }
#endif

//    qDebug() << TIMEMS << "The QSV pixel format not offered in get_format()";
    return AV_PIX_FMT_NONE;
}

AVPixelFormat find_fmt_by_hw_type(const AVHWDeviceType type)
{
    enum AVPixelFormat fmt;
    switch (type) {
        case AV_HWDEVICE_TYPE_VAAPI:
            fmt = AV_PIX_FMT_VAAPI;
            break;
        case AV_HWDEVICE_TYPE_DXVA2:
            fmt = AV_PIX_FMT_DXVA2_VLD;
            break;
        case AV_HWDEVICE_TYPE_D3D11VA:
            fmt = AV_PIX_FMT_D3D11;
            break;
        case AV_HWDEVICE_TYPE_VDPAU:
            fmt = AV_PIX_FMT_VDPAU;
            break;
        case AV_HWDEVICE_TYPE_VIDEOTOOLBOX:
            fmt = AV_PIX_FMT_VIDEOTOOLBOX;
            break;
        case AV_HWDEVICE_TYPE_CUDA:
            fmt = AV_PIX_FMT_CUDA;
            break;
        case AV_HWDEVICE_TYPE_QSV:
            fmt = AV_PIX_FMT_QSV;
            break;
        case AV_HWDEVICE_TYPE_DRM:
            fmt = AV_PIX_FMT_DRM_PRIME;
            break;
        case AV_HWDEVICE_TYPE_OPENCL:
            fmt = AV_PIX_FMT_OPENCL;
            break;
        case AV_HWDEVICE_TYPE_MEDIACODEC:
            fmt = AV_PIX_FMT_MEDIACODEC;
            break;
        default:
            fmt = AV_PIX_FMT_NONE;
            break;
    }

    return fmt;
}

AVPixelFormat get_hw_format(AVCodecContext *ctx, const AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;
    for (p = pix_fmts; *p != -1; p++) {
        if (*p == m_hw_pix_format) {
            //qDebug() << TIMEMS << "get_hw_format ok";
            return *p;
        }
    }
}
