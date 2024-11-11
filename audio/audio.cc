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
                    break;
            }
        }
        packetcount = packetcount + 1;
        if(packetcount == 20) break;
    }
    av_packet_unref(packet);
    av_frame_free(&frames);
    avcodec_free_context(&avcontext);
    avformat_free_context(context);
}

void audio_encoding_error(char *errormsg)
{
    perror(errormsg);
    std::cout << std::endl;
}