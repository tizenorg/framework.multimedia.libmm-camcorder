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
#include <gst/interfaces/xoverlay.h>
#include <gst/interfaces/cameracontrol.h>
#include <sys/time.h>
#include <unistd.h>

#include "mm_camcorder_internal.h"
#include "mm_camcorder_gstcommon.h"

/*-----------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/
/* Table for compatibility between audio codec and file format */
gboolean	audiocodec_fileformat_compatibility_table[MM_AUDIO_CODEC_NUM][MM_FILE_FORMAT_NUM] =
{          /* 3GP ASF AVI MATROSKA MP4 OGG NUT QT REAL AMR AAC MP3 AIFF AU WAV MID MMF DIVX FLV VOB IMELODY WMA WMV JPG */
/*AMR*/       { 1,  0,  0,       0,  0,  0,  0, 0,   0,  1,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*G723.1*/    { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MP3*/       { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*OGG*/       { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*AAC*/       { 1,  0,  1,       1,  1,  0,  0, 0,   0,  0,  1,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*WMA*/       { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MMF*/       { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*ADPCM*/     { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*WAVE*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  1,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*WAVE_NEW*/  { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MIDI*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*IMELODY*/   { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MXMF*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MPA*/       { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MP2*/       { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*G711*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*G722*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*G722.1*/    { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*G722.2*/    { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*G723*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*G726*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*G728*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*G729*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*G729A*/     { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*G729.1*/    { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*REAL*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*AAC_LC*/    { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*AAC_MAIN*/  { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*AAC_SRS*/   { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*AAC_LTP*/   { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*AAC_HE_V1*/ { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*AAC_HE_V2*/ { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*AC3*/       { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*ALAC*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*ATRAC*/     { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*SPEEX*/     { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*VORBIS*/    { 0,  0,  0,       0,  0,  1,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*AIFF*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*AU*/        { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*NONE*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*PCM*/       { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*ALAW*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MULAW*/     { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MS_ADPCM*/  { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
};

/* Table for compatibility between video codec and file format */
gboolean	videocodec_fileformat_compatibility_table[MM_VIDEO_CODEC_NUM][MM_FILE_FORMAT_NUM] =
{             /* 3GP ASF AVI MATROSKA MP4 OGG NUT QT REAL AMR AAC MP3 AIFF AU WAV MID MMF DIVX FLV VOB IMELODY WMA WMV JPG */
/*NONE*/         { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*H263*/         { 1,  0,  1,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*H264*/         { 1,  0,  1,       1,  1,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*H26L*/         { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MPEG4*/        { 1,  0,  1,       1,  1,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MPEG1*/        { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*WMV*/          { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*DIVX*/         { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*XVID*/         { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*H261*/         { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*H262*/         { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*H263V2*/       { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*H263V3*/       { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MJPEG*/        { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MPEG2*/        { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MPEG4_SIMPLE*/ { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MPEG4_ADV*/    { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MPEG4_MAIN*/   { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MPEG4_CORE*/   { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MPEG4_ACE*/    { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MPEG4_ARTS*/   { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*MPEG4_AVC*/    { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*REAL*/         { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*VC1*/          { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*AVS*/          { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*CINEPAK*/      { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*INDEO*/        { 0,  0,  0,       0,  0,  0,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
/*THEORA*/       { 0,  0,  0,       0,  0,  1,  0, 0,   0,  0,  0,  0,   0, 0,  0,  0,  0,   0,  0,  0,      0,  0,  0,  0,  },
};


/*-----------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/
#define USE_AUDIO_CLOCK_TUNE
#define _MMCAMCORDER_WAIT_EOS_TIME                10.0     /* sec */
#define _MMCAMCORDER_CONVERT_OUTPUT_BUFFER_NUM    6
#define _MMCAMCORDER_MIN_TIME_TO_PASS_FRAME 30000000 /* ns */
#define _MMCAMCORDER_FRAME_PASS_MIN_FPS 30
#define _MMCAMCORDER_NANOSEC_PER_1SEC             1000000000
#define _MMCAMCORDER_NANOSEC_PER_1MILISEC         1000
#define _MMCAMCORDER_DUAL_CAMERA_BUFFER_WAIT_TIME 40000000 /* ns */

/*-----------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:						|
-----------------------------------------------------------------------*/
/* STATIC INTERNAL FUNCTION */
/**
 * These functions are preview video data probing function.
 * If this function is linked with certain pad by gst_pad_add_buffer_probe(),
 * this function will be called when data stream pass through the pad.
 *
 * @param[in]	pad		probing pad which calls this function.
 * @param[in]	buffer		buffer which contains stream data.
 * @param[in]	u_data		user data.
 * @return	This function returns true on success, or false value with error
 * @remarks
 * @see		__mmcamcorder_create_preview_pipeline()
 */
static gboolean __mmcamcorder_video_dataprobe_preview(GstPad *pad, GstBuffer *buffer, gpointer u_data);
static gboolean __mmcamcorder_video_dataprobe_push_buffer_to_record(GstPad *pad, GstBuffer *buffer, gpointer u_data);
static gboolean __mmcamcorder_dual_camera_buffer_check(mmf_camcorder_t *hcamcorder, GstBuffer *buffer, int index);
static void __mmcamcorder_videoframe_render_error_cb(GstElement *element, void *error_id, gpointer u_data);

static int __mmcamcorder_get_amrnb_bitrate_mode(int bitrate);

/*=======================================================================================
|  FUNCTION DEFINITIONS									|
=======================================================================================*/
/*-----------------------------------------------------------------------
|    GLOBAL FUNCTION DEFINITIONS:					|
-----------------------------------------------------------------------*/
int _mmcamcorder_create_preview_elements(MMHandleType handle)
{
	int err = MM_ERROR_NONE;
	int i = 0;
	int camera_width = 0;
	int camera_height = 0;
	int camera_rotate = 0;
	int camera_flip = 0;
	int fps = 0;
	int hold_af = 0;
	int codectype = 0;
	int capture_width = 0;
	int capture_height = 0;
	int capture_jpg_quality = 100;
	int video_stabilization = 0;
	int anti_shake = 0;
	int rect_width = 0;
	int rect_height = 0;
	int display_rotate = 0;
	int display_flip = 0;
	int EnableConvertedSC = 0;
	int inset_win_x = 0;
	int inset_win_y = 0;
	int inset_win_width = 0;
	int inset_win_height = 0;
	int inset_win_dev = 0;
	int inset_win_style = 0;
	int inset_win_orientation = 0;
	bool hide_inset_window = FALSE;
	int color_tone = MM_CAMCORDER_COLOR_TONE_NONE;
	char *videosrc_name = NULL;
	char *videosink_name = NULL;
	char *videoconvert_name = NULL;
	char *err_name = NULL;

	GList *element_list = NULL;

	GstPad *srcpad = NULL;
	GstPad *sinkpad = NULL;

	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;
	type_element *VideosrcElement = NULL;
	type_int_array *input_index = NULL;

	mmf_return_val_if_fail(hcamcorder, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_CAMCORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->element, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	_mmcam_dbg_log("");

	/* Check existence */
	for (i = _MMCAMCORDER_VIDEOSRC_SRC ; i <= _MMCAMCORDER_VIDEOSINK_SINK ; i++) {
		if (sc->element[i].gst) {
			if (((GObject *)sc->element[i].gst)->ref_count > 0) {
				gst_object_unref(sc->element[i].gst);
			}
			_mmcam_dbg_log("element[index:%d] is Already existed.", i);
		}
	}

	/* Get video device index info */
	_mmcamcorder_conf_get_value_int_array(hcamcorder->conf_ctrl,
	                                      CONFIGURE_CATEGORY_CTRL_CAMERA,
	                                      "InputIndex",
	                                      &input_index);
	if (input_index == NULL) {
		_mmcam_dbg_err("Failed to get input_index");
		return MM_ERROR_CAMCORDER_CREATE_CONFIGURE;
	}

	err = mm_camcorder_get_attributes(handle, &err_name,
	                                  MMCAM_CAMERA_WIDTH, &camera_width,
	                                  MMCAM_CAMERA_HEIGHT, &camera_height,
	                                  MMCAM_CAMERA_FORMAT, &sc->info_image->preview_format,
	                                  MMCAM_CAMERA_FPS, &fps,
	                                  MMCAM_CAMERA_ROTATION, &camera_rotate,
	                                  MMCAM_CAMERA_FLIP, &camera_flip,
	                                  MMCAM_CAMERA_VIDEO_STABILIZATION, &video_stabilization,
	                                  MMCAM_CAMERA_ANTI_HANDSHAKE, &anti_shake,
	                                  MMCAM_CAPTURE_WIDTH, &capture_width,
	                                  MMCAM_CAPTURE_HEIGHT, &capture_height,
	                                  MMCAM_CAMERA_HDR_CAPTURE, &sc->info_image->hdr_capture_mode,
	                                  MMCAM_IMAGE_ENCODER, &codectype,
	                                  MMCAM_IMAGE_ENCODER_QUALITY, &capture_jpg_quality,
	                                  "camera-hold-af-after-capturing", &hold_af,
	                                  MMCAM_DISPLAY_RECT_WIDTH, &rect_width,
	                                  MMCAM_DISPLAY_RECT_HEIGHT, &rect_height,
	                                  MMCAM_DISPLAY_ROTATION, &display_rotate,
	                                  MMCAM_DISPLAY_FLIP, &display_flip,
	                                  MMCAM_FILTER_COLOR_TONE, &color_tone,
	                                  "enable-converted-stream-callback", &EnableConvertedSC,
	                                  NULL);
	if (err != MM_ERROR_NONE) {
		_mmcam_dbg_warn("Get attrs fail. (%s:%x)", err_name, err);
		SAFE_FREE(err_name);
		return err;
	}

	/* Get fourcc from picture format */
	sc->fourcc = _mmcamcorder_get_fourcc(sc->info_image->preview_format, codectype, hcamcorder->use_zero_copy_format);

	/* Get videosrc element and its name from configure */
	_mmcamcorder_conf_get_element(hcamcorder->conf_main,
	                              CONFIGURE_CATEGORY_MAIN_VIDEO_INPUT,
	                              "VideosrcElement",
	                              &VideosrcElement);
	_mmcamcorder_conf_get_value_element_name(VideosrcElement, &videosrc_name);

	/**
	 * Create child element
	 */
	__ta__("                videosrc_src",
	_MMCAMCORDER_ELEMENT_MAKE(sc, sc->element, _MMCAMCORDER_VIDEOSRC_SRC, videosrc_name, "videosrc_src", element_list, err);
	);
	__ta__("                videosrc_capsfilter",
	_MMCAMCORDER_ELEMENT_MAKE(sc, sc->element, _MMCAMCORDER_VIDEOSRC_FILT, "capsfilter", "videosrc_filter", element_list, err);
	);

	/* init high-speed-fps */
	MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "high-speed-fps", 0);

	/* set capture size, quality and flip setting which were set before mm_camcorder_realize */
	MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "capture-width", capture_width);
	MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "capture-height", capture_height);
	MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "capture-jpg-quality", capture_jpg_quality);
	MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "hdr-capture", sc->info_image->hdr_capture_mode);

	/* get video convert element name */
	_mmcamcorder_conf_get_value_element_name(sc->VideoconvertElement, &videoconvert_name);

	__ta__("                videosrc_queue",
	_MMCAMCORDER_ELEMENT_MAKE(sc, sc->element, _MMCAMCORDER_VIDEOSRC_QUE, "queue", "videosrc_queue", element_list, err);
	);

	/* set camera flip */
	__ta__("                _mmcamcorder_set_videosrc_flip",
	_mmcamcorder_set_videosrc_flip(handle, camera_flip);
	);

	/* set video stabilization mode */
	__ta__("                _mmcamcorder_set_videosrc_stabilization",
	_mmcamcorder_set_videosrc_stabilization(handle, video_stabilization);
	);

	/* set anti handshake mode */
	__ta__("                _mmcamcorder_set_videosrc_anti_shake",
	_mmcamcorder_set_videosrc_anti_shake(handle, anti_shake);
	);

	if (sc->is_modified_rate) {
		MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "high-speed-fps", fps);
	}

	/* Set basic infomation of videosrc element */
	__ta__("                _mmcamcorder_conf_set_value_element_property:Videosrc",
	_mmcamcorder_conf_set_value_element_property(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, VideosrcElement);
	);

	/* Set video device index */
	__ta__("                set camera-id",
	MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "camera-id", input_index->default_value);
	);

	_mmcam_dbg_log("Current mode[%d]", hcamcorder->type);

	/* Get videosink name */
	_mmcamcorder_conf_get_value_element_name(sc->VideosinkElement, &videosink_name);

	__ta__("                videosink_queue",
	_MMCAMCORDER_ELEMENT_MAKE(sc, sc->element, _MMCAMCORDER_VIDEOSINK_QUE, "queue", "videosink_queue", element_list, err);
	);

	__ta__("                videosink_sink",
	_MMCAMCORDER_ELEMENT_MAKE(sc, sc->element, _MMCAMCORDER_VIDEOSINK_SINK, videosink_name, "videosink_sink", element_list, err);
	);

	if (strcmp(videosink_name, "fakesink") != 0) {
		__ta__("                _mmcamcorder_videosink_window_set",
		if (_mmcamcorder_videosink_window_set(handle, sc->VideosinkElement) != MM_ERROR_NONE) {
			_mmcam_dbg_err("_mmcamcorder_videosink_window_set error");
			err = MM_ERROR_CAMCORDER_INVALID_ARGUMENT;
			goto pipeline_creation_error;
		}
		);
	}

	/* Set caps by rotation */
	__ta__("                _mmcamcorder_set_videosrc_rotation",
	_mmcamcorder_set_videosrc_rotation(handle, camera_rotate);
	);

	__ta__("                _mmcamcorder_conf_set_value_element_property:Videosink",
	_mmcamcorder_conf_set_value_element_property(sc->element[_MMCAMCORDER_VIDEOSINK_SINK].gst, sc->VideosinkElement);
	);

	/* add elements to main pipeline */
	__ta__("                _mmcamcorder_add_elements_to_bin:1",
	if (!_mmcamcorder_add_elements_to_bin(GST_BIN(sc->element[_MMCAMCORDER_MAIN_PIPE].gst), element_list)) {
		_mmcam_dbg_err("element_list add error.");
		err = MM_ERROR_CAMCORDER_RESOURCE_CREATION;
		goto pipeline_creation_error;
	}
	);

	/* link elements */
	__ta__("                _mmcamcorder_link_elements",
	if (!_mmcamcorder_link_elements(element_list)) {
		_mmcam_dbg_err( "element link error." );
		err = MM_ERROR_CAMCORDER_GST_LINK;
		goto pipeline_creation_error;
	}
	);

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	return MM_ERROR_NONE;

pipeline_creation_error:
	_MMCAMCORDER_ELEMENT_REMOVE(sc->element, _MMCAMCORDER_VIDEOSRC_SRC);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->element, _MMCAMCORDER_VIDEOSRC_FILT);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->element, _MMCAMCORDER_VIDEOSRC_CLS_QUE);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->element, _MMCAMCORDER_VIDEOSRC_CLS);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->element, _MMCAMCORDER_VIDEOSRC_CLS_FILT);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->element, _MMCAMCORDER_VIDEOSRC_QUE);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->element, _MMCAMCORDER_VIDEOSINK_QUE);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->element, _MMCAMCORDER_VIDEOSINK_CLS);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->element, _MMCAMCORDER_VIDEOSINK_SINK);

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	return err;
}


int _mmcamcorder_create_audiosrc_bin(MMHandleType handle)
{
	int err = MM_ERROR_NONE;
	int val = 0;
	int rate = 0;
	int format = 0;
	int channel = 0;
	int a_enc = MM_AUDIO_CODEC_AMR;
	int a_dev = MM_AUDIO_DEVICE_MIC;
	double volume = 0.0;
	char *err_name = NULL;
	char *audiosrc_name = NULL;
	char *cat_name = NULL;

	GstCaps *caps = NULL;
	GstPad *pad = NULL;
	GList *element_list  = NULL;

	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;
	_MMCamcorderGstElement *last_element = NULL;
	type_element *AudiosrcElement = NULL;

	mmf_return_val_if_fail(hcamcorder, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_CAMCORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->element, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	_mmcam_dbg_log("");

	err = _mmcamcorder_check_audiocodec_fileformat_compatibility(handle);
	if (err != MM_ERROR_NONE) {
		return err;
	}

	err = mm_camcorder_get_attributes(handle, &err_name,
	                                  MMCAM_AUDIO_DEVICE, &a_dev,
	                                  MMCAM_AUDIO_ENCODER, &a_enc,
	                                  MMCAM_AUDIO_ENCODER_BITRATE, &val,
	                                  MMCAM_AUDIO_SAMPLERATE, &rate,
	                                  MMCAM_AUDIO_FORMAT, &format,
	                                  MMCAM_AUDIO_CHANNEL, &channel,
	                                  MMCAM_AUDIO_VOLUME, &volume,
	                                  NULL);
	if (err != MM_ERROR_NONE) {
		_mmcam_dbg_warn("Get attrs fail. (%s:%x)", err_name, err);
		SAFE_FREE(err_name);
		return err;
	}

	/* Check existence */
	if (sc->encode_element[_MMCAMCORDER_AUDIOSRC_BIN].gst) {
		if (((GObject *)sc->encode_element[_MMCAMCORDER_AUDIOSRC_BIN].gst)->ref_count > 0) {
			gst_object_unref(sc->encode_element[_MMCAMCORDER_AUDIOSRC_BIN].gst);
		}
		_mmcam_dbg_log("_MMCAMCORDER_AUDIOSRC_BIN is Already existed. Unref once...");
	}

	/* Create bin element */
	__ta__("                audiosource_bin",
	_MMCAMCORDER_BIN_MAKE(sc, sc->encode_element, _MMCAMCORDER_AUDIOSRC_BIN, "audiosource_bin", err);
	);

	if (a_dev == MM_AUDIO_DEVICE_MODEM) {
		cat_name = strdup("AudiomodemsrcElement");
	} else {
		/* MM_AUDIO_DEVICE_MIC or others */
		cat_name = strdup("AudiosrcElement");
	}

	if (cat_name == NULL) {
		_mmcam_dbg_err("strdup failed.");
		err = MM_ERROR_CAMCORDER_LOW_MEMORY;
		goto pipeline_creation_error;
	}

	_mmcamcorder_conf_get_element(hcamcorder->conf_main,
	                              CONFIGURE_CATEGORY_MAIN_AUDIO_INPUT,
	                              cat_name,
	                              &AudiosrcElement);
	_mmcamcorder_conf_get_value_element_name(AudiosrcElement, &audiosrc_name);

	free(cat_name);
	cat_name = NULL;

	_mmcam_dbg_log("Audio src name : %s", audiosrc_name);

	__ta__("                audiosrc_src",
	_MMCAMCORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMCAMCORDER_AUDIOSRC_SRC, audiosrc_name, "audiosrc_src", element_list, err);
	);

	_mmcamcorder_conf_set_value_element_property(sc->encode_element[_MMCAMCORDER_AUDIOSRC_SRC].gst, AudiosrcElement);

	__ta__("                audiosource_capsfilter",
	_MMCAMCORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMCAMCORDER_AUDIOSRC_FILT, "capsfilter", "audiosrc_capsfilter", element_list, err);
	);

	_MMCAMCORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMCAMCORDER_AUDIOSRC_QUE, "queue", "audiosrc_queue", element_list, err);
	MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_AUDIOSRC_QUE].gst, "max-size-buffers", 0);
	MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_AUDIOSRC_QUE].gst, "max-size-bytes", 0);
	MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_AUDIOSRC_QUE].gst, "max-size-time", (int64_t)0);

	if (a_enc != MM_AUDIO_CODEC_VORBIS) {
		__ta__("                audiosource_volume",
		_MMCAMCORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMCAMCORDER_AUDIOSRC_VOL, "volume", "audiosrc_volume", element_list, err);
		);
	}

	/* Set basic infomation */
	if (a_enc != MM_AUDIO_CODEC_VORBIS) {
		int depth = 0;

		if (volume == 0.0) {
			/* Because data probe of audio src do the same job, it doesn't need to set "mute" here. Already null raw data. */
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_AUDIOSRC_VOL].gst, "volume", 1.0);
		} else {
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_AUDIOSRC_VOL].gst, "mute", FALSE);
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_AUDIOSRC_VOL].gst, "volume", volume);
		}

		if (format == MM_CAMCORDER_AUDIO_FORMAT_PCM_S16_LE) {
			depth = 16;
		} else { /* MM_CAMCORDER_AUDIO_FORMAT_PCM_U8 */
			depth = 8;
		}

		caps = gst_caps_new_simple("audio/x-raw-int",
		                           "rate", G_TYPE_INT, rate,
		                           "channels", G_TYPE_INT, channel,
		                           "depth", G_TYPE_INT, depth,
		                           NULL);
		_mmcam_dbg_log("caps [x-raw-int, rate:%d, channel:%d, depth:%d]",
		               rate, channel, depth);
	} else {
		/* what are the audio encoder which should get audio/x-raw-float? */
		caps = gst_caps_new_simple("audio/x-raw-float",
		                           "rate", G_TYPE_INT, rate,
		                           "channels", G_TYPE_INT, channel,
		                           "endianness", G_TYPE_INT, BYTE_ORDER,
		                           "width", G_TYPE_INT, 32,
		                           NULL);
		_mmcam_dbg_log("caps [x-raw-float, rate:%d, channel:%d, endianness:%d, width:32]",
		               rate, channel, BYTE_ORDER);
	}

	if (caps) {
		MMCAMCORDER_G_OBJECT_SET((sc->encode_element[_MMCAMCORDER_AUDIOSRC_FILT].gst), "caps", caps);
		gst_caps_unref(caps);
		caps = NULL;
	} else {
		_mmcam_dbg_err("create caps error");
		err = MM_ERROR_CAMCORDER_RESOURCE_CREATION;
		goto pipeline_creation_error;
	}

	if (!_mmcamcorder_add_elements_to_bin(GST_BIN(sc->encode_element[_MMCAMCORDER_AUDIOSRC_BIN].gst), element_list)) {
		_mmcam_dbg_err("element add error.");
		err = MM_ERROR_CAMCORDER_RESOURCE_CREATION;
		goto pipeline_creation_error;
	}

	if (!_mmcamcorder_link_elements(element_list)) {
		_mmcam_dbg_err("element link error.");
		err = MM_ERROR_CAMCORDER_GST_LINK;
		goto pipeline_creation_error;
	}

	last_element = (_MMCamcorderGstElement*)(g_list_last(element_list)->data);
	pad = gst_element_get_static_pad(last_element->gst, "src");
	if ((gst_element_add_pad(sc->encode_element[_MMCAMCORDER_AUDIOSRC_BIN].gst, gst_ghost_pad_new("src", pad) )) < 0) {
		gst_object_unref(pad);
		pad = NULL;
		_mmcam_dbg_err("failed to create ghost pad on _MMCAMCORDER_AUDIOSRC_BIN.");
		err = MM_ERROR_CAMCORDER_GST_LINK;
		goto pipeline_creation_error;
	}

	gst_object_unref(pad);
	pad = NULL;

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	return MM_ERROR_NONE;

pipeline_creation_error:
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_AUDIOSRC_SRC);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_AUDIOSRC_FILT);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_AUDIOSRC_QUE);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_AUDIOSRC_VOL);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_AUDIOSRC_BIN);

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	return err;
}


int _mmcamcorder_create_encodesink_bin(MMHandleType handle, MMCamcorderEncodebinProfile profile)
{
	int err = MM_ERROR_NONE;
	int result = 0;
	int channel = 0;
	int audio_enc = 0;
	int v_bitrate = 0;
	int a_bitrate = 0;
	int encodebin_profile = 0;
	int auto_audio_convert = 0;
	int auto_audio_resample = 0;
	int auto_color_space = 0;
	char *gst_element_venc_name = NULL;
	char *gst_element_aenc_name = NULL;
	char *gst_element_ienc_name = NULL;
	char *gst_element_mux_name = NULL;
	char *gst_element_rsink_name = NULL;
	char *str_profile = NULL;
	char *str_aac = NULL;
	char *str_aar = NULL;
	char *str_acs = NULL;
	char *err_name = NULL;
	char *videoconvert_name = NULL;

	GstCaps *caps = NULL;
	GstPad *pad = NULL;
	GList *element_list = NULL;
	GList *element_list2 = NULL;

	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;
	type_element *VideoencElement = NULL;
	type_element *AudioencElement = NULL;
	type_element *ImageencElement = NULL;
	type_element *MuxElement = NULL;
	type_element *RecordsinkElement = NULL;

	mmf_return_val_if_fail(hcamcorder, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_CAMCORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->element, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	_mmcam_dbg_log("start - profile : %d", profile);

	/* Check existence */
	if (sc->encode_element[_MMCAMCORDER_ENCSINK_BIN].gst) {
		if (((GObject *)sc->encode_element[_MMCAMCORDER_ENCSINK_BIN].gst)->ref_count > 0) {
			gst_object_unref(sc->encode_element[_MMCAMCORDER_ENCSINK_BIN].gst);
		}
		_mmcam_dbg_log("_MMCAMCORDER_ENCSINK_BIN is Already existed.");
	}

	/* Create bin element */
	__ta__("                encodesink_bin",
	_MMCAMCORDER_BIN_MAKE(sc, sc->encode_element, _MMCAMCORDER_ENCSINK_BIN, "encodesink_bin", err);
	);

	/* Create child element */
	if (profile != MM_CAMCORDER_ENCBIN_PROFILE_AUDIO) {
		/* create appsrc and capsfilter */
		__ta__("                encodesink_src",
		_MMCAMCORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMCAMCORDER_ENCSINK_SRC, "appsrc", "encodesink_src", element_list, err);
		);

		__ta__("                encodesink_filter",
		_MMCAMCORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMCAMCORDER_ENCSINK_FILT, "capsfilter", "encodesink_filter", element_list, err);
		);

		/* release element_list, they will be placed out of encodesink bin */
		if (element_list) {
			g_list_free(element_list);
			element_list = NULL;
		}

		/* set appsrc as live source */
		MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_SRC].gst, "is-live", TRUE);
		MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_SRC].gst, "max-bytes", (int64_t)0);

		/* set capsfilter */
		pad = gst_element_get_static_pad(sc->element[_MMCAMCORDER_VIDEOSRC_QUE].gst, "src");
		if (!pad) {
			_mmcam_dbg_err("get videosrc_que src pad failed");
			goto pipeline_creation_error;
		}

		if (sc->info_video->record_dual_stream) {
			GstCaps *video_caps = gst_caps_copy(GST_PAD_CAPS(pad));

			if (video_caps) {
				gst_caps_set_simple(video_caps,
				                    "width", G_TYPE_INT, sc->info_video->video_width,
				                    "height", G_TYPE_INT, sc->info_video->video_height,
				                    NULL);

				_mmcam_dbg_log("video caps [%"GST_PTR_FORMAT"]", video_caps);

				MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_FILT].gst, "caps", video_caps);

				gst_caps_unref(video_caps);
				video_caps = NULL;
			} else {
				_mmcam_dbg_err("create recording pipeline caps failed");

				gst_object_unref(pad);
				pad = NULL;

				goto pipeline_creation_error;
			}
		} else {
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_FILT].gst, "caps", GST_PAD_CAPS(pad));
		}

		gst_object_unref(pad);
		pad = NULL;

		/* connect signal for ready to push buffer */
		MMCAMCORDER_SIGNAL_CONNECT(sc->encode_element[_MMCAMCORDER_ENCSINK_SRC].gst,
		                           _MMCAMCORDER_HANDLER_VIDEOREC,
		                           "need-data",
		                           _mmcamcorder_ready_to_encode_callback,
		                           hcamcorder);
	}

	__ta__("                encodesink_encbin",
	_MMCAMCORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMCAMCORDER_ENCSINK_ENCBIN, "encodebin", "encodesink_encbin", element_list, err);
	);

	/* check element availability */
	mm_camcorder_get_attributes(handle, &err_name,
	                            MMCAM_AUDIO_ENCODER, &audio_enc,
	                            MMCAM_AUDIO_CHANNEL, &channel,
	                            MMCAM_VIDEO_ENCODER_BITRATE, &v_bitrate,
	                            MMCAM_AUDIO_ENCODER_BITRATE, &a_bitrate,
	                            NULL);

	_mmcam_dbg_log("Profile[%d]", profile);

	/* Set information */
	if (profile == MM_CAMCORDER_ENCBIN_PROFILE_VIDEO) {
		str_profile = "VideoProfile";
		str_aac = "VideoAutoAudioConvert";
		str_aar = "VideoAutoAudioResample";
		str_acs = "VideoAutoColorSpace";
	} else if (profile == MM_CAMCORDER_ENCBIN_PROFILE_AUDIO) {
		str_profile = "AudioProfile";
		str_aac = "AudioAutoAudioConvert";
		str_aar = "AudioAutoAudioResample";
		str_acs = "AudioAutoColorSpace";
	} else if (profile == MM_CAMCORDER_ENCBIN_PROFILE_IMAGE) {
		str_profile = "ImageProfile";
		str_aac = "ImageAutoAudioConvert";
		str_aar = "ImageAutoAudioResample";
		str_acs = "ImageAutoColorSpace";
	}

	_mmcamcorder_conf_get_value_int(hcamcorder->conf_main, CONFIGURE_CATEGORY_MAIN_RECORD, str_profile, &encodebin_profile);
	_mmcamcorder_conf_get_value_int(hcamcorder->conf_main, CONFIGURE_CATEGORY_MAIN_RECORD, str_aac, &auto_audio_convert);
	_mmcamcorder_conf_get_value_int(hcamcorder->conf_main, CONFIGURE_CATEGORY_MAIN_RECORD, str_aar, &auto_audio_resample);
	_mmcamcorder_conf_get_value_int(hcamcorder->conf_main, CONFIGURE_CATEGORY_MAIN_RECORD, str_acs, &auto_color_space);

	_mmcam_dbg_log("Profile:%d, AutoAudioConvert:%d, AutoAudioResample:%d, AutoColorSpace:%d",
	               encodebin_profile, auto_audio_convert, auto_audio_resample, auto_color_space);

	MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "profile", encodebin_profile);
	MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "auto-audio-convert", auto_audio_convert);
	MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "auto-audio-resample", auto_audio_resample);
	MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "auto-colorspace", auto_color_space);
	MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "use-video-toggle", FALSE);

	/* get video convert element name */
	_mmcamcorder_conf_get_value_element_name(sc->VideoconvertElement, &videoconvert_name);

	/* Codec */
	if (profile == MM_CAMCORDER_ENCBIN_PROFILE_VIDEO) {
		int use_venc_queue = 0;

		VideoencElement = _mmcamcorder_get_type_element(handle, MM_CAM_VIDEO_ENCODER);

		if (!VideoencElement) {
			_mmcam_dbg_err("Fail to get type element");
			err = MM_ERROR_CAMCORDER_RESOURCE_CREATION;
			goto pipeline_creation_error;
		}

		_mmcamcorder_conf_get_value_element_name(VideoencElement, &gst_element_venc_name);

		MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "venc-name", gst_element_venc_name);
		_MMCAMCORDER_ENCODEBIN_ELMGET(sc, _MMCAMCORDER_ENCSINK_VENC, "video-encode", err);

		_mmcamcorder_conf_get_value_int(hcamcorder->conf_main,
		                                CONFIGURE_CATEGORY_MAIN_RECORD,
		                                "UseVideoEncoderQueue",
		                                &use_venc_queue);
		if (use_venc_queue) {
			_MMCAMCORDER_ENCODEBIN_ELMGET(sc, _MMCAMCORDER_ENCSINK_VENC_QUE, "use-venc-queue", err);
		}
	}

	if (sc->audio_disable == FALSE &&
	    profile != MM_CAMCORDER_ENCBIN_PROFILE_IMAGE) {
		int use_aenc_queue =0;

		AudioencElement = _mmcamcorder_get_type_element(handle, MM_CAM_AUDIO_ENCODER);
		if (!AudioencElement) {
			_mmcam_dbg_err("Fail to get type element");
			err = MM_ERROR_CAMCORDER_RESOURCE_CREATION;
			goto pipeline_creation_error;
		}

		_mmcamcorder_conf_get_value_element_name(AudioencElement, &gst_element_aenc_name);

		MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "aenc-name", gst_element_aenc_name);
		_MMCAMCORDER_ENCODEBIN_ELMGET(sc, _MMCAMCORDER_ENCSINK_AENC, "audio-encode", err);

		if (audio_enc == MM_AUDIO_CODEC_AMR && channel == 2) {
			caps = gst_caps_new_simple("audio/x-raw-int",
			                           "channels", G_TYPE_INT, 1,
			                           NULL);
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "auto-audio-convert", TRUE);
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "acaps", caps);
			gst_caps_unref (caps);
			caps = NULL;
		}

		if (audio_enc == MM_AUDIO_CODEC_OGG) {
			caps = gst_caps_new_simple("audio/x-raw-int",
			                           NULL);
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "auto-audio-convert", TRUE);
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "acaps", caps);
			gst_caps_unref (caps);
			caps = NULL;
			_mmcam_dbg_log("***** MM_AUDIO_CODEC_OGG : setting audio/x-raw-int ");
		}

		_mmcamcorder_conf_get_value_int(hcamcorder->conf_main,
		                                CONFIGURE_CATEGORY_MAIN_RECORD,
		                                "UseAudioEncoderQueue",
		                                &use_aenc_queue);
		if (use_aenc_queue) {
			_MMCAMCORDER_ENCODEBIN_ELMGET(sc, _MMCAMCORDER_ENCSINK_AENC_QUE, "use-aenc-queue", err);
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_AENC_QUE].gst,"max-size-time", (int64_t)0);
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_AENC_QUE].gst,"max-size-buffers", 0);
		}
	}

	if (profile == MM_CAMCORDER_ENCBIN_PROFILE_IMAGE) {
		ImageencElement = _mmcamcorder_get_type_element(handle, MM_CAM_IMAGE_ENCODER);
		if (!ImageencElement) {
			_mmcam_dbg_err("Fail to get type element");
			err = MM_ERROR_CAMCORDER_RESOURCE_CREATION;
			goto pipeline_creation_error;
		}

		_mmcamcorder_conf_get_value_element_name(ImageencElement, &gst_element_ienc_name);

		MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "ienc-name", gst_element_ienc_name);
		_MMCAMCORDER_ENCODEBIN_ELMGET(sc, _MMCAMCORDER_ENCSINK_IENC, "image-encode", err);
	}

	/* Mux */
	if (profile != MM_CAMCORDER_ENCBIN_PROFILE_IMAGE) {
		MuxElement = _mmcamcorder_get_type_element(handle, MM_CAM_FILE_FORMAT);
		if (!MuxElement) {
			_mmcam_dbg_err("Fail to get type element");
			err = MM_ERROR_CAMCORDER_RESOURCE_CREATION;
			goto pipeline_creation_error;
		}

		_mmcamcorder_conf_get_value_element_name(MuxElement, &gst_element_mux_name);

		__ta__("                mux",
		MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "mux-name", gst_element_mux_name);
		);
		_MMCAMCORDER_ENCODEBIN_ELMGET(sc, _MMCAMCORDER_ENCSINK_MUX, "mux", err);

		_mmcamcorder_conf_set_value_element_property(sc->encode_element[_MMCAMCORDER_ENCSINK_MUX].gst, MuxElement);
	}

	/* Sink */
	if (profile != MM_CAMCORDER_ENCBIN_PROFILE_IMAGE) {
		/* for recording */
		_mmcamcorder_conf_get_element(hcamcorder->conf_main,
		                              CONFIGURE_CATEGORY_MAIN_RECORD,
		                              "RecordsinkElement",
		                              &RecordsinkElement );
		_mmcamcorder_conf_get_value_element_name(RecordsinkElement, &gst_element_rsink_name);

		__ta__("                Recordsink_sink",
		_MMCAMCORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMCAMCORDER_ENCSINK_SINK, gst_element_rsink_name, NULL, element_list, err);
		);

		_mmcamcorder_conf_set_value_element_property(sc->encode_element[_MMCAMCORDER_ENCSINK_SINK].gst, RecordsinkElement);
	} else {
		/* for stillshot */
		__ta__("                fakesink",
		_MMCAMCORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMCAMCORDER_ENCSINK_SINK, "fakesink", NULL, element_list, err);
		);
	}

	if (profile == MM_CAMCORDER_ENCBIN_PROFILE_VIDEO) {
		/* video encoder attribute setting */
		if (v_bitrate > 0) {
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_VENC].gst, "bitrate", v_bitrate);
		} else {
			_mmcam_dbg_warn("video bitrate is too small[%d], so skip setting. Use DEFAULT value.", v_bitrate);
		}
		/*MMCAMCORDER_G_OBJECT_SET ((sc->encode_element[_MMCAMCORDER_ENCSINK_VENC].gst),"hw-accel", v_hw);*/
		_mmcamcorder_conf_set_value_element_property(sc->encode_element[_MMCAMCORDER_ENCSINK_VENC].gst, VideoencElement);
	}

	if (sc->audio_disable == FALSE &&
	    profile != MM_CAMCORDER_ENCBIN_PROFILE_IMAGE) {
		/* audio encoder attribute setting */
		if (a_bitrate > 0) {
			switch (audio_enc) {
			case MM_AUDIO_CODEC_AMR:
				result = __mmcamcorder_get_amrnb_bitrate_mode(a_bitrate);
				_mmcam_dbg_log("Set AMR encoder[%s] mode [%d]", gst_element_aenc_name, result);
				MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_AENC].gst, "band-mode", result);
				break;
			case MM_AUDIO_CODEC_AAC:
				_mmcam_dbg_log("Set AAC encoder[%s] bitrate [%d]", gst_element_aenc_name, a_bitrate);
				MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_AENC].gst, "bitrate", a_bitrate);
				break;
			default:
				_mmcam_dbg_log("Audio codec is not AMR or AAC... you need to implement setting function for audio encoder bit-rate");
				break;
			}
		} else {
			_mmcam_dbg_warn("Setting bitrate is too small, so skip setting. Use DEFAULT value.");
		}

		_mmcamcorder_conf_set_value_element_property(sc->encode_element[_MMCAMCORDER_ENCSINK_AENC].gst, AudioencElement);
	}

	_mmcam_dbg_log("Element creation complete");

	/* Add element to bin */
	if (!_mmcamcorder_add_elements_to_bin(GST_BIN(sc->encode_element[_MMCAMCORDER_ENCSINK_BIN].gst), element_list)) {
		_mmcam_dbg_err("element add error.");
		err = MM_ERROR_CAMCORDER_RESOURCE_CREATION;
		goto pipeline_creation_error;
	}

	_mmcam_dbg_log("Element add complete");

	if (profile == MM_CAMCORDER_ENCBIN_PROFILE_VIDEO) {
		pad = gst_element_get_request_pad(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "video");
		if (gst_element_add_pad(sc->encode_element[_MMCAMCORDER_ENCSINK_BIN].gst, gst_ghost_pad_new("video_sink0", pad)) < 0) {
			gst_object_unref(pad);
			pad = NULL;
			_mmcam_dbg_err("failed to create ghost video_sink0 on _MMCAMCORDER_ENCSINK_BIN.");
			err = MM_ERROR_CAMCORDER_GST_LINK;
			goto pipeline_creation_error;
		}
		gst_object_unref(pad);
		pad = NULL;

		if (sc->audio_disable == FALSE) {
			pad = gst_element_get_request_pad(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "audio");
			if (gst_element_add_pad(sc->encode_element[_MMCAMCORDER_ENCSINK_BIN].gst, gst_ghost_pad_new("audio_sink0", pad)) < 0) {
				gst_object_unref(pad);
				pad = NULL;
				_mmcam_dbg_err("failed to create ghost audio_sink0 on _MMCAMCORDER_ENCSINK_BIN.");
				err = MM_ERROR_CAMCORDER_GST_LINK;
				goto pipeline_creation_error;
			}
			gst_object_unref(pad);
			pad = NULL;
		}
	} else if (profile == MM_CAMCORDER_ENCBIN_PROFILE_AUDIO) {
		pad = gst_element_get_request_pad(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "audio");
		if (gst_element_add_pad(sc->encode_element[_MMCAMCORDER_ENCSINK_BIN].gst, gst_ghost_pad_new("audio_sink0", pad)) < 0) {
			gst_object_unref(pad);
			pad = NULL;
			_mmcam_dbg_err("failed to create ghost audio_sink0 on _MMCAMCORDER_ENCSINK_BIN.");
			err = MM_ERROR_CAMCORDER_GST_LINK;
			goto pipeline_creation_error;
		}
		gst_object_unref(pad);
		pad = NULL;
	} else {
		/* for stillshot */
		pad = gst_element_get_request_pad(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "image");
		if (gst_element_add_pad(sc->encode_element[_MMCAMCORDER_ENCSINK_BIN].gst, gst_ghost_pad_new("image_sink0", pad)) < 0) {
			gst_object_unref(pad);
			pad = NULL;
			_mmcam_dbg_err("failed to create ghost image_sink0 on _MMCAMCORDER_ENCSINK_BIN.");
			err = MM_ERROR_CAMCORDER_GST_LINK;
			goto pipeline_creation_error;
		}
		gst_object_unref(pad);
		pad = NULL;
	}

	_mmcam_dbg_log("Get pad complete");

	/* Link internal element */
	if (!_mmcamcorder_link_elements(element_list)) {
		_mmcam_dbg_err("element link error.");
		err = MM_ERROR_CAMCORDER_GST_LINK;
		goto pipeline_creation_error;
	}

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	_mmcam_dbg_log("done");

	return MM_ERROR_NONE;

