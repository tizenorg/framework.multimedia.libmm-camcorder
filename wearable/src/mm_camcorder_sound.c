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
#include <avsystem.h>
#include <audio-session-manager.h>
#include "mm_camcorder_internal.h"
#include "mm_camcorder_sound.h"

/*---------------------------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:								|
---------------------------------------------------------------------------------------*/
static void __solo_sound_callback(void *data, int id);


void _mmcamcorder_sound_solo_play(MMHandleType handle, const char* filepath, gboolean sync)
{
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);

	int sound_handle = 0;
	int ret = MM_ERROR_NONE;
	int sound_enable = TRUE;
	int gain_type = VOLUME_GAIN_SHUTTER1;

	mmf_return_if_fail(filepath && hcamcorder);

	_mmcam_dbg_log("START : %s", filepath);

	ret = mm_camcorder_get_attributes((MMHandleType)hcamcorder, NULL,
	                                  "capture-sound-enable", &sound_enable,
	                                  NULL);

	_mmcam_dbg_log("Capture sound enable %d", sound_enable);
	if (!sound_enable) {
		return;
	}

	ret = pthread_mutex_trylock(&(hcamcorder->sound_lock));
	if (ret != 0) {
		_mmcam_dbg_warn("g_mutex_trylock failed.[%s]", strerror(ret));
		return;
	}

	/* check filename to set gain_type */
	if (!strcmp(filepath, _MMCAMCORDER_FILEPATH_CAPTURE_SND) ||
	    !strcmp(filepath, _MMCAMCORDER_FILEPATH_CAPTURE_EXT_01_SND) ||
	    !strcmp(filepath, _MMCAMCORDER_FILEPATH_CAPTURE_EXT_02_SND)) {
		_MMCamcorderMsgItem msg;

		gain_type = VOLUME_GAIN_SHUTTER1;
		hcamcorder->send_capture_sound_msg = TRUE;

		/* send message if single capture sound */
		msg.id = MM_MESSAGE_CAMCORDER_CAPTURE_SOUND;
		msg.param.code = TRUE;
		__ta__("CAPTURE SOUND:MM_MESSAGE_CAMCORDER_CAPUTRE_SOUND",
		_mmcamcroder_send_message((MMHandleType)hcamcorder, &msg);
		);
	} else if (!strcmp(filepath, _MMCAMCORDER_FILEPATH_CAPTURE2_SND)) {
		gain_type = VOLUME_GAIN_SHUTTER2;
	} else if (!strcmp(filepath, _MMCAMCORDER_FILEPATH_REC_START_SND) ||
	           !strcmp(filepath, _MMCAMCORDER_FILEPATH_REC_STOP_SND)) {
		gain_type = VOLUME_GAIN_CAMCORDING;
	}

	_mmcam_dbg_log("gain type 0x%x", gain_type);

	if (hcamcorder->shutter_sound_policy == VCONFKEY_CAMERA_SHUTTER_SOUND_POLICY_ON) {
		__ta__("CAPTURE SOUND:mm_sound_play_loud_solo_sound",
		ret = mm_sound_play_loud_solo_sound(filepath, VOLUME_TYPE_FIXED | gain_type,
		                                    __solo_sound_callback, (void*)hcamcorder, &sound_handle);
		);
	} else {
		gboolean sound_status = hcamcorder->sub_context->info_image->sound_status;
		mm_sound_device_in device_in;
		mm_sound_device_out device_out;

		/* get sound path */
		mm_sound_get_active_device(&device_in, &device_out);

		_mmcam_dbg_log("sound status %d, device out %x", sound_status, device_out);

		if (sound_status || device_out != MM_SOUND_DEVICE_OUT_SPEAKER) {
			__ta__("CAPTURE SOUND:mm_sound_play_sound",
			ret = mm_sound_play_solo_sound(filepath, VOLUME_TYPE_SYSTEM | gain_type,
			                               __solo_sound_callback, (void*)hcamcorder, &sound_handle);
			);
		}
	}
	if (ret != MM_ERROR_NONE) {
		_mmcam_dbg_err( "Capture sound play FAILED.[%x]", ret );
		/* reset flag */
		hcamcorder->send_capture_sound_msg = FALSE;
	} else {
		/* increase capture sound count */
		hcamcorder->capture_sound_count++;

		/* wait for sound completed signal */
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

static void __solo_sound_callback(void *data, int id)
{
	_MMCamcorderMsgItem msg;
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(data);

	mmf_return_if_fail(hcamcorder);

	_mmcam_dbg_log("START");

	/* decrease capture sound count */
	pthread_mutex_lock(&(hcamcorder->sound_lock));
	if (hcamcorder->capture_sound_count > 0) {
		hcamcorder->capture_sound_count--;
	} else {
		_mmcam_dbg_warn("invalid capture_sound_count %d, reset count", hcamcorder->capture_sound_count);
		hcamcorder->capture_sound_count = 0;
	}
	pthread_mutex_unlock(&(hcamcorder->sound_lock));

	/* send capture sound completed message */
	if (hcamcorder->send_capture_sound_msg) {
		_mmcam_dbg_log("send MM_MESSAGE_CAMCORDER_CAPTURE_SOUND_COMPLETED");

		msg.id = MM_MESSAGE_CAMCORDER_CAPTURE_SOUND_COMPLETED;
		msg.param.code = TRUE;
		__ta__("CAPTURE SOUND:MM_MESSAGE_CAMCORDER_CAPTURE_SOUND_COMPLETED",
		_mmcamcroder_send_message((MMHandleType)hcamcorder, &msg);
		);

		hcamcorder->send_capture_sound_msg = FALSE;
	}

	_mmcam_dbg_log("Signal SEND");
	pthread_cond_broadcast(&(hcamcorder->sound_cond));

	_mmcam_dbg_log("DONE");

	return;
}

