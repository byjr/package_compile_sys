#include "mixer_ctrl.h"
#include "../slog/slog.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int _mixer_ctrl_destroy(mixer_ctrl_t *ptr) {
	int res = 0;
	if(!ptr) {
		return 0;
	}
	if(ptr->mHdl) {
		res = snd_mixer_close(ptr->mHdl);
		if(res < 0) {
			s_err("snd_mixer_close failure!");
			return -1;
		}
	}
	free(ptr);
	return 0;
}
int mixer_ctrl_destroy(mixer_ctrl_t *ptr) {
	pthread_mutex_lock(&mutex);
	_mixer_ctrl_destroy(ptr);
	pthread_mutex_unlock(&mutex);
	return 0;
}
static void _mixer_ctrl_sync(mixer_ctrl_t *ptr) {
	long int alsa_left = 0, alsa_right = 0;
	long int volMin = ptr->mPar->volMin;
	snd_mixer_selem_get_playback_volume(ptr->mVolElem, SND_MIXER_SCHN_FRONT_LEFT, &alsa_left);
	snd_mixer_selem_get_playback_volume(ptr->mVolElem, SND_MIXER_SCHN_FRONT_RIGHT, &alsa_right);
	if(alsa_left != volMin && alsa_right != volMin) {
		ptr->mChannel = MIXER_CHANNEL_ALL;
		ptr->mVolume = (alsa_left + alsa_right) >> 1;
		return;
	}
	if(alsa_left != volMin) {
		ptr->mChannel = MIXER_CHANNEL_LEFT;
		ptr->mVolume = alsa_left;
		return ;
	}
	ptr->mChannel = MIXER_CHANNEL_RIGHT;
	ptr->mVolume = alsa_right;
	return ;
}

mixer_ctrl_t *mixer_ctrl_create(mixer_ctrl_par_t *par) {
	mixer_ctrl_t *ptr = (mixer_ctrl_t *)calloc(1, sizeof(mixer_ctrl_t));
	if(!ptr) {
		s_err("calloc mixer_ctrl_t failure");
		return NULL;
	}
	ptr->mPar = par;
	ptr->mMtx = &mutex;
	int res = snd_mixer_open(&ptr->mHdl, 0);
	if(res) {
		s_err("snd_mixer_open failure,res=%d", res);
		_mixer_ctrl_destroy(ptr);
		return NULL;
	}
	res = snd_mixer_attach(ptr->mHdl, par->device);
	if(res) {
		s_err("snd_mixer_attach failure,res=%d", res);
		_mixer_ctrl_destroy(ptr);
		return NULL;
	}
	res = snd_mixer_selem_register(ptr->mHdl, NULL, NULL);
	if(res) {
		s_err("snd_mixer_selem_register failure,res=%d", res);
		_mixer_ctrl_destroy(ptr);
		return NULL;
	}
	res = snd_mixer_load(ptr->mHdl);
	if(res) {
		s_err("snd_mixer_load failure,res=%d", res);
		_mixer_ctrl_destroy(ptr);
		return NULL;
	}
	snd_mixer_selem_id_alloca(&ptr->mSelemId);
	snd_mixer_selem_id_set_index(ptr->mSelemId, par->idx);
	snd_mixer_selem_id_set_name(ptr->mSelemId, par->name);
	ptr->mVolElem = snd_mixer_find_selem(ptr->mHdl, ptr->mSelemId);
	if(!ptr->mVolElem) {
		s_err("snd_mixer_find_selem failure");
		_mixer_ctrl_destroy(ptr);
		return NULL;
	}
	snd_mixer_selem_set_playback_volume_range(ptr->mVolElem, par->volMin, par->volMax);
	_mixer_ctrl_sync(ptr);
	return ptr;
}

