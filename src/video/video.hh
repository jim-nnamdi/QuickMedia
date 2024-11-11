
#include <iostream>
#include <string>
#include <memory>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
}

template <typename ... Ts>
class Video {
    public:
    void Video();
    void Video(const Video& video);
    void read_video_frames(const char* videofile);
    void process_video_frames(Ts ... ts);

    /* default index value for the video stream */
    static constexpr int video_stream_index = -1;
    void video_encoding_error(char* errormsg);
};