pipeline_creation_error :
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_ENCSINK_ENCBIN);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_ENCSINK_SRC);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_ENCSINK_FILT);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_ENCSINK_VENC);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_ENCSINK_AENC);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_ENCSINK_IENC);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_ENCSINK_MUX);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_ENCSINK_SINK);
	_MMCAMCORDER_ELEMENT_REMOVE(sc->encode_element, _MMCAMCORDER_ENCSINK_BIN);

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	if (element_list2) {
		g_list_free(element_list2);
		element_list2 = NULL;
	}

	return err;
}


int _mmcamcorder_create_preview_pipeline(MMHandleType handle)
{
	int err = MM_ERROR_NONE;

	GstPad *srcpad = NULL;
	GstBus *bus = NULL;

	mmf_camcorder_t *hcamcorder= MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;

	mmf_return_val_if_fail(hcamcorder, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_CAMCORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->element, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	_mmcam_dbg_log("");

	/** Create gstreamer element **/
	/* Main pipeline */
	_MMCAMCORDER_PIPELINE_MAKE(sc, sc->element, _MMCAMCORDER_MAIN_PIPE, "camera_pipeline", err);

	/* Sub pipeline */
	__ta__("            _mmcamcorder_create_preview_elements",
	err = _mmcamcorder_create_preview_elements((MMHandleType)hcamcorder);
	);
	if (err != MM_ERROR_NONE) {
		goto pipeline_creation_error;
	}

	/* Set data probe function */
	if (sc->element[_MMCAMCORDER_VIDEOSRC_QUE].gst) {
		_mmcam_dbg_warn("add video dataprobe to videosrc queue");
		srcpad = gst_element_get_static_pad(sc->element[_MMCAMCORDER_VIDEOSRC_QUE].gst, "src");
	} else {
		_mmcam_dbg_err("there is no queue plugin");
		goto pipeline_creation_error;
	}

	if (srcpad) {
		MMCAMCORDER_ADD_BUFFER_PROBE(srcpad, _MMCAMCORDER_HANDLER_PREVIEW,
		                             __mmcamcorder_video_dataprobe_preview, hcamcorder);
		gst_object_unref(srcpad);
		srcpad = NULL;
	} else {
		_mmcam_dbg_err("failed to get srcpad");
		goto pipeline_creation_error;
	}

	/* set dataprobe for video recording */
	srcpad = gst_element_get_static_pad(sc->element[_MMCAMCORDER_VIDEOSINK_QUE].gst, "src");
	MMCAMCORDER_ADD_BUFFER_PROBE(srcpad, _MMCAMCORDER_HANDLER_PREVIEW,
	                             __mmcamcorder_video_dataprobe_push_buffer_to_record, hcamcorder);
	gst_object_unref(srcpad);
	srcpad = NULL;

	bus = gst_pipeline_get_bus(GST_PIPELINE(sc->element[_MMCAMCORDER_MAIN_PIPE].gst));

	/* Register pipeline message callback */
	hcamcorder->pipeline_cb_event_id = gst_bus_add_watch(bus, _mmcamcorder_pipeline_cb_message, (gpointer)hcamcorder);

	/* set sync handler */
	gst_bus_set_sync_handler(bus, _mmcamcorder_pipeline_bus_sync_callback, (gpointer)hcamcorder);

	gst_object_unref(bus);
	bus = NULL;

	/* Below signals are meaningfull only when video source is using. */
	MMCAMCORDER_SIGNAL_CONNECT(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst,
	                           _MMCAMCORDER_HANDLER_PREVIEW,
	                           "nego-complete",
	                           _mmcamcorder_negosig_handler,
	                           hcamcorder);

	return MM_ERROR_NONE;

pipeline_creation_error:
	_MMCAMCORDER_ELEMENT_REMOVE(sc->element, _MMCAMCORDER_MAIN_PIPE);
	return err;
}


void _mmcamcorder_negosig_handler(GstElement *videosrc, MMHandleType handle)
{
	mmf_camcorder_t *hcamcorder= MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;

	_mmcam_dbg_log("");

	mmf_return_if_fail(hcamcorder);
	mmf_return_if_fail(hcamcorder->sub_context);
	sc = MMF_CAMCORDER_SUBCONTEXT(handle);

	/* kernel was modified. No need to set.
	_mmcamcorder_set_attribute_to_camsensor(handle);
	*/

	if (sc->cam_stability_count != _MMCAMCORDER_CAMSTABLE_COUNT) {
		sc->cam_stability_count = _MMCAMCORDER_CAMSTABLE_COUNT;
	}

	if (hcamcorder->type != MM_CAMCORDER_MODE_AUDIO) {
		_MMCamcorderImageInfo *info = NULL;
		info = sc->info_image;
		if (info->resolution_change == TRUE) {
			_mmcam_dbg_log("open toggle of stillshot sink.");
			MMCAMCORDER_G_OBJECT_SET(sc->encode_element[_MMCAMCORDER_ENCSINK_ENCBIN].gst, "block", FALSE);
			info->resolution_change = FALSE;
		}
	}
}


void _mmcamcorder_ready_to_encode_callback(GstElement *element, guint size, gpointer handle)
{
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;

	/*_mmcam_dbg_log("start");*/

	mmf_return_if_fail(hcamcorder);
	mmf_return_if_fail(hcamcorder->sub_context);
	sc = MMF_CAMCORDER_SUBCONTEXT(handle);

	/* set flag */
	if (sc->info_video->push_encoding_buffer == PUSH_ENCODING_BUFFER_INIT) {
		sc->info_video->push_encoding_buffer = PUSH_ENCODING_BUFFER_RUN;
		_mmcam_dbg_warn("set push_encoding_buffer RUN");
	}
}


int _mmcamcorder_videosink_window_set(MMHandleType handle, type_element* VideosinkElement)
{
	int err = MM_ERROR_NONE;
	int size = 0;
	int retx = 0;
	int rety = 0;
	int retwidth = 0;
	int retheight = 0;
	int visible = 0;
	int rotation = MM_DISPLAY_ROTATION_NONE;
	int flip = MM_FLIP_NONE;
	int display_mode = MM_DISPLAY_MODE_DEFAULT;
	int display_geometry_method = MM_DISPLAY_METHOD_LETTER_BOX;
	int display_surface = MM_DISPLAY_SURFACE_X;
	int origin_size = 0;
	int zoom_attr = 0;
	int zoom_level = 0;
	int do_scaling = FALSE;
	int *overlay = NULL;
	gulong xid;
	char *err_name = NULL;
	char *videosink_name = NULL;

	GstElement *vsink = NULL;

	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;

	_mmcam_dbg_log("");

	mmf_return_val_if_fail(hcamcorder, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_CAMCORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->element, MM_ERROR_CAMCORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->element[_MMCAMCORDER_VIDEOSINK_SINK].gst, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	vsink = sc->element[_MMCAMCORDER_VIDEOSINK_SINK].gst;

	/* Get video display information */
	__ta__("                    videosink get attributes",
	err = mm_camcorder_get_attributes(handle, &err_name,
	                                  MMCAM_DISPLAY_RECT_X, &retx,
	                                  MMCAM_DISPLAY_RECT_Y, &rety,
	                                  MMCAM_DISPLAY_RECT_WIDTH, &retwidth,
	                                  MMCAM_DISPLAY_RECT_HEIGHT, &retheight,
	                                  MMCAM_DISPLAY_ROTATION, &rotation,
	                                  MMCAM_DISPLAY_FLIP, &flip,
	                                  MMCAM_DISPLAY_VISIBLE, &visible,
	                                  MMCAM_DISPLAY_HANDLE, (void**)&overlay, &size,
	                                  MMCAM_DISPLAY_MODE, &display_mode,
	                                  MMCAM_DISPLAY_GEOMETRY_METHOD, &display_geometry_method,
	                                  MMCAM_DISPLAY_SCALE, &zoom_attr,
	                                  MMCAM_DISPLAY_EVAS_DO_SCALING, &do_scaling,
	                                  MMCAM_DISPLAY_SURFACE, &display_surface,
	                                  NULL);
	);

	_mmcamcorder_conf_get_value_element_name(VideosinkElement, &videosink_name);

	if (display_surface == MM_DISPLAY_SURFACE_X_EXT) {
		void *pixmap_cb = (void *)overlay;
		void *pixmap_cb_user_data = NULL;
		int data_size = 0;

		mm_camcorder_get_attributes(handle, &err_name,
		                            MMCAM_DISPLAY_HANDLE_USER_DATA, &pixmap_cb_user_data, &data_size,
		                            NULL);

		_mmcam_dbg_log("set pixmap cb[%p] and user data[%p]", pixmap_cb, pixmap_cb_user_data);

		/* set pixmap callback and user data */
		MMCAMCORDER_G_OBJECT_SET(vsink, "pixmap-id-callback", pixmap_cb);
		MMCAMCORDER_G_OBJECT_SET(vsink, "pixmap-id-callback-userdata", pixmap_cb_user_data);

		/* connect render error signal */
		MMCAMCORDER_SIGNAL_CONNECT(vsink,
		                           _MMCAMCORDER_HANDLER_PREVIEW,
		                           "frame-render-error",
		                           __mmcamcorder_videoframe_render_error_cb,
		                           hcamcorder);
	} else {
		_mmcam_dbg_log("(overlay=%p, size=%d)", overlay, size);

		/* Set display handle */
		if (!strcmp(videosink_name, "xvimagesink") ||
		    !strcmp(videosink_name, "ximagesink")) {
			if (overlay) {
				xid = *overlay;
				_mmcam_dbg_log("xid = %lu )", xid);
				__ta__("                    gst_x_overlay_set_xwindow_id",
				gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(vsink), xid);
				);
			} else {
				_mmcam_dbg_warn("Handle is NULL. Set xid as 0.. but, it's not recommended.");
				__ta__("                    gst_x_overlay_set_xwindow_id:0",
				gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(vsink), 0);
				);
			}
		} else if (!strcmp(videosink_name, "evasimagesink") ||
		           !strcmp(videosink_name, "evaspixmapsink")) {
			_mmcam_dbg_log("videosink : %s, handle : %p", videosink_name, overlay);
			if (overlay) {
				__ta__("                    set evas-object",
				MMCAMCORDER_G_OBJECT_SET(vsink, "evas-object", overlay);
				MMCAMCORDER_G_OBJECT_SET(vsink, "origin-size", !do_scaling);
				);
			} else {
				_mmcam_dbg_err("display handle(eavs object) is NULL");
				return MM_ERROR_CAMCORDER_INVALID_ARGUMENT;
			}
		} else {
			_mmcam_dbg_warn("Who are you?? (Videosink: %s)", videosink_name);
		}
	}

	_mmcam_dbg_log("%s set: display_geometry_method[%d],origin-size[%d],visible[%d],rotate[%d],flip[%d]",
	               videosink_name, display_geometry_method, origin_size, visible, rotation, flip);

	/* Set attribute */
	if (!strcmp(videosink_name, "xvimagesink") ||
	    !strcmp(videosink_name, "evaspixmapsink")) {
		/* set rotation */
		MMCAMCORDER_G_OBJECT_SET(vsink, "rotate", rotation);

		/* set flip */
		MMCAMCORDER_G_OBJECT_SET(vsink, "flip", flip);

		switch (zoom_attr) {
		case MM_DISPLAY_SCALE_DEFAULT:
			zoom_level = 1;
			break;
		case MM_DISPLAY_SCALE_DOUBLE_LENGTH:
			zoom_level = 2;
			break;
		case MM_DISPLAY_SCALE_TRIPLE_LENGTH:
			zoom_level = 3;
			break;
		default:
			_mmcam_dbg_warn("Unsupported zoom value. set as default.");
			zoom_level = 1;
			break;
		}

		__ta__("                    vsink:geomethod,mode,visible,mode",
		MMCAMCORDER_G_OBJECT_SET(vsink, "display-geometry-method", display_geometry_method);
		MMCAMCORDER_G_OBJECT_SET(vsink, "display-mode", display_mode);
		MMCAMCORDER_G_OBJECT_SET(vsink, "visible", visible);
		MMCAMCORDER_G_OBJECT_SET(vsink, "zoom", (float)zoom_level);
		);

		if (display_geometry_method == MM_DISPLAY_METHOD_CUSTOM_ROI) {
			g_object_set(vsink,
			             "dst-roi-x", retx,
			             "dst-roi-y", rety,
			             "dst-roi-w", retwidth,
			             "dst-roi-h", retheight,
			             NULL);
		}
	}

	return MM_ERROR_NONE;
}


