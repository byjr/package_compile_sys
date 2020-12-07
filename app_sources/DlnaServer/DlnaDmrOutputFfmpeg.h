//
// Created by eddy on 18-1-30.
//

#ifndef DUEROS_DCS_APP_DLNA_DLNADMROUTPUTFFMPEG_H
#define DUEROS_DCS_APP_DLNA_DLNADMROUTPUTFFMPEG_H

#include <string>
#include <Dlna/DlnaDmrInterfaceOutput.h>
#include <Dlna/DlnaDmrSongMetaData.h>
#include <Player/AudioPlayerImpl.h>
#include <Player/AudioPlayerListener.h>

namespace duerOSDcsApp {
namespace dueros_dlna {
using namespace duerOSDcsApp::mediaPlayer;
struct track_time_info {
    long duration;
    long position;
};

class OutputFfmpeg:public IOutput,public AudioPlayerListener {
	std::shared_ptr<AudioPlayerImpl> m_mediaPlayer;
public:

    int init(void);

    void set_next_uri(const std::string uri);

    void set_uri(const std::string uri,
                 output_update_meta_cb_t meta_cb);

    int play(output_transition_cb_t callback);

    int stop(void);

    int pause(void);

    int seek(long position_nanos);

    int get_position(long* track_duration,
                     long* track_pos);

    int get_volume(float* v);

    int set_volume(float value);

    int get_mute(int* m);

    int set_mute(int m);

    int get_state(void);

    int resume(void);

    void onPlaybackStarted();

    void onPlaybackFinished();

    void onPlaybackStopped();

    void onPlaybackNearlyfinished();

    void onPlaybackError();

    void onPlaybackPaused();

    void onPlaybackResumed();

    void onBufferUnderrun();

    void onBufferRefilled();

    void onRecvFirstpacket();

};
}
}
#endif //DLNADEMO_OUTPUTGSTREAMER_H
