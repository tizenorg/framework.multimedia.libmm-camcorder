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

#ifndef __MM_CAMCORDER_SOUND_H__
#define __MM_CAMCORDER_SOUND_H__

/*=======================================================================================
| INCLUDE FILES										|
========================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================================================================
| GLOBAL DEFINITIONS AND DECLARATIONS FOR CAMCORDER					|
========================================================================================*/

/*=======================================================================================
| MACRO DEFINITIONS									|
========================================================================================*/
#define _MMCAMCORDER_FILEPATH_CAPTURE_SND        "/usr/share/sounds/mm-camcorder/Shutter.ogg"
#define _MMCAMCORDER_FILEPATH_CAPTURE2_SND       "/usr/share/sounds/mm-camcorder/Camera_click_short.ogg"
#define _MMCAMCORDER_FILEPATH_CAPTURE_EXT_01_SND "/usr/share/sounds/mm-camcorder/Shutter_ext_01.ogg"
#define _MMCAMCORDER_FILEPATH_CAPTURE_EXT_02_SND "/usr/share/sounds/mm-camcorder/Shutter_ext_02.ogg"
#define _MMCAMCORDER_FILEPATH_REC_START_SND      "/usr/share/sounds/mm-camcorder/Cam_Start.ogg"
#define _MMCAMCORDER_FILEPATH_REC_STOP_SND       "/usr/share/sounds/mm-camcorder/Cam_Stop.ogg"
#define _MMCAMCORDER_FILEPATH_REC_PAUSE_SND      "/usr/share/sounds/mm-camcorder/Cam_pause.ogg"

/*=======================================================================================
| ENUM DEFINITIONS									|
========================================================================================*/

/*=======================================================================================
| STRUCTURE DEFINITIONS									|
========================================================================================*/

/*=======================================================================================
| CONSTANT DEFINITIONS									|
========================================================================================*/


/*=======================================================================================
| GLOBAL FUNCTION PROTOTYPES								|
========================================================================================*/
void _mmcamcorder_sound_solo_play(MMHandleType handle, const char *filepath, gboolean sync);

#ifdef __cplusplus
}
#endif

#endif /* __MM_CAMCORDER_SOUND_H__ */