int _mmcamcorder_vframe_stablize(MMHandleType handle)
{
	mmf_camcorder_t *hcamcorder= MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;

	/*_mmcam_dbg_log("%d", _MMCAMCORDER_CAMSTABLE_COUNT);*/

	mmf_return_val_if_fail(hcamcorder, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);

	mmf_return_val_if_fail(sc, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	if (sc->cam_stability_count != _MMCAMCORDER_CAMSTABLE_COUNT) {
		sc->cam_stability_count = _MMCAMCORDER_CAMSTABLE_COUNT;
	}

	return MM_ERROR_NONE;
}

/* Retreive device information and set them to attributes */
gboolean _mmcamcorder_get_device_info(MMHandleType handle)
{
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;
	GstCameraControl *control = NULL;
	GstCameraControlExifInfo exif_info = {0,};

	mmf_return_val_if_fail(hcamcorder, FALSE);
	sc = MMF_CAMCORDER_SUBCONTEXT(handle);

	if (sc && sc->element) {
		int err = MM_ERROR_NONE;
		char *err_name = NULL;
		double focal_len = 0.0;

		/* Video input device */
		if (sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst) {
			/* Exif related information */
			control = GST_CAMERA_CONTROL(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst);
			if (control != NULL) {
				gst_camera_control_get_exif_info(control, &exif_info); //get video input device information
				focal_len = ((double)exif_info.focal_len_numerator) / ((double) exif_info.focal_len_denominator);
			} else {
				_mmcam_dbg_err("Fail to get camera control interface!");
				focal_len = 0.0;
			}
		}

		/* Set values to attributes */
		err = mm_camcorder_set_attributes(handle, &err_name,
		                                  MMCAM_CAMERA_FOCAL_LENGTH, focal_len,
		                                  NULL);
		if (err != MM_ERROR_NONE) {
			_mmcam_dbg_err("Set attributes error(%s:%x)!", err_name, err);
			if (err_name) {
				free(err_name);
				err_name = NULL;
			}
			return FALSE;
		}
	} else {
		_mmcam_dbg_warn( "Sub context isn't exist.");
		return FALSE;
	}

	return TRUE;
}


static gboolean __mmcamcorder_video_dataprobe_preview(GstPad *pad, GstBuffer *buffer, gpointer u_data)
{
	int current_state = MM_CAMCORDER_STATE_NONE;

	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(u_data);
	_MMCamcorderSubContext *sc = NULL;
	_MMCamcorderKPIMeasure *kpi = NULL;

	mmf_return_val_if_fail(buffer, FALSE);
	mmf_return_val_if_fail(GST_BUFFER_DATA(buffer), FALSE);
	mmf_return_val_if_fail(hcamcorder, TRUE);

	sc = MMF_CAMCORDER_SUBCONTEXT(u_data);
	mmf_return_val_if_fail(sc, TRUE);

	current_state = hcamcorder->state;

	if (sc->drop_vframe > 0) {
		if (sc->pass_first_vframe > 0) {
			sc->pass_first_vframe--;
			_mmcam_dbg_log("Pass video frame by pass_first_vframe");
		} else {
			sc->drop_vframe--;
			_mmcam_dbg_log("Drop video frame by drop_vframe");
			return FALSE;
		}
	} else if (sc->cam_stability_count > 0) {
		sc->cam_stability_count--;
		_mmcam_dbg_log("Drop video frame by cam_stability_count");
		return FALSE;
	}

	if (current_state >= MM_CAMCORDER_STATE_PREPARE) {
		int diff_sec;
		int frame_count = 0;
		struct timeval current_video_time;

		kpi = &(sc->kpi);
		if (kpi->init_video_time.tv_sec == kpi->last_video_time.tv_sec &&
		    kpi->init_video_time.tv_usec == kpi->last_video_time.tv_usec &&
		    kpi->init_video_time.tv_usec  == 0) {
			/*
			_mmcam_dbg_log("START to measure FPS");
			*/
			gettimeofday(&(kpi->init_video_time), NULL);
		}

		frame_count = ++(kpi->video_framecount);

		gettimeofday(&current_video_time, NULL);
		diff_sec = current_video_time.tv_sec - kpi->last_video_time.tv_sec;
		if (diff_sec != 0) {
			kpi->current_fps = (frame_count - kpi->last_framecount) / diff_sec;
			if ((current_video_time.tv_sec - kpi->init_video_time.tv_sec) != 0) {
				int framecount = kpi->video_framecount;
				int elased_sec = current_video_time.tv_sec - kpi->init_video_time.tv_sec;
				kpi->average_fps = framecount / elased_sec;
			}

			kpi->last_framecount = frame_count;
			kpi->last_video_time.tv_sec = current_video_time.tv_sec;
			kpi->last_video_time.tv_usec = current_video_time.tv_usec;
			/*
			_mmcam_dbg_log("current fps(%d), average(%d)", kpi->current_fps, kpi->average_fps);
			*/
		}
	}

	/* video stream callback */
	if (hcamcorder->vstream_cb && buffer) {
		GstCaps *caps = NULL;
		GstStructure *structure = NULL;
		int state = MM_CAMCORDER_STATE_NULL;
		unsigned int fourcc = 0;
		MMCamcorderVideoStreamDataType stream;

		state = _mmcamcorder_get_state((MMHandleType)hcamcorder);
		if (state < MM_CAMCORDER_STATE_PREPARE) {
			_mmcam_dbg_warn("Not ready for stream callback");
			return TRUE;
		}

		caps = gst_buffer_get_caps(buffer);
		if (caps == NULL) {
			_mmcam_dbg_warn( "Caps is NULL." );
			return TRUE;
		}

		/* clear stream data structure */
		memset(&stream, 0x0, sizeof(MMCamcorderVideoStreamDataType));

		structure = gst_caps_get_structure( caps, 0 );
		gst_structure_get_int(structure, "width", &(stream.width));
		gst_structure_get_int(structure, "height", &(stream.height));
		gst_structure_get_fourcc(structure, "format", &fourcc);
		stream.format = _mmcamcorder_get_pixtype(fourcc);
		gst_caps_unref( caps );
		caps = NULL;

		/*
		_mmcam_dbg_log( "Call video steramCb, data[%p], Width[%d],Height[%d], Format[%d]",
		                GST_BUFFER_DATA(buffer), stream.width, stream.height, stream.format );
		*/

		if (stream.width == 0 || stream.height == 0) {
			_mmcam_dbg_warn("Wrong condition!!");
			return TRUE;
		}

		/* set size and timestamp */
		stream.length_total = GST_BUFFER_SIZE(buffer);
		stream.timestamp = (unsigned int)(GST_BUFFER_TIMESTAMP(buffer)/1000000); /* nano sec -> mili sec */

		/* set data pointers */
		if (stream.format == MM_PIXEL_FORMAT_NV12 ||
		    stream.format == MM_PIXEL_FORMAT_NV21 ||
		    stream.format == MM_PIXEL_FORMAT_I420) {
			if (hcamcorder->use_zero_copy_format && GST_BUFFER_MALLOCDATA(buffer)) {
				SCMN_IMGB *scmn_imgb = (SCMN_IMGB *)GST_BUFFER_MALLOCDATA(buffer);

				if (stream.format == MM_PIXEL_FORMAT_NV12 ||
				    stream.format == MM_PIXEL_FORMAT_NV21) {
					stream.data_type = MM_CAM_STREAM_DATA_YUV420SP;
					stream.num_planes = 2;
					stream.data.yuv420sp.y = scmn_imgb->a[0];
					stream.data.yuv420sp.length_y = stream.width * stream.height;
					stream.data.yuv420sp.uv = scmn_imgb->a[1];
					stream.data.yuv420sp.length_uv = stream.data.yuv420sp.length_y >> 1;
					/*
					_mmcam_dbg_log("format[%d][num_planes:%d] [Y]p:0x%x,size:%d [UV]p:0x%x,size:%d",
					               stream.format, stream.num_planes,
					               stream.data.yuv420sp.y, stream.data.yuv420sp.length_y,
					               stream.data.yuv420sp.uv, stream.data.yuv420sp.length_uv);
					*/
				} else {
					stream.data_type = MM_CAM_STREAM_DATA_YUV420P;
					stream.num_planes = 3;
					stream.data.yuv420p.y = scmn_imgb->a[0];
					stream.data.yuv420p.length_y = stream.width * stream.height;
					stream.data.yuv420p.u = scmn_imgb->a[1];
					stream.data.yuv420p.length_u = stream.data.yuv420p.length_y >> 2;
					stream.data.yuv420p.v = scmn_imgb->a[2];
					stream.data.yuv420p.length_v = stream.data.yuv420p.length_u;
					/*
					_mmcam_dbg_log("S420[num_planes:%d] [Y]p:0x%x,size:%d [U]p:0x%x,size:%d [V]p:0x%x,size:%d",
					                stream.num_planes,
					                stream.data.yuv420p.y, stream.data.yuv420p.length_y,
					                stream.data.yuv420p.u, stream.data.yuv420p.length_u,
					                stream.data.yuv420p.v, stream.data.yuv420p.length_v);
					*/
				}
			} else {
				if (stream.format == MM_PIXEL_FORMAT_NV12 ||
				    stream.format == MM_PIXEL_FORMAT_NV21) {
					stream.data_type = MM_CAM_STREAM_DATA_YUV420SP;
					stream.num_planes = 2;
					stream.data.yuv420sp.y = GST_BUFFER_DATA(buffer);
					stream.data.yuv420sp.length_y = stream.width * stream.height;
					stream.data.yuv420sp.uv = stream.data.yuv420sp.y + stream.data.yuv420sp.length_y;
					stream.data.yuv420sp.length_uv = stream.data.yuv420sp.length_y >> 1;
					/*
					_mmcam_dbg_log("format[%d][num_planes:%d] [Y]p:0x%x,size:%d [UV]p:0x%x,size:%d",
					               stream.format, stream.num_planes,
					               stream.data.yuv420sp.y, stream.data.yuv420sp.length_y,
					               stream.data.yuv420sp.uv, stream.data.yuv420sp.length_uv);
					*/
				} else {
					stream.data_type = MM_CAM_STREAM_DATA_YUV420P;
					stream.num_planes = 3;
					stream.data.yuv420p.y = GST_BUFFER_DATA(buffer);
					stream.data.yuv420p.length_y = stream.width * stream.height;
					stream.data.yuv420p.u = stream.data.yuv420p.y + stream.data.yuv420p.length_y;
					stream.data.yuv420p.length_u = stream.data.yuv420p.length_y >> 2;
					stream.data.yuv420p.v = stream.data.yuv420p.u + stream.data.yuv420p.length_u;
					stream.data.yuv420p.length_v = stream.data.yuv420p.length_u;
					/*
					_mmcam_dbg_log("I420[num_planes:%d] [Y]p:0x%x,size:%d [U]p:0x%x,size:%d [V]p:0x%x,size:%d",
					                stream.num_planes,
					                stream.data.yuv420p.y, stream.data.yuv420p.length_y,
					                stream.data.yuv420p.u, stream.data.yuv420p.length_u,
					                stream.data.yuv420p.v, stream.data.yuv420p.length_v);
					*/
				}
			}
		} else {
			if (stream.format == MM_PIXEL_FORMAT_YUYV ||
			    stream.format == MM_PIXEL_FORMAT_UYVY ||
			    stream.format == MM_PIXEL_FORMAT_422P ||
			    stream.format == MM_PIXEL_FORMAT_ITLV_JPEG_UYVY) {
				stream.data_type = MM_CAM_STREAM_DATA_YUV422;
				stream.data.yuv422.yuv = GST_BUFFER_DATA(buffer);
				stream.data.yuv422.length_yuv = stream.length_total;
			} else {
				stream.data_type = MM_CAM_STREAM_DATA_YUV420;
				stream.data.yuv420.yuv = GST_BUFFER_DATA(buffer);
				stream.data.yuv420.length_yuv = stream.length_total;
			}

			stream.num_planes = 1;
			/*
			_mmcam_dbg_log("%c%c%c%c[num_planes:%d] [0]p:0x%x,size:%d",
			               fourcc, fourcc>>8, fourcc>>16, fourcc>>24,
			               stream.num_planes, stream.data.yuv420.yuv, stream.data.yuv420.length_yuv);
			*/
		}

		/* call application callback */
		_MMCAMCORDER_LOCK_VSTREAM_CALLBACK(hcamcorder);
		if (hcamcorder->vstream_cb) {
			hcamcorder->vstream_cb(&stream, hcamcorder->vstream_cb_param);
		}
		_MMCAMCORDER_UNLOCK_VSTREAM_CALLBACK(hcamcorder);
	}

	return TRUE;
}


static gboolean __mmcamcorder_video_dataprobe_push_buffer_to_record(GstPad *pad, GstBuffer *buffer, gpointer u_data)
{
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(u_data);
	_MMCamcorderSubContext *sc = NULL;
	unsigned int diff = 0;           /* msec */

	mmf_return_val_if_fail(buffer, FALSE);
	mmf_return_val_if_fail(GST_BUFFER_DATA(buffer), FALSE);
	mmf_return_val_if_fail(hcamcorder, TRUE);

	sc = MMF_CAMCORDER_SUBCONTEXT(u_data);
	mmf_return_val_if_fail(sc, TRUE);

	/* push buffer in appsrc to encode */
	if (sc->info_video->push_encoding_buffer == PUSH_ENCODING_BUFFER_RUN &&
	    sc->info_video->record_dual_stream == FALSE &&
	    sc->encode_element[_MMCAMCORDER_ENCSINK_SRC].gst) {
		int ret = 0;
		GstClock *clock = NULL;

		if (sc->encode_element[_MMCAMCORDER_AUDIOSRC_SRC].gst) {
			if (sc->info_video->is_firstframe) {
				sc->info_video->is_firstframe = FALSE;
				clock = GST_ELEMENT_CLOCK(sc->encode_element[_MMCAMCORDER_AUDIOSRC_SRC].gst);
				if (clock) {
					gst_object_ref(clock);
					sc->info_video->base_video_ts = GST_BUFFER_TIMESTAMP(buffer) - (gst_clock_get_time(clock) - GST_ELEMENT(sc->encode_element[_MMCAMCORDER_ENCSINK_SRC].gst)->base_time);
					gst_object_unref(clock);
				}
			}
		} else {
			if(sc->info_video->is_firstframe) {
				sc->info_video->is_firstframe = FALSE;
				sc->info_video->base_video_ts = GST_BUFFER_TIMESTAMP(buffer);
			}
		}
		GST_BUFFER_TIMESTAMP(buffer) = GST_BUFFER_TIMESTAMP (buffer) - sc->info_video->base_video_ts;

		g_signal_emit_by_name(sc->encode_element[_MMCAMCORDER_ENCSINK_SRC].gst, "push-buffer", buffer, &ret);

		/*_mmcam_dbg_log("push buffer result : 0x%x", ret);*/
	}

	/* skip display if too fast FPS */
	if (sc->info_video && sc->info_video->fps > _MMCAMCORDER_FRAME_PASS_MIN_FPS) {
		if (sc->info_video->prev_preview_ts != 0) {
			diff = GST_BUFFER_TIMESTAMP(buffer) - sc->info_video->prev_preview_ts;
			if (diff < _MMCAMCORDER_MIN_TIME_TO_PASS_FRAME) {
				return FALSE;
			}
		}

		/*_mmcam_dbg_log("diff %u", diff);*/

		sc->info_video->prev_preview_ts = GST_BUFFER_TIMESTAMP(buffer);
	}

	return TRUE;
}


static gboolean __mmcamcorder_dual_camera_buffer_check(mmf_camcorder_t *hcamcorder, GstBuffer *buffer, int index)
{
	int ret = TRUE;
	SCMN_IMGB *imgb = NULL;

	if (hcamcorder == NULL) {
		_mmcam_dbg_err("should not be reached here");
		return FALSE;
	}

	/* lock mutex */
	pthread_mutex_lock(&hcamcorder->dual_buffer_lock);

	if (hcamcorder->dual_buffer_count == 1) {
		/* other buffer was already come. send signal */
		hcamcorder->dual_buffer_count = 0;
		pthread_cond_signal(&hcamcorder->dual_buffer_cond);
	} else {
		/* no buffer was come. wait other buffer */
		struct timespec timeout;
		struct timeval tv;

		gettimeofday(&tv, NULL);
		timeout.tv_sec = tv.tv_sec;
		timeout.tv_nsec = tv.tv_usec * _MMCAMCORDER_NANOSEC_PER_1MILISEC + _MMCAMCORDER_DUAL_CAMERA_BUFFER_WAIT_TIME;
		if (timeout.tv_nsec >= _MMCAMCORDER_NANOSEC_PER_1SEC) {
			timeout.tv_sec = timeout.tv_sec + 1;
			timeout.tv_nsec = timeout.tv_nsec - _MMCAMCORDER_NANOSEC_PER_1SEC;
		}

		hcamcorder->dual_buffer_count = 1;

		if (pthread_cond_timedwait(&(hcamcorder->dual_buffer_cond), &(hcamcorder->dual_buffer_lock), &timeout)) {
			_mmcam_dbg_warn("time out - skip buffer");
			hcamcorder->dual_buffer_count = 0;
			ret = FALSE;
		}
	}

	if (ret) {
		imgb = GST_BUFFER_MALLOCDATA(buffer);

		/* restore jpeg_data - hcamcorder->jpeg_data will be freed when gst buffer is finalized */
		if (imgb && hcamcorder->jpeg_data[index]) {
			imgb->jpeg_data = hcamcorder->jpeg_data[index];
			imgb->jpeg_size = hcamcorder->jpeg_size[index];
			hcamcorder->jpeg_data[index] = NULL;
			hcamcorder->jpeg_size[index] = 0;

			_mmcam_dbg_warn("restore jpeg_data %p, size %d",
			                imgb->jpeg_data, imgb->jpeg_size);
		}
	} else {
		imgb = GST_BUFFER_MALLOCDATA(buffer);

		/* check jpeg_data for dual camera capture */
		if (imgb && imgb->jpeg_data && imgb->jpeg_size > 0) {
			if (hcamcorder->jpeg_data[index]) {
				free(hcamcorder->jpeg_data[index]);
				hcamcorder->jpeg_data[index] = NULL;
			}

			hcamcorder->jpeg_data[index] = imgb->jpeg_data;
			hcamcorder->jpeg_size[index] = imgb->jpeg_size;
			imgb->jpeg_data = NULL;
			imgb->jpeg_size = 0;

			_mmcam_dbg_warn("keep temporary jpeg_data %p, size %d",
			                hcamcorder->jpeg_data[index], hcamcorder->jpeg_size[index]);
		}
	}

	pthread_mutex_unlock(&hcamcorder->dual_buffer_lock);

	return ret;
}


static void __mmcamcorder_videoframe_render_error_cb(GstElement *element, void *error_id, gpointer u_data)
{
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(u_data);

	mmf_return_if_fail(hcamcorder);

	if (error_id) {
		_mmcam_dbg_err("render error id 0x%x", *(int *)error_id);
	} else {
		_mmcam_dbg_err("error_id is NULL");
	}

	if (hcamcorder->render_error_cb) {
		_mmcam_dbg_log("call render error callback");
		hcamcorder->render_error_cb((unsigned int *)error_id, hcamcorder->render_error_cb_user_param);
	} else {
		_mmcam_dbg_log("render error callback is NULL");
	}

	return;
}


int __mmcamcorder_get_amrnb_bitrate_mode(int bitrate)
{
	int result = MM_CAMCORDER_MR475;

	if (bitrate< 5150) {
		result = MM_CAMCORDER_MR475; /*AMR475*/
	} else if (bitrate< 5900) {
		result = MM_CAMCORDER_MR515; /*AMR515*/
	} else if (bitrate < 6700) {
		result = MM_CAMCORDER_MR59; /*AMR59*/
	} else if (bitrate< 7400) {
		result = MM_CAMCORDER_MR67; /*AMR67*/
	} else if (bitrate< 7950) {
		result = MM_CAMCORDER_MR74; /*AMR74*/
	} else if (bitrate < 10200) {
		result = MM_CAMCORDER_MR795; /*AMR795*/
	} else if (bitrate < 12200) {
		result = MM_CAMCORDER_MR102; /*AMR102*/
	} else {
		result = MM_CAMCORDER_MR122; /*AMR122*/
	}

	return result;
}

int _mmcamcorder_get_eos_message(MMHandleType handle)
{
	double elapsed = 0.0;

	GstMessage *gMessage = NULL;
	GstBus *bus = NULL;
	GstClockTime timeout = 1 * GST_SECOND; /* maximum waiting time */
	GTimer *timer = NULL;

	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;

	mmf_return_val_if_fail(hcamcorder, MM_ERROR_CAMCORDER_NOT_INITIALIZED);
	sc = MMF_CAMCORDER_SUBCONTEXT(handle);

	_mmcam_dbg_log("");

	bus = gst_pipeline_get_bus(GST_PIPELINE(sc->encode_element[_MMCAMCORDER_ENCODE_MAIN_PIPE].gst));
	timer = g_timer_new();

	if (sc && !(sc->bget_eos)) {
		while (TRUE) {
			elapsed = g_timer_elapsed(timer, NULL);

			/*_mmcam_dbg_log("elapsed:%f sec", elapsed);*/

			if (elapsed > _MMCAMCORDER_WAIT_EOS_TIME) {
				_mmcam_dbg_warn("Timeout. EOS isn't received.");
				 g_timer_destroy(timer);
				 gst_object_unref(bus);
				 return MM_ERROR_CAMCORDER_RESPONSE_TIMEOUT;
			}

			gMessage = gst_bus_timed_pop(bus, timeout);
			if (gMessage != NULL) {
				_mmcam_dbg_log("Get message(%x).", GST_MESSAGE_TYPE(gMessage));
				_mmcamcorder_pipeline_cb_message(bus, gMessage, (void*)hcamcorder);

				if (GST_MESSAGE_TYPE(gMessage) == GST_MESSAGE_EOS || sc->bget_eos) {
					gst_message_unref(gMessage);
					break;
				}
				gst_message_unref(gMessage);
			} else {
				_mmcam_dbg_log("timeout of gst_bus_timed_pop()");
				if (sc->bget_eos) {
					_mmcam_dbg_log("Get EOS in another thread.");
					break;
				}
			}
		}
	}

	g_timer_destroy(timer);
	timer = NULL;
	gst_object_unref(bus);
	bus = NULL;

	_mmcam_dbg_log("END");

	return MM_ERROR_NONE;
}


void _mmcamcorder_remove_element_handle(MMHandleType handle, void *element, int first_elem, int last_elem)
{
	int i = 0;
	_MMCamcorderGstElement *remove_element = (_MMCamcorderGstElement *)element;

	mmf_return_if_fail(handle && remove_element);
	mmf_return_if_fail((first_elem >= 0) && (last_elem > 0) && (last_elem > first_elem));

	_mmcam_dbg_log("");

	for (i = first_elem ; i <= last_elem ; i++) {
		remove_element[i].gst = NULL;
		remove_element[i].id = _MMCAMCORDER_NONE;
	}

	return;
}


int _mmcamcorder_check_audiocodec_fileformat_compatibility(MMHandleType handle)
{
	int err = MM_ERROR_NONE;
	int audio_codec = MM_AUDIO_CODEC_INVALID;
	int file_format = MM_FILE_FORMAT_INVALID;

	char *err_name = NULL;

	err = mm_camcorder_get_attributes(handle, &err_name,
	                                  MMCAM_AUDIO_ENCODER, &audio_codec,
	                                  MMCAM_FILE_FORMAT, &file_format,
	                                  NULL);
	if (err != MM_ERROR_NONE) {
		_mmcam_dbg_warn("Get attrs fail. (%s:%x)", err_name, err);
		SAFE_FREE(err_name);
		return err;
	}

	/* Check compatibility between audio codec and file format */
	if (audio_codec >= MM_AUDIO_CODEC_INVALID && audio_codec < MM_AUDIO_CODEC_NUM &&
	    file_format >= MM_FILE_FORMAT_INVALID && file_format < MM_FILE_FORMAT_NUM) {
		if (audiocodec_fileformat_compatibility_table[audio_codec][file_format] == 0) {
			_mmcam_dbg_err("Audio codec[%d] and file format[%d] compatibility FAILED.",
			               audio_codec, file_format);
			return MM_ERROR_CAMCORDER_ENCODER_WRONG_TYPE;
		}

		_mmcam_dbg_log("Audio codec[%d] and file format[%d] compatibility SUCCESS.",
		               audio_codec, file_format);
	} else {
		_mmcam_dbg_err("Audio codec[%d] or file format[%d] is INVALID.",
		               audio_codec, file_format);
		return MM_ERROR_CAMCORDER_ENCODER_WRONG_TYPE;
	}

	return MM_ERROR_NONE;
}


int _mmcamcorder_check_videocodec_fileformat_compatibility(MMHandleType handle)
{
	int err = MM_ERROR_NONE;
	int video_codec = MM_VIDEO_CODEC_INVALID;
	int file_format = MM_FILE_FORMAT_INVALID;

	char *err_name = NULL;

	err = mm_camcorder_get_attributes(handle, &err_name,
	                                  MMCAM_VIDEO_ENCODER, &video_codec,
	                                  MMCAM_FILE_FORMAT, &file_format,
	                                  NULL);
	if (err != MM_ERROR_NONE) {
		_mmcam_dbg_warn("Get attrs fail. (%s:%x)", err_name, err);
		SAFE_FREE(err_name);
		return err;
	}

	/* Check compatibility between audio codec and file format */
	if (video_codec >= MM_VIDEO_CODEC_INVALID && video_codec < MM_VIDEO_CODEC_NUM &&
	    file_format >= MM_FILE_FORMAT_INVALID && file_format < MM_FILE_FORMAT_NUM) {
		if (videocodec_fileformat_compatibility_table[video_codec][file_format] == 0) {
			_mmcam_dbg_err("Video codec[%d] and file format[%d] compatibility FAILED.",
			               video_codec, file_format);
			return MM_ERROR_CAMCORDER_ENCODER_WRONG_TYPE;
		}

		_mmcam_dbg_log("Video codec[%d] and file format[%d] compatibility SUCCESS.",
		               video_codec, file_format);
	} else {
		_mmcam_dbg_err("Video codec[%d] or file format[%d] is INVALID.",
		               video_codec, file_format);
		return MM_ERROR_CAMCORDER_ENCODER_WRONG_TYPE;
	}

	return MM_ERROR_NONE;
}


bool _mmcamcorder_set_display_rotation(MMHandleType handle, int display_rotate)
{
	char* videosink_name = NULL;

	mmf_camcorder_t *hcamcorder = NULL;
	_MMCamcorderSubContext *sc = NULL;

	hcamcorder = MMF_CAMCORDER(handle);
	mmf_return_val_if_fail(hcamcorder, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_CAMCORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->element, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	if (sc->element[_MMCAMCORDER_VIDEOSINK_SINK].gst) {
		/* Get videosink name */
		_mmcamcorder_conf_get_value_element_name(sc->VideosinkElement, &videosink_name);
		if (!strcmp(videosink_name, "xvimagesink") ||
		    !strcmp(videosink_name, "evaspixmapsink")) {
			MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSINK_SINK].gst,
			                         "rotate", display_rotate);
			_mmcam_dbg_log("Set display-rotate [%d] done.", display_rotate);
			return TRUE;
		} else {
			_mmcam_dbg_warn("videosink[%s] does not support DISPLAY_ROTATION.", videosink_name);
			return FALSE;
		}
	} else {
		_mmcam_dbg_err("Videosink element is null");
		return FALSE;
	}
}


