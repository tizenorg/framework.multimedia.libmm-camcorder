/*
 * libmm-camcorder
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Jeongmo Yang <jm80.yang@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/*=======================================================================================
|  INCLUDE FILES									|
=======================================================================================*/
#include <mm_sound.h>
#include <mm_sound_private.h>
#include <audio-session-manager.h>
#include "mm_camcorder_internal.h"
#include "mm_camcorder_sound.h"

/*---------------------------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/
#define SAMPLE_SOUND_NAME       "camera-shutter"
#define SAMPLE_SOUND_VOLUME     65535
#define SAMPLE_SOUND_RATE       44100
#define DEFAULT_ACTIVE_DEVICE   -1

/*---------------------------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:								|
---------------------------------------------------------------------------------------*/
static void __solo_sound_callback(void *data);


static void __pulseaudio_context_state_cb(pa_context *pulse_context, void *user_data)
{
	int state = 0;
	SOUND_INFO *info = NULL;

	mmf_return_if_fail(user_data);

	info = (SOUND_INFO *)user_data;

	state = pa_context_get_state(pulse_context);
	switch (state) {
	case PA_CONTEXT_READY:
		_mmcam_dbg_log("pulseaudio context READY");
		if (info->pulse_context == pulse_context) {
			/* Signal */
			_mmcam_dbg_log("pulseaudio send signal");
			pa_threaded_mainloop_signal(info->pulse_mainloop, 0);
		}
		break;
	case PA_CONTEXT_TERMINATED:
		if (info->pulse_context == pulse_context) {
			/* Signal */
			_mmcam_dbg_log("Context terminated : pulseaudio send signal");
			pa_threaded_mainloop_signal(info->pulse_mainloop, 0);
		}
		break;
	case PA_CONTEXT_UNCONNECTED:
	case PA_CONTEXT_CONNECTING:
	case PA_CONTEXT_AUTHORIZING:
	case PA_CONTEXT_SETTING_NAME:
	case PA_CONTEXT_FAILED:
	default:
		_mmcam_dbg_log("pulseaudio context %p, state %d",
		               pulse_context, state);
		break;
	}

	return;
}

#ifdef _MMCAMCORDER_UPLOAD_SAMPLE
static void __pulseaudio_stream_write_cb(pa_stream *stream, size_t length, void *user_data)
{
	sf_count_t read_length;
	short *data;
	SOUND_INFO *info = NULL;

	mmf_return_if_fail(user_data);

	info = (SOUND_INFO *)user_data;

	_mmcam_dbg_log("START");

	data = pa_xmalloc(length);

	read_length = (sf_count_t)(length/pa_frame_size(&(info->sample_spec)));

	if ((sf_readf_short(info->infile, data, read_length)) != read_length) {
		pa_xfree(data);
		return;
	}

	pa_stream_write(stream, data, length, pa_xfree, 0, PA_SEEK_RELATIVE);

	info->sample_length -= length;

	if (info->sample_length <= 0) {
		pa_stream_set_write_callback(info->sample_stream, NULL, NULL);
		pa_stream_finish_upload(info->sample_stream);

		pa_threaded_mainloop_signal(info->pulse_mainloop, 0);
		_mmcam_dbg_log("send signal DONE");
	}

	_mmcam_dbg_log("DONE read_length %d", read_length);
}


static void __pulseaudio_remove_sample_finish_cb(pa_context *pulse_context, int success, void *user_data)
{
	SOUND_INFO *info = NULL;

	mmf_return_if_fail(user_data);

	info = (SOUND_INFO *)user_data;

	_mmcam_dbg_log("START");

	pa_threaded_mainloop_signal(info->pulse_mainloop, 0);

	_mmcam_dbg_log("DONE");
}
#endif /* _MMCAMCORDER_UPLOAD_SAMPLE */