long mixer_ctrl_getVol(mixer_ctrl_t *ptr) {
	int res = 0;
	pthread_mutex_lock(ptr->mMtx);
	_mixer_ctrl_sync(ptr);
	res  = ptr->mVolume;
	pthread_mutex_unlock(ptr->mMtx);
	return res;
}
ssize_t mixer_ctrl_getChannel(mixer_ctrl_t *ptr) {
	size_t res = 0;
	pthread_mutex_lock(ptr->mMtx);
	_mixer_ctrl_sync(ptr);
	res  = ptr->mChannel;
	pthread_mutex_unlock(ptr->mMtx);
	return res;
}
static int _mixer_ctrl_setVol(mixer_ctrl_t *ptr, long int vol) {
	int res  = 0;
	_mixer_ctrl_sync(ptr);
	if(ptr->mVolume == vol) {
		return 0;
	}
	ptr->mVolume = vol;
	switch(ptr->mChannel) {
	case MIXER_CHANNEL_ALL:
		res = snd_mixer_selem_set_playback_volume_all(ptr->mVolElem, ptr->mVolume);
		if(res < 0) {
			s_err("snd_mixer_selem_set_playback_volume_all failure!");
			return -1;
		}
		break;
	case MIXER_CHANNEL_LEFT:
		res = snd_mixer_selem_set_playback_volume(ptr->mVolElem, SND_MIXER_SCHN_FRONT_LEFT, ptr->mVolume);
		if(res < 0) {
			s_err("snd_mixer_selem_set_playback_volume failure!");
			return -1;
		}
		break;
	case MIXER_CHANNEL_RIGHT:
		res = snd_mixer_selem_set_playback_volume(ptr->mVolElem, SND_MIXER_SCHN_FRONT_RIGHT, ptr->mVolume);
		if(res < 0) {
			s_err("snd_mixer_selem_set_playback_volume failure!");
			return -1;
		}
		break;
	default:
		s_err("channel %d is not exist", ptr->mChannel);
		return -1;
	}
	return 0;
}
int mixer_ctrl_setVol(mixer_ctrl_t *ptr, long int vol) {
	int res = 0;
	pthread_mutex_lock(ptr->mMtx);
	res  = _mixer_ctrl_setVol(ptr, vol);
	pthread_mutex_unlock(ptr->mMtx);
	return res;
}
static int _mixer_ctrl_setChannel(mixer_ctrl_t *ptr, size_t channel) {
	s_err(__func__);
	int res = 0;
	long int volMin = ptr->mPar->volMin;
	ptr->mChannel = channel;
	switch(ptr->mChannel) {
	case MIXER_CHANNEL_ALL:
		res = snd_mixer_selem_set_playback_volume_all(ptr->mVolElem, ptr->mVolume);
		if(res < 0) {
			s_err("snd_mixer_selem_set_playback_volume_all failure!");
			return -1;
		}
		s_dbg("snd_mixer_selem_set_playback_volume_all %d", ptr->mVolume);
		break;
	case MIXER_CHANNEL_LEFT:
		res = snd_mixer_selem_set_playback_volume(ptr->mVolElem, SND_MIXER_SCHN_FRONT_RIGHT, volMin);
		if(res < 0) {
			s_err("snd_mixer_selem_set_playback_volume dis right failure!");
			return -1;
		}
		s_inf("SND_MIXER_SCHN_FRONT_RIGHT dis");
		res = snd_mixer_selem_set_playback_volume(ptr->mVolElem, SND_MIXER_SCHN_FRONT_LEFT, ptr->mVolume);
		if(res < 0) {
			s_err("snd_mixer_selem_set_playback_volume failure!");
			return -1;
		}
		s_dbg("SND_MIXER_SCHN_FRONT_LEFT %d", ptr->mVolume);
		break;
	case MIXER_CHANNEL_RIGHT:
		res = snd_mixer_selem_set_playback_volume(ptr->mVolElem, SND_MIXER_SCHN_FRONT_LEFT, volMin);
		if(res < 0) {
			s_err("snd_mixer_selem_set_playback_volume dis left failure!");
			return -1;
		}
		s_dbg("SND_MIXER_SCHN_FRONT_LEFT dis");
		res = snd_mixer_selem_set_playback_volume(ptr->mVolElem, SND_MIXER_SCHN_FRONT_RIGHT, ptr->mVolume);
		if(res < 0) {
			s_err("snd_mixer_selem_set_playback_volume failure!");
			return -1;
		}
		s_inf("SND_MIXER_SCHN_FRONT_RIGHT %d", ptr->mVolume);
		break;
	default:
		s_err("channel %d is not exist", ptr->mChannel);
		return -1;
	}
	return 0;
}

int mixer_ctrl_setChannel(mixer_ctrl_t *ptr, size_t channel) {
	int res = 0;
	pthread_mutex_lock(ptr->mMtx);
	res  = _mixer_ctrl_setChannel(ptr, channel);
	pthread_mutex_unlock(ptr->mMtx);
	return res;
}