bool _mmcamcorder_set_display_flip(MMHandleType handle, int display_flip)
{
	char* videosink_name = NULL;

	mmf_camcorder_t *hcamcorder = NULL;
	_MMCamcorderSubContext *sc = NULL;

	hcamcorder = MMF_CAMCORDER(handle);
	mmf_return_val_if_fail(hcamcorder, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_CAMCORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->element, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	if (sc->element[_MMCAMCORDER_VIDEOSINK_SINK].gst) {
		/* Get videosink name */
		_mmcamcorder_conf_get_value_element_name(sc->VideosinkElement, &videosink_name);
		if (!strcmp(videosink_name, "xvimagesink") ||
		    !strcmp(videosink_name, "evaspixmapsink")) {
			MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSINK_SINK].gst,
			                         "flip", display_flip);
			_mmcam_dbg_log("Set display flip [%d] done.", display_flip);
			return TRUE;
		} else {
			_mmcam_dbg_warn("videosink[%s] does not support DISPLAY_FLIP", videosink_name);
			return FALSE;
		}
	} else {
		_mmcam_dbg_err("Videosink element is null");
		return FALSE;
	}
}


bool _mmcamcorder_set_videosrc_rotation(MMHandleType handle, int videosrc_rotate)
{
	int width = 0;
	int height = 0;
	int fps = 0;
	_MMCamcorderSubContext *sc = NULL;

	mmf_camcorder_t *hcamcorder = NULL;

	hcamcorder = MMF_CAMCORDER(handle);
	mmf_return_val_if_fail(hcamcorder, MM_ERROR_CAMCORDER_NOT_INITIALIZED);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	if (!sc) {
		_mmcam_dbg_log("sub context is not initailized");
		return TRUE;
	}

	mm_camcorder_get_attributes(handle, NULL,
	                            MMCAM_CAMERA_WIDTH, &width,
	                            MMCAM_CAMERA_HEIGHT, &height,
	                            MMCAM_CAMERA_FPS, &fps,
	                            NULL);

	_mmcam_dbg_log("set rotate %d", videosrc_rotate);

	return _mmcamcorder_set_videosrc_caps(handle, sc->fourcc, width, height, fps, videosrc_rotate);
}