#ifdef _MMCAMCORDER_UPLOAD_SAMPLE
gboolean _mmcamcorder_sound_init(MMHandleType handle, char *filename)
#else /* _MMCAMCORDER_UPLOAD_SAMPLE */
gboolean _mmcamcorder_sound_init(MMHandleType handle)
#endif /* _MMCAMCORDER_UPLOAD_SAMPLE */
{
	int ret = 0;
	int sound_enable = TRUE;
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	SOUND_INFO *info = NULL;
	mm_sound_device_in device_in;
	mm_sound_device_out device_out;
	pa_mainloop_api *api = NULL;

	mmf_return_val_if_fail(hcamcorder, FALSE);

	/* check sound play enable */
	ret = mm_camcorder_get_attributes((MMHandleType)hcamcorder, NULL,
	                                  "capture-sound-enable", &sound_enable,
	                                  NULL);
	if (ret == MM_ERROR_NONE) {
		_mmcam_dbg_log("Capture sound enable %d", sound_enable);
		if (sound_enable == FALSE) {
			return TRUE;
		}
	} else {
		_mmcam_dbg_warn("capture-sound-enable get FAILED.[%x]", ret);
	}

	info = &(hcamcorder->snd_info);

	pthread_mutex_lock(&(info->open_mutex));

	if (info->state > _MMCAMCORDER_SOUND_STATE_NONE) {
		_mmcam_dbg_warn("already initialized [%d]", info->state);
		pthread_mutex_unlock(&(info->open_mutex));
		return FALSE;
	}

#ifdef _MMCAMCORDER_UPLOAD_SAMPLE
	if (info->filename) {
		free(info->filename);
		info->filename = NULL;
	}

	info->filename = strdup(filename);
	if (info->filename == NULL) {
		_mmcam_dbg_err("strdup failed");
		return FALSE;
	}
#endif /* _MMCAMCORDER_UPLOAD_SAMPLE */

	pthread_mutex_init(&(info->play_mutex), NULL);
	pthread_cond_init(&(info->play_cond), NULL);

#ifdef _MMCAMCORDER_UPLOAD_SAMPLE
	/* read sample */
	memset (&(info->sfinfo), 0, sizeof(SF_INFO));
	info->infile = sf_open(info->filename, SFM_READ, &(info->sfinfo));
	if (!(info->infile)) {
		_mmcam_dbg_err("Failed to open sound file");
		goto SOUND_INIT_ERROR;
	}

	/* open PCM handle and set session */
	__ta__("        mm_sound_pcm_play_open",
	ret = mm_sound_pcm_play_open_ex(&(info->handle), info->sfinfo.samplerate,
	                                (info->sfinfo.channels == 1) ? MMSOUND_PCM_MONO : MMSOUND_PCM_STEREO,
	                                MMSOUND_PCM_S16_LE, VOLUME_TYPE_FIXED, ASM_EVENT_EXCLUSIVE_MMSOUND);
	);
	if (ret < 0) {
		/* error */
		_mmcam_dbg_err("mm_sound_pcm_play_open failed [%x]", ret);
		goto SOUND_INIT_ERROR;
	}
#else /* _MMCAMCORDER_UPLOAD_SAMPLE */
	/* open PCM handle and set session */
	__ta__("        mm_sound_pcm_play_open",
	ret = mm_sound_pcm_play_open_ex(&(info->handle), SAMPLE_SOUND_RATE,
	                                MMSOUND_PCM_STEREO, MMSOUND_PCM_S16_LE,
	                                VOLUME_TYPE_FIXED, ASM_EVENT_EXCLUSIVE_MMSOUND);
	);
	if (ret < 0) {
		/* error */
		_mmcam_dbg_err("mm_sound_pcm_play_open failed [%x]", ret);
		goto SOUND_INIT_ERROR;
	}
#endif /* _MMCAMCORDER_UPLOAD_SAMPLE */

	_mmcam_dbg_log("mm_sound_pcm_play_open done");

	/**
	 * Init Pulseaudio thread
	 */
	/* create pulseaudio mainloop */
	info->pulse_mainloop = pa_threaded_mainloop_new();
	ret = pa_threaded_mainloop_start(info->pulse_mainloop);

	/* lock pulseaudio thread */
	pa_threaded_mainloop_lock(info->pulse_mainloop);
	/* get pulseaudio api */
	api = pa_threaded_mainloop_get_api(info->pulse_mainloop);
	/* create pulseaudio context */
	info->pulse_context = pa_context_new(api, NULL);
	/* set pulseaudio context callback */
	pa_context_set_state_callback(info->pulse_context, __pulseaudio_context_state_cb, info);

	if (pa_context_connect(info->pulse_context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
		_mmcam_dbg_err("pa_context_connect error");
	}

	/* wait READY state of pulse context */
	while (TRUE) {
		pa_context_state_t state = pa_context_get_state(info->pulse_context);

		_mmcam_dbg_log("pa context state is now %d", state);

		if (!PA_CONTEXT_IS_GOOD (state)) {
			_mmcam_dbg_log("connection failed");
			break;
		}

		if (state == PA_CONTEXT_READY) {
			_mmcam_dbg_log("pa context READY");
			break;
		}

		/* Wait until the context is ready */
		_mmcam_dbg_log("waiting..................");
		pa_threaded_mainloop_wait(info->pulse_mainloop);
		_mmcam_dbg_log("waiting DONE. check again...");
	}

	/* unlock pulseaudio thread */
	pa_threaded_mainloop_unlock(info->pulse_mainloop);

#ifdef _MMCAMCORDER_UPLOAD_SAMPLE
	/**
	 * Upload sample
	 */
	if (pa_sndfile_read_sample_spec(info->infile, &(info->sample_spec)) < 0) {
		_mmcam_dbg_err("Failed to determine sample specification from file");
		goto SOUND_INIT_ERROR;
	}

	info->sample_spec.format = PA_SAMPLE_S16LE;

	if (pa_sndfile_read_channel_map(info->infile, &(info->channel_map)) < 0) {
		pa_channel_map_init_extend(&(info->channel_map), info->sample_spec.channels, PA_CHANNEL_MAP_DEFAULT);

		if (info->sample_spec.channels > 2) {
			_mmcam_dbg_warn("Failed to determine sample specification from file");
		}
	}

	info->sample_length = (size_t)info->sfinfo.frames * pa_frame_size(&(info->sample_spec));

	pa_threaded_mainloop_lock(info->pulse_mainloop);

	/* prepare uploading */
	info->sample_stream = pa_stream_new(info->pulse_context, SAMPLE_SOUND_NAME, &(info->sample_spec), NULL);
	/* set stream write callback */
	pa_stream_set_write_callback(info->sample_stream, __pulseaudio_stream_write_cb, info);
	/* upload sample (ASYNC) */
	pa_stream_connect_upload(info->sample_stream, info->sample_length);
	/* wait for upload completion */
	pa_threaded_mainloop_wait(info->pulse_mainloop);

	pa_threaded_mainloop_unlock (info->pulse_mainloop);
#endif /* _MMCAMCORDER_UPLOAD_SAMPLE */

	/* backup current route */
	info->active_out_backup = DEFAULT_ACTIVE_DEVICE;

	ret = mm_sound_get_active_device(&device_in, &device_out);
	if (ret != MM_ERROR_NONE) {
		_mmcam_dbg_err("mm_sound_get_active_device failed [%x]. skip sound play.", ret);
		goto SOUND_INIT_ERROR;
	}

	_mmcam_dbg_log("current out [%x]", device_out);

	if (device_out != MM_SOUND_DEVICE_OUT_SPEAKER) {
		ret = mm_sound_set_active_route (MM_SOUND_ROUTE_OUT_SPEAKER);
		if (ret != MM_ERROR_NONE) {
			_mmcam_dbg_err("mm_sound_set_active_route failed [%x]. skip sound play.", ret);
			goto SOUND_INIT_ERROR;
		}
		info->active_out_backup = device_out;
	}

	info->state = _MMCAMCORDER_SOUND_STATE_INIT;

	_mmcam_dbg_log("init DONE");

	pthread_mutex_unlock(&(info->open_mutex));

	return TRUE;

SOUND_INIT_ERROR:

#ifdef _MMCAMCORDER_UPLOAD_SAMPLE
	/**
	 * Release allocated resources
	 */
	if (info->filename) {
		free(info->filename);
		info->filename = NULL;
	}
#endif /* _MMCAMCORDER_UPLOAD_SAMPLE */

	/* remove pulse mainloop */
	if (info->pulse_mainloop) {
		/* remove pulse context */
		if (info->pulse_context) {
#ifdef _MMCAMCORDER_UPLOAD_SAMPLE
			/* remove uploaded sample */
			if (info->sample_stream) {
				pa_threaded_mainloop_lock(info->pulse_mainloop);

				/* Remove sample (ASYNC) */
				pa_operation_unref(pa_context_remove_sample(info->pulse_context, SAMPLE_SOUND_NAME, __pulseaudio_remove_sample_finish_cb, info));

				/* Wait for async operation */
				pa_threaded_mainloop_wait(info->pulse_mainloop);
			}
#endif /* _MMCAMCORDER_UPLOAD_SAMPLE */

			/* Make sure we don't get any further callbacks */
			pa_context_set_state_callback(info->pulse_context, NULL, NULL);

			pa_context_disconnect(info->pulse_context);
			pa_context_unref(info->pulse_context);
			info->pulse_context = NULL;
		}

		pa_threaded_mainloop_stop(info->pulse_mainloop);
		pa_threaded_mainloop_free(info->pulse_mainloop);
		info->pulse_mainloop = NULL;
	}

	/* remove mutex and cond */
	pthread_mutex_destroy(&(info->play_mutex));
	pthread_cond_destroy(&(info->play_cond));

	pthread_mutex_unlock(&(info->open_mutex));

	return FALSE;
}


gboolean _mmcamcorder_sound_play(MMHandleType handle)
{
	int ret = 0;
	int sound_enable = TRUE;

	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	SOUND_INFO *info = NULL;
	pa_operation *pulse_op = NULL;

	mmf_return_val_if_fail(hcamcorder, FALSE);

	/* check sound play enable */
	ret = mm_camcorder_get_attributes((MMHandleType)hcamcorder, NULL,
	                                  "capture-sound-enable", &sound_enable,
	                                  NULL);
	if (ret == MM_ERROR_NONE) {
		_mmcam_dbg_log("Capture sound enable %d", sound_enable);
		if (sound_enable == FALSE) {
			return TRUE;
		}
	} else {
		_mmcam_dbg_warn("capture-sound-enable get FAILED.[%x]", ret);
	}

	info = &(hcamcorder->snd_info);

	pthread_mutex_lock(&(info->open_mutex));

	if (info->state < _MMCAMCORDER_SOUND_STATE_INIT) {
		_mmcam_dbg_log("not initialized state:[%d]", info->state);
		pthread_mutex_unlock(&(info->open_mutex));
		return FALSE;
	}

	_mmcam_dbg_log("Play start");

	__ta__("                    pa_context_play_sample",
	pulse_op = pa_context_play_sample(info->pulse_context,
	                                  SAMPLE_SOUND_NAME,
	                                  NULL,
	                                  SAMPLE_SOUND_VOLUME,
	                                  NULL,
	                                  NULL);
	);
	if (pulse_op) {
		pa_operation_unref(pulse_op);
		pulse_op = NULL;
	}

	pthread_mutex_unlock(&(info->open_mutex));

	_mmcam_dbg_log("Done");

	return TRUE;
}


gboolean _mmcamcorder_sound_finalize(MMHandleType handle)
{
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	SOUND_INFO *info = NULL;
	mm_sound_device_in device_in;
	mm_sound_device_out device_out;
	int ret = 0;

	mmf_return_val_if_fail(hcamcorder, FALSE);

	info = &(hcamcorder->snd_info);

	_mmcam_dbg_err("START");

	pthread_mutex_lock(&(info->open_mutex));

	if (info->state < _MMCAMCORDER_SOUND_STATE_INIT) {
		_mmcam_dbg_warn("not initialized");
		pthread_mutex_unlock(&(info->open_mutex));
		return TRUE;
	}

	/**
	 * Restore route
	 */
	_mmcam_dbg_log("restore route");
	if (info->active_out_backup != DEFAULT_ACTIVE_DEVICE) {
		ret = mm_sound_get_active_device(&device_in, &device_out);
		if (ret != MM_ERROR_NONE) {
			_mmcam_dbg_err("mm_sound_get_active_device failed [%x]. skip sound play.", ret);
		}

		_mmcam_dbg_log("current out [%x]", device_out);

		if (device_out != info->active_out_backup) {
			ret = mm_sound_set_active_route (info->active_out_backup);
			if (ret != MM_ERROR_NONE) {
				_mmcam_dbg_err("mm_sound_set_active_route failed [%x]. skip sound play.", ret);
			}
		}
	}

#ifdef _MMCAMCORDER_UPLOAD_SAMPLE
	/**
	 * Remove sample
	 */
	_mmcam_dbg_log("remove sample");

	pa_threaded_mainloop_lock(info->pulse_mainloop);

	/* Remove sample (ASYNC) */
	pa_operation_unref(pa_context_remove_sample(info->pulse_context, SAMPLE_SOUND_NAME, __pulseaudio_remove_sample_finish_cb, info));

	/* Wait for async operation */
	pa_threaded_mainloop_wait(info->pulse_mainloop);

	pa_threaded_mainloop_unlock(info->pulse_mainloop);
#endif /* _MMCAMCORDER_UPLOAD_SAMPLE */

	/**
	 * Release pulseaudio thread
	 */
	_mmcam_dbg_log("release pulseaudio thread");

	pa_threaded_mainloop_lock(info->pulse_mainloop);

	pa_context_disconnect(info->pulse_context);

	/* Make sure we don't get any further callbacks */
	pa_context_set_state_callback(info->pulse_context, NULL, NULL);

	pa_context_unref(info->pulse_context);
	info->pulse_context = NULL;

	pa_threaded_mainloop_unlock(info->pulse_mainloop);

	pa_threaded_mainloop_stop(info->pulse_mainloop);
	pa_threaded_mainloop_free(info->pulse_mainloop);
	info->pulse_mainloop = NULL;

#ifdef _MMCAMCORDER_UPLOAD_SAMPLE
	if (info->filename) {
		free(info->filename);
		info->filename = NULL;
	}
#endif /* _MMCAMCORDER_UPLOAD_SAMPLE */

	info->state = _MMCAMCORDER_SOUND_STATE_NONE;
	info->active_out_backup = DEFAULT_ACTIVE_DEVICE;

	/* release mutex and cond */
	_mmcam_dbg_log("release play_mutex/cond");
	pthread_mutex_destroy(&(info->play_mutex));
	pthread_cond_destroy(&(info->play_cond));

	/* close PCM */
	mm_sound_pcm_play_close(info->handle);
	info->handle = 0;

	pthread_mutex_unlock(&(info->open_mutex));

	_mmcam_dbg_err("DONE");

	return TRUE;
}


gboolean _mmcamcorder_sound_capture_play_cb(gpointer data)
{
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(data);

	mmf_return_val_if_fail(hcamcorder, FALSE);

	_mmcam_dbg_log("Capture sound PLAY in idle callback");

	_mmcamcorder_sound_solo_play((MMHandleType)hcamcorder, _MMCAMCORDER_FILEPATH_CAPTURE_SND, FALSE);

	return FALSE;
}


void _mmcamcorder_sound_solo_play(MMHandleType handle, const char* filepath, gboolean sync)
{
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);

	int sound_handle = 0;
	int ret = 0;
	int sound_enable = TRUE;

	mmf_return_if_fail(filepath && hcamcorder);

	_mmcam_dbg_log( "START" );

	ret = mm_camcorder_get_attributes((MMHandleType)hcamcorder, NULL,
	                                  "capture-sound-enable", &sound_enable,
	                                  NULL);
	if (ret == MM_ERROR_NONE) {
		_mmcam_dbg_log("Capture sound enable %d", sound_enable);
		if (sound_enable == FALSE) {
			return;
		}
	} else {
		_mmcam_dbg_warn("capture-sound-enable get FAILED.[%x]", ret);
	}

	ret = pthread_mutex_trylock(&(hcamcorder->sound_lock));
	if (ret != 0) {
		_mmcam_dbg_warn("g_mutex_trylock failed.[%s]", strerror(ret));
		return;
	}

	__ta__("CAPTURE SOUND:mm_sound_play_loud_solo_sound",
	ret = mm_sound_play_loud_solo_sound(filepath, VOLUME_TYPE_FIXED, __solo_sound_callback,
	                                    (void*)hcamcorder, &sound_handle);
	);
	if (ret != MM_ERROR_NONE) {
		_mmcam_dbg_err( "Capture sound play FAILED.[%x]", ret );
	} else {
		if (sync) {
			struct timespec timeout;
			struct timeval tv;

			gettimeofday( &tv, NULL );
			timeout.tv_sec = tv.tv_sec + 2;
			timeout.tv_nsec = tv.tv_usec * 1000;

			_mmcam_dbg_log("Wait for signal");

			MMTA_ACUM_ITEM_BEGIN("CAPTURE SOUND:wait sound play finish", FALSE);

			if (!pthread_cond_timedwait(&(hcamcorder->sound_cond), &(hcamcorder->sound_lock), &timeout)) {
				_mmcam_dbg_log("signal received.");
			} else {
				_mmcam_dbg_warn("capture sound play timeout.");
				if (sound_handle > 0) {
					mm_sound_stop_sound(sound_handle);
				}
			}

			MMTA_ACUM_ITEM_END("CAPTURE SOUND:wait sound play finish", FALSE);
		}
	}

	pthread_mutex_unlock(&(hcamcorder->sound_lock));

	_mmcam_dbg_log("DONE");

	return;
}

static void __solo_sound_callback(void *data)
{
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(data);

	mmf_return_if_fail(hcamcorder);

	_mmcam_dbg_log("START");

	_mmcam_dbg_log("Signal SEND");
	pthread_cond_broadcast(&(hcamcorder->sound_cond));

	_mmcam_dbg_log("DONE");

	return;
}

