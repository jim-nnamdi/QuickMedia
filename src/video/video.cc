#include "video.hh"

template <typename ... Ts>
void Video<Ts ...>::process_video_frames(Ts ... ts) {
    (read_video_frames(ts), ...);
}

/* reads incoming stream and fetches the video data */
template <typename ... Ts>
void Video<Ts ...>::read_video_frames(const char* filename) {
    AVFormatContext* context = nullptr;
    if(avformat_open_input(&context, filename, nullptr, nullptr) < 0)
        video_encoding_error("error opening file");
    if(avformat_find_stream_info(context, nullptr) < 0)
        video_encoding_error("cannot find video context");
    int video_stream_index_value = video_stream_index;
    
    /* read the number of incoming streams from the context */
    /* checks the codec parameters to fetch the codec type */
    for(int i = 0; i < context->nb_streams;i++){
        if(context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            video_stream_index_value = i; break;
    }
    if(video_stream_index_value == -1) video_encoding_error("invalid video stream");

    /* operations to get the codec and codec parameters */
    AVCodecParameters* parameters = context->streams[video_stream_index_value]->codecpar;
    const AVCodec* avcodec = avcodec_find_decoder(parameters->codec_id);
    AVCodecContext* avcontext = avcodec_alloc_context3(avcodec);
    avcodec_parameters_to_context(avcontext,parameters);
    if(avcodec_open2(avcontext,avcodec, nullptr) < 0) 
        video_encoding_error("cannot open context");
    
    /* memory allocation for the frames and packets */
    AVFrame* frame = av_frame_alloc(); AVPacket* packet;
    av_init_packet(packet);
    while(av_read_frame(context,packet) < 0 ){
        /* match the packet stream index to the video stream index */
        /* if value matches send packet and recieve frame data */
        if(packet->stream_index == video_stream_index_value) {
            int ret = avcodec_send_packet(avcontext,packet);
            if (ret < 0) video_encoding_error("cannot send packet");
            while(avcodec_receive_frame(avcontext, frame)){
                ret = avcodec_receive_frame(avcontext, frame);
                if(ret == AVERROR(EAGAIN) || ret == AVERROR(EOF)) break;
            }
        }
    }
    av_packet_unref(packet);
    av_frame_free(&frame);
    avcodec_free_context(&avcontext);
    avformat_free_context(context);
}

void video_encoding_error(char* errormsg) {
    perror(errormsg);
    std::cout << std::endl;
}