bool _mmcamcorder_set_videosrc_caps(MMHandleType handle, unsigned int fourcc, int width, int height, int fps, int rotate)
{
	int set_width = 0;
	int set_height = 0;
	int set_rotate = 0;
	int fps_auto = 0;
	int image_filter_input_format = MM_PIXEL_FORMAT_I420;
	unsigned int caps_fourcc = 0;
	gboolean do_set_caps = FALSE;

	GstCaps *caps = NULL;

	mmf_camcorder_t *hcamcorder = NULL;
	_MMCamcorderSubContext *sc = NULL;

	hcamcorder = MMF_CAMCORDER(handle);
	mmf_return_val_if_fail(hcamcorder, FALSE);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);

	if (!sc || !(sc->element)) {
		_mmcam_dbg_log("sub context is not initialized");
		return TRUE;
	}

	if (!sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst) {
		_mmcam_dbg_err("Video src is NULL!");
		return FALSE;
	}

	if (!sc->element[_MMCAMCORDER_VIDEOSRC_FILT].gst) {
		_mmcam_dbg_err("Video filter is NULL!");
		return FALSE;
	}

	/* init set_hybrid_mode */
	hcamcorder->is_set_hybrid_mode = FALSE;

	if (hcamcorder->type != MM_CAMCORDER_MODE_AUDIO) {
		int capture_width = 0;
		int capture_height = 0;
		int video_width = 0;
		int video_height = 0;
		double motion_rate = _MMCAMCORDER_DEFAULT_RECORDING_MOTION_RATE;

		mm_camcorder_get_attributes(handle, NULL,
		                            MMCAM_CAMERA_RECORDING_MOTION_RATE, &motion_rate,
		                            MMCAM_CAMERA_FPS_AUTO, &fps_auto,
		                            MMCAM_CAPTURE_WIDTH, &capture_width,
		                            MMCAM_CAPTURE_HEIGHT, &capture_height,
		                            MMCAM_VIDEO_WIDTH, &video_width,
		                            MMCAM_VIDEO_HEIGHT, &video_height,
		                            NULL);
		_mmcam_dbg_log("motion rate %f, capture size %dx%d, fps auto %d",
		               motion_rate, capture_width, capture_height, fps_auto);

		if(motion_rate != _MMCAMCORDER_DEFAULT_RECORDING_MOTION_RATE) {
			MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "high-speed-fps", fps);
		} else {
			MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "high-speed-fps", 0);
		}

		MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "enable-hybrid-mode", hcamcorder->try_set_hybrid_mode);
		MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "capture-width", capture_width);
		MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "capture-height", capture_height);
		MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "fps-auto", fps_auto);

		/* set fps */
		sc->info_video->fps = fps;

		if (!hcamcorder->try_set_hybrid_mode) {
			/* assume that it's camera capture mode */
			MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSINK_SINK].gst, "csc-range", 1);
		} else {
			/* assume that it's video recording mode */
			MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSINK_SINK].gst, "csc-range", 0);

			/* set video size */
			MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "video-width", video_width);
			MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "video-height", video_height);
		}
	}

	/* Interleaved format does not support rotation */
	if (sc->info_image->preview_format != MM_PIXEL_FORMAT_ITLV_JPEG_UYVY) {
		/* store videosrc rotation */
		sc->videosrc_rotate = rotate;

		/* Define width, height and rotate in caps */

		/* This will be applied when rotate is 0, 90, 180, 270 if rear camera.
		This will be applied when rotate is 0, 180 if front camera. */
		set_rotate = rotate * 90;

		if (rotate == MM_VIDEO_INPUT_ROTATION_90 ||
		    rotate == MM_VIDEO_INPUT_ROTATION_270) {
			set_width = height;
			set_height = width;
			if (hcamcorder->device_type == MM_VIDEO_DEVICE_CAMERA1) {
				if (rotate == MM_VIDEO_INPUT_ROTATION_90) {
					set_rotate = 270;
				} else {
					set_rotate = 90;
				}
			}
		} else {
			set_width = width;
			set_height = height;
		}
	} else {
		sc->videosrc_rotate = MM_VIDEO_INPUT_ROTATION_NONE;
		set_rotate = 0;
		set_width = width;
		set_height = height;

		_mmcam_dbg_warn("ITLV format doe snot support INPUT ROTATE. Ignore ROTATE[%d]", rotate);
	}

	MMCAMCORDER_G_OBJECT_GET(sc->element[_MMCAMCORDER_VIDEOSRC_FILT].gst, "caps", &caps);
	if (caps && !gst_caps_is_any(caps)) {
		GstStructure *structure = NULL;

		structure = gst_caps_get_structure(caps, 0);
		if (structure) {
			int caps_width = 0;
			int caps_height = 0;
			int caps_fps = 0;
			int caps_rotate = 0;

			gst_structure_get_fourcc(structure, "format", &caps_fourcc);
			gst_structure_get_int(structure, "width", &caps_width);
			gst_structure_get_int(structure, "height", &caps_height);
			gst_structure_get_int(structure, "fps", &caps_fps);
			gst_structure_get_int(structure, "rotate", &caps_rotate);

			if (set_width == caps_width && set_height == caps_height &&
			    fourcc == caps_fourcc && set_rotate == caps_rotate && fps == caps_fps) {
				_mmcam_dbg_log("No need to replace caps.");
			} else {
				_mmcam_dbg_log("current [%c%c%c%c %dx%d, fps %d, rot %d], new [%c%c%c%c %dx%d, fps %d, rot %d]",
				               caps_fourcc, caps_fourcc>>8, caps_fourcc>>16, caps_fourcc>>24,
				               caps_width, caps_height, caps_fps, caps_rotate,
				               fourcc, fourcc>>8, fourcc>>16, fourcc>>24,
				               set_width, set_height, fps, set_rotate);
				do_set_caps = TRUE;
			}
		} else {
			_mmcam_dbg_log("can not get structure of caps. set new one...");
			do_set_caps = TRUE;
		}
	} else {
		_mmcam_dbg_log("No caps. set new one...");
		do_set_caps = TRUE;
	}

	if (caps) {
		gst_caps_unref(caps);
		caps = NULL;
	}

	if (do_set_caps) {
		char *caps_string = NULL;

		if (fourcc == GST_MAKE_FOURCC('S','R','3','2')) {
			caps = gst_caps_new_simple("video/x-raw-rgb",
			                           "format", GST_TYPE_FOURCC, fourcc,
			                           "bpp", G_TYPE_INT, 32,
			                           "depth", G_TYPE_INT, 24,
			                           "endianness", G_TYPE_INT, 4321,
			                           "red_mask", G_TYPE_INT, 65280,
			                           "green_mask", G_TYPE_INT, 16711680,
			                           "blue_mask", G_TYPE_INT, -16777216,
			                           "width", G_TYPE_INT, set_width,
			                           "height", G_TYPE_INT, set_height,
			                           "framerate", GST_TYPE_FRACTION, fps, 1,
			                           "rotate", G_TYPE_INT, set_rotate,
			                           NULL);
		} else {
			caps = gst_caps_new_simple("video/x-raw-yuv",
			                           "format", GST_TYPE_FOURCC, fourcc,
			                           "width", G_TYPE_INT, set_width,
			                           "height", G_TYPE_INT, set_height,
			                           "framerate", GST_TYPE_FRACTION, fps, 1,
			                           "rotate", G_TYPE_INT, set_rotate,
			                           NULL);
		}

		caps_string = gst_caps_to_string(caps);
		if (caps_string) {
			_mmcam_dbg_log("caps [%s]", caps_string);
			free(caps_string);
			caps_string = NULL;
		}

		MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_FILT].gst, "caps", caps);
		gst_caps_unref(caps);
		caps = NULL;
		_mmcam_dbg_log("vidoesrc new caps set. [%c%c%c%c %dx%d, fps %d, rotate %d]",
		               fourcc, fourcc>>8, fourcc>>16, fourcc>>24,
		               set_width, set_height, fps, set_rotate);
	}

	return TRUE;
}


