#ifndef AUDIO_H
#define AUDIO_H

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
class Audio {
    public:
    void Audio();
    void Audio(const Audio& audio);
    void read_audio_frames(const char* audiofile);
    void process_audio_frames(Ts ... ts);

    /* default index value for the audio stream */
    static constexpr int audio_stream_index = -1;
    void audio_encoding_error(char* errormsg);
};

#endif