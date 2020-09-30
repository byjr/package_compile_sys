#ifdef __cplusplus
extern "C" {
#endif
#ifndef mixer_ctrl_H_
#define mixer_ctrl_H_
#include <alsa/asoundlib.h>

typedef struct mixer_ctrl_par_s {
    const char *device;
    const char *name;
    size_t idx;
    long int volMin;
    long int volMax;
} mixer_ctrl_par_t;

typedef struct mixer_ctrl_s {
    mixer_ctrl_par_t *mPar;
    snd_mixer_t *mHdl;
    snd_mixer_elem_t *mVolElem;
    snd_mixer_selem_id_t *mSelemId;
    size_t mChannel;
    long mVolume;
    pthread_mutex_t *mMtx;
} mixer_ctrl_t;

typedef enum mixer_channel_s {
    MIXER_CHANNEL_ALL = 0,
    MIXER_CHANNEL_LEFT,
    MIXER_CHANNEL_RIGHT,
} mixer_channel_t;

mixer_ctrl_t *mixer_ctrl_create(mixer_ctrl_par_t *par);
int mixer_ctrl_destroy(mixer_ctrl_t *ptr);

int mixer_ctrl_setChannel(mixer_ctrl_t *ptr, size_t channel);
int mixer_ctrl_setVol(mixer_ctrl_t *ptr, long int vol);
long int mixer_ctrl_getVol(mixer_ctrl_t *ptr);
ssize_t mixer_ctrl_getChannel(mixer_ctrl_t *ptr);
#endif

#ifdef __cplusplus
}
#endif