bool _mmcamcorder_set_videosrc_flip(MMHandleType handle, int videosrc_flip)
{
	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;

	mmf_return_val_if_fail(hcamcorder, FALSE);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, TRUE);

	_mmcam_dbg_log("Set FLIP %d", videosrc_flip);

	if (sc->element && sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst) {
		int hflip = 0;
		int vflip = 0;

		/* Interleaved format does not support FLIP */
		if (sc->info_image->preview_format != MM_PIXEL_FORMAT_ITLV_JPEG_UYVY) {
			hflip = (videosrc_flip & MM_FLIP_HORIZONTAL) == MM_FLIP_HORIZONTAL;
			vflip = (videosrc_flip & MM_FLIP_VERTICAL) == MM_FLIP_VERTICAL;

			_mmcam_dbg_log("videosrc flip H:%d, V:%d", hflip, vflip);

			MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "hflip", hflip);
			MMCAMCORDER_G_OBJECT_SET(sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst, "vflip", vflip);
		} else {
			_mmcam_dbg_warn("ITLV format does not support FLIP. Ignore FLIP[%d]",
			                videosrc_flip);
		}
	} else {
		_mmcam_dbg_warn("element is NULL");
		return FALSE;
	}

	return TRUE;
}


bool _mmcamcorder_set_videosrc_anti_shake(MMHandleType handle, int anti_shake)
{
	GstCameraControl *control = NULL;
	_MMCamcorderSubContext *sc = NULL;
	GstElement *v_src = NULL;

	int set_value = 0;

	mmf_return_val_if_fail(handle, FALSE);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, TRUE);

	v_src = sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst;

	if (!v_src) {
		_mmcam_dbg_warn("videosrc element is NULL");
		return FALSE;
	}

	set_value = _mmcamcorder_convert_msl_to_sensor(handle, MM_CAM_CAMERA_ANTI_HANDSHAKE, anti_shake);

	/* set anti-shake with camera control */
	if (!GST_IS_CAMERA_CONTROL(v_src)) {
		_mmcam_dbg_warn("Can't cast Video source into camera control.");
		return FALSE;
	}

	control = GST_CAMERA_CONTROL(v_src);
	if (gst_camera_control_set_ahs(control, set_value)) {
		_mmcam_dbg_log("Succeed in operating anti-handshake. value[%d]", set_value);
		return TRUE;
	} else {
		_mmcam_dbg_warn("Failed to operate anti-handshake. value[%d]", set_value);
	}

	return FALSE;
}


