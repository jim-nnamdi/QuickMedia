#include "video.hh"
#include <cstdint>
#include <string>
#include <cstring>

template <typename... Ts>
void Video<Ts...>::process_video_frames(Ts... ts)
{
    (read_video_frames(ts), ...);
}

template <typename... Ts>
void Video<Ts...>::read_video_frames(const char *filename)
{
    AVFormatContext *context = nullptr;
    if (avformat_open_input(&context, filename, nullptr, nullptr) < 0)
        video_encoding_error("error opening file");
    if (avformat_find_stream_info(context, nullptr) < 0)
        video_encoding_error("cannot find video context");
    int video_stream_index_value = video_stream_index;

    for (int i = 0; i < context->nb_streams; i++)
    {
        if (context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            video_stream_index_value = i;
        break;
    }
    if (video_stream_index_value == -1)
        video_encoding_error("invalid video stream");

    AVCodecParameters *parameters = context->streams[video_stream_index_value]->codecpar;
    const AVCodec *avcodec = avcodec_find_decoder(parameters->codec_id);
    AVCodecContext *avcontext = avcodec_alloc_context3(avcodec);
    avcodec_parameters_to_context(avcontext, parameters);
    if (avcodec_open2(avcontext, avcodec, nullptr) < 0)
        video_encoding_error("cannot open context");

    AVFrame *frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();
    if(rgb_frame == nullptr) return -1;
    uint8_t buffer = nullptr;
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, avcontext->width, avcontext->height,1);
    buffer = (uint8_t)av_malloc(num_bytes * sizeof(uint8_t));
    av_image_fill_arrays(frame->height, frame->linesize,buffer,AV_PIX_FMT_RGB24,avcontext->width, avcontext->height, 1);

    struct SwsContext *sws_ctx = nullptr;
    sws_ctx = sws_getContext(avcontext->width, avcontext->height, avcontext->pix_fmt,avcontext->width, avcontext->height,AV_PIX_FMT_RGB24,SWS_BILINEAR, nullptr, nullptr, nullptr);
    if(!sws_ctx) fprintf(stderr, "Error initializing the sws context"); return -1;
    AVPacket *packet;
    av_init_packet(packet);
    int i = 0;
    while (av_read_frame(context, packet) < 0)
    {
        if (packet->stream_index == video_stream_index_value)
        {
            int64_t duration = packet->pts;
            printf("video duration : '%d'\n", duration);
            int ret = avcodec_send_packet(avcontext, packet);
            if (ret < 0)
                video_encoding_error("cannot send packet");
            while (ret >= 0)
            {
                ret = avcodec_receive_frame(avcontext, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR(EOF))
                    break;
            }
        }
        sws_scale(sws_context,(uint8_t const* const*)frame->data,frame->linesize,0,avcontext->height,rgb_frame->data,rgb_frame->linesize);
        if(++i <= 5)
            save_frame(rgb_frame, avcontext->width, avcontext->height, i);
    }
    av_free(buffer);
    av_packet_unref(packet);
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    avcodec_free_context(&avcontext);
    avformat_free_context(context);
}

void save_frame(AVFrame* rgb_frame, int width, int height, int iframe)
{
    FILE* pfile;
    std::string filename = "frame" + std::to_string(iframe) + ".ppm";
    pfile = fopen(filename.c_str(),"wb");
    if(!pfile) perror(filename.c_str()); return;
    fprintf(pfile, "P6\n%d %d\n255\n", width, height);
    for(int i = 0; i < height; ++i)
        fwrite(rgb_frame->data[0] + i * rgb_frame->linesize[0],1,width,pfile);
    fclose(pfile);

}

void video_encoding_error(char *errormsg)
{
    perror(errormsg);
    std::cout << std::endl;
}