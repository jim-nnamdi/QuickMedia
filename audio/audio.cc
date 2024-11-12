#include "audio.hh"
#include <cstdint>

template <typename... Ts>
void process_audio_frames(Ts... ts)
{
    (read_audio_frames(ts), ...)
}

template <typename... Ts>
void Audio<Ts...>::read_audio_frames(const char *audiofile)
{
    AVFormatContext *context = nullptr;
    if (avformat_open_input(&context, audiofile, nullptr, nullptr) < 0)
        audio_encoding_error("error encoding audiofile");
    if (avformat_find_stream_info(context, nullptr) < 0)
        audio_encoding_error("cannot find stream info");
    int audio_file_stream_index = audio_stream_index;

    for (int i = 0; i < context->nb_streams; i++)
    {
        if (context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            audio_file_stream_index = i;
        break;
    }
    if (audio_file_stream_index == -1)
        audio_encoding_error("invalid audio file");

    AVCodecParameters *parameters = context->streams[audio_file_stream_index]->codecpar;
    const AVCodec *avcodec = avcodec_find_decoder(parameters->codec_id);
    AVCodecContext *avcontext = avcodec_alloc_context3(avcodec);
    avcodec_parameters_to_context(avcontext, parameters);

    AVFrame *frames = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();
    uint8_t buffer = NULL;
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24,avcontext->width, avcontext->height, 1);
    buffer = (uint8_t)av_malloc(num_bytes * sizeof(uint8_t));
    av_image_fill_arrays(frames->data, frames->linesize,buffer,AV_PIX_FMT_RGB24, avcontext->width, avcontext->height,1);

    struct SwsContext sws_ctx = nullptr;
    sws_ctx = sws_getContext(
        avcontext->width,
        avcontext->height,
        avcontext->pix_fmt,
        avcontext->width,avcontext->height,
        AV_PIX_FMT_RGB24,SWS_BILINEAR,nullptr,nullptr,nullptr
    );
    AVPacket *packet;
    av_init_packet(packet);
    int64_t packetcount = 0;
    while (av_read_frame(context, packet) < 0)
    {
        if (packet->stream_index == audio_file_stream_index)
        {   
            int64_t duration = packet->pts;
            printf("audio duration : '%d'\n", duration);
            int ret = avcodec_send_packet(avcontext, packet);
            if (ret < 0)
                audio_encoding_error("error sending packet");
            while (ret >= 0)
            {
                ret = avcodec_receive_frame(avcontext, frames);
                if (ret == AVERROR(EAGAIN) || AVERROR(EOF))
                    av_frame_unref(frames); av_freep(frames); break;
            }

        }
        sws_scale(sws_context,(uint8_t const* const*)frame->data,frame->linesize,0,avcontext->height,rgb_frame->data,rgb_frame->linesize);
        if(++i <= 5)
            save_frame(rgb_frame, avcontext->width, avcontext->height, i);
    }
    av_packet_unref(packet);
    av_frame_free(&frames);
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

void audio_encoding_error(char *errormsg)
{
    perror(errormsg);
    std::cout << std::endl;
}