bool _mmcamcorder_set_videosrc_stabilization(MMHandleType handle, int stabilization)
{
	_MMCamcorderSubContext *sc = NULL;
	GstElement *v_src = NULL;

	mmf_return_val_if_fail(handle, FALSE);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, TRUE);

	v_src = sc->element[_MMCAMCORDER_VIDEOSRC_SRC].gst;
	if (!v_src) {
		_mmcam_dbg_warn("videosrc element is NULL");
		return FALSE;
	}

	/* check property of videosrc element - support VDIS */
	if(g_object_class_find_property(G_OBJECT_GET_CLASS(G_OBJECT(v_src)), "enable-vdis-mode")) {
		int camera_width = 0;
		int camera_height = 0;

		if (stabilization == MM_CAMCORDER_VIDEO_STABILIZATION_ON) {
			_mmcam_dbg_log("ENABLE video stabilization");

			/* VDIS mode only supports NV12 and [720p or 1080p] */
			mm_camcorder_get_attributes(handle, NULL,
			                            MMCAM_CAMERA_WIDTH, &camera_width,
			                            MMCAM_CAMERA_HEIGHT, &camera_height,
			                            NULL);
			if (sc->info_image->preview_format == MM_PIXEL_FORMAT_NV12 &&
			    camera_width >= 1280 && camera_height >= 720) {
				_mmcam_dbg_log("NV12, size %dx%d, ENABLE video stabilization",
				               camera_width, camera_height, stabilization);
				/* set vdis mode */
				g_object_set(G_OBJECT(v_src),
				             "enable-vdis-mode", TRUE,
				             NULL);
			} else {
				_mmcam_dbg_warn("invalid preview format %c%c%c%c or size %dx%d",
				                sc->fourcc, sc->fourcc>>8, sc->fourcc>>16, sc->fourcc>>24,
				                camera_width, camera_height);
				return FALSE;
			}
		} else {
			/* set vdis mode */
			g_object_set(G_OBJECT(v_src),
			             "enable-vdis-mode", FALSE,
			             NULL);

			_mmcam_dbg_log("DISABLE video stabilization");
		}
	} else if (stabilization == MM_CAMCORDER_VIDEO_STABILIZATION_ON) {
		_mmcam_dbg_err("no property for video stabilization, so can not set ON");
		return FALSE;
	} else {
		_mmcam_dbg_warn("no property for video stabilization");
	}

	return TRUE;
}


bool _mmcamcorder_set_camera_resolution(MMHandleType handle, int width, int height)
{
	int fps = 0;

	mmf_camcorder_t *hcamcorder = MMF_CAMCORDER(handle);
	_MMCamcorderSubContext *sc = NULL;

	mmf_return_val_if_fail(hcamcorder, FALSE);

	sc = MMF_CAMCORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, TRUE);

	mm_camcorder_get_attributes(handle, NULL,
	                            MMCAM_CAMERA_FPS, &fps,
	                            NULL);

	_mmcam_dbg_log("set %dx%d", width, height);

	return _mmcamcorder_set_videosrc_caps(handle, sc->fourcc, width, height, fps, sc->videosrc_rotate);
}
