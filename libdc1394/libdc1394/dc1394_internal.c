/*
 * 1394-Based Digital Camera Control Library, internal functions
 * Copyright (C) Damien Douxchamps <ddouxchamps@users.sf.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "dc1394_internal.h"
#include "dc1394_utils.h"
#include "dc1394_register.h"

const char *dc1394_feature_desc[DC1394_FEATURE_NUM] =
{
  "Brightness",
  "Exposure",
  "Sharpness",
  "White Balance",
  "Hue",
  "Saturation",
  "Gamma",
  "Shutter",
  "Gain",
  "Iris",
  "Focus",
  "Temperature",
  "Trigger",
  "Trigger Delay",
  "White Shading",
  "Frame Rate",
  "Zoom",
  "Pan",
  "Tilt",
  "Optical Filter",
  "Capture Size",
  "Capture Quality"
};

const char *dc1394_error_strings[DC1394_ERROR_NUM] =
{
  "Success",
  "Generic failure",
  "No frame",
  "No camera",
  "This node is not a camera",
  "Function not supported by this camera",
  "Camera not initialized",
  "Invalid feature",
  "Invalid video format",
  "Invalid video mode",
  "Invalid framerate",
  "Invalid trigger mode",
  "Invalid trigger source",
  "Invalid ISO speed",
  "Invalid IIDC version",
  "Invalid Format_7 color coding",
  "Invalid Format_7 elementary Bayer tile",
  "Invalid capture mode",
  "Requested value is out of range",
  "Invalid error code",
  "Memory allocation failure",
  "Tagged register not found",
  "Format_7 Error_flag_1 is set",
  "Format_7 Error_flag_2 is set",
  "Invalid Bayer method",
  "Invalid argument value",
  "Invalid video1394 device",
  "Could not allocate an ISO channel",
  "Could not allocate bandwidth",
  "IOCTL failure",
  "Capture is not set",
  "RAW1394 capture failure",
  "Capture is running",
  "RAW1394 failure"
};

/*
  These arrays define how many image quadlets there
  are in a packet given a mode and a frame rate
  This is defined in the 1394 digital camera spec 
*/
const int quadlets_per_packet_format_0[56] = 
{
  -1,  -1,  15,  30,  60,  120,  240,  480,
  10,  20,  40,  80, 160,  320,  640, 1280,
  30,  60, 120, 240, 480,  960, 1920, 3840,
  40,  80, 160, 320, 640, 1280, 2560, 5120,
  60, 120, 240, 480, 960, 1920, 3840, 7680,
  20,  40,  80, 160, 320,  640, 1280, 2560,
  40,  80, 160, 320, 640, 1280, 2560, 5120
};
 
const int quadlets_per_packet_format_1[64] = 
{
   -1, 125, 250,  500, 1000, 2000, 4000, 8000,
   -1,  -1, 375,  750, 1500, 3000, 6000,   -1,
   -1,  -1, 125,  250,  500, 1000, 2000, 4000,
   96, 192, 384,  768, 1536, 3072, 6144,   -1,
  144, 288, 576, 1152, 2304, 4608,   -1,   -1,
   48,  96, 192,  384,  768, 1536, 3073, 6144,
   -1, 125, 250,  500, 1000, 2000, 4000, 8000,
   96, 192, 384,  768, 1536, 3072, 6144,   -1
};
 
const int quadlets_per_packet_format_2[64] = 
{
  160, 320,  640, 1280, 2560, 5120,   -1, -1,
  240, 480,  960, 1920, 3840, 7680,   -1, -1,
   80, 160,  320,  640, 1280, 2560, 5120, -1,
  250, 500, 1000, 2000, 4000, 8000,   -1, -1,
  375, 750, 1500, 3000, 6000,   -1,   -1, -1,
  125, 250,  500, 1000, 2000, 4000, 8000, -1, 
  160, 320,  640, 1280, 2560, 5120,   -1, -1,
  250, 500, 1000, 2000, 4000, 8000,   -1, -1
};

dc1394camera_t*
dc1394_new_camera(uint32_t port, nodeid_t node)
{
  dc1394camera_t *cam;
 
  cam=(dc1394camera_t *)malloc(sizeof(dc1394camera_t));
  cam = dc1394_new_camera_platform (port, node);
 
  if (cam==NULL)
    return NULL;
  
   cam->port=port;
   cam->node=node;
   cam->iso_channel_is_set=0;
   cam->iso_channel=-1;
   cam->iso_bandwidth=0;
   cam->capture_is_set=0;
   cam->broadcast=DC1394_FALSE;
   cam->absolute_control_csr=0;
   // default values for PHY stuff:
   cam->phy_speed=-1;
   cam->power_class=DC1394_POWER_CLASS_NONE;
   cam->phy_delay=DC1394_PHY_DELAY_MAX_144_NS;
 
   return cam;
}

/********************************************************
 _dc1394_get_quadlets_per_packet

 This routine reports the number of useful image quadlets 
 per packet
*********************************************************/
dc1394error_t
_dc1394_get_quadlets_per_packet(dc1394video_mode_t mode, dc1394framerate_t frame_rate, uint32_t *qpp) // ERROR handling to be updated
{
  uint32_t mode_index;
  uint32_t frame_rate_index= frame_rate - DC1394_FRAMERATE_MIN;
  uint32_t format;
  dc1394error_t err;

  err=_dc1394_get_format_from_mode(mode, &format);
  DC1394_ERR_RTN(err,"Invalid mode ID");
  
  //fprintf(stderr,"format: %d\n",format);

  switch(format) {
  case DC1394_FORMAT0:
    mode_index= mode - DC1394_VIDEO_MODE_FORMAT0_MIN;
    
    if ( ((mode >= DC1394_VIDEO_MODE_FORMAT0_MIN) && (mode <= DC1394_VIDEO_MODE_FORMAT0_MAX)) && 
	 ((frame_rate >= DC1394_FRAMERATE_MIN) && (frame_rate <= DC1394_FRAMERATE_MAX)) ) {
      *qpp=quadlets_per_packet_format_0[DC1394_FRAMERATE_NUM*mode_index+frame_rate_index];
    }
    else {
      err=DC1394_INVALID_VIDEO_MODE;
      DC1394_ERR_RTN(err,"Invalid framerate (%d) or mode (%d)", frame_rate, mode);
    }
    return DC1394_SUCCESS;
  case DC1394_FORMAT1:
    mode_index= mode - DC1394_VIDEO_MODE_FORMAT1_MIN;
    
    if ( ((mode >= DC1394_VIDEO_MODE_FORMAT1_MIN) && (mode <= DC1394_VIDEO_MODE_FORMAT1_MAX)) && 
	 ((frame_rate >= DC1394_FRAMERATE_MIN) && (frame_rate <= DC1394_FRAMERATE_MAX)) ) {
      *qpp=quadlets_per_packet_format_1[DC1394_FRAMERATE_NUM*mode_index+frame_rate_index];
    }
    else {
      err=DC1394_INVALID_VIDEO_MODE;
      DC1394_ERR_RTN(err,"Invalid framerate (%d) or mode (%d)", frame_rate, mode);
    }
    return DC1394_SUCCESS;
  case DC1394_FORMAT2:
    mode_index= mode - DC1394_VIDEO_MODE_FORMAT2_MIN;
    
    if ( ((mode >= DC1394_VIDEO_MODE_FORMAT2_MIN) && (mode <= DC1394_VIDEO_MODE_FORMAT2_MAX)) && 
	 ((frame_rate >= DC1394_FRAMERATE_MIN) && (frame_rate <= DC1394_FRAMERATE_MAX)) ) {
      *qpp=quadlets_per_packet_format_2[DC1394_FRAMERATE_NUM*mode_index+frame_rate_index];
    }
    else {
      err=DC1394_INVALID_VIDEO_MODE;
      DC1394_ERR_RTN(err,"Invalid framerate (%d) or mode (%d)", frame_rate, mode);
    }
    return DC1394_SUCCESS;
  case DC1394_FORMAT6:
  case DC1394_FORMAT7:
    err=DC1394_INVALID_VIDEO_FORMAT;
    DC1394_ERR_RTN(err,"Format 6 and 7 don't have qpp");
    break;
  }

  return DC1394_FAILURE;
}

/**********************************************************
 _dc1394_quadlets_from_format

 This routine reports the number of quadlets that make up a 
 frame given the format and mode
***********************************************************/
dc1394error_t
_dc1394_quadlets_from_format(dc1394camera_t *camera, dc1394video_mode_t video_mode, uint32_t *quads) 
{

  uint32_t w, h, color_coding;
  float bpp;
  dc1394error_t err;

  err=dc1394_get_image_size_from_video_mode(camera, video_mode, &w, &h);
  DC1394_ERR_RTN(err, "Invalid mode ID");

  err=dc1394_get_color_coding_from_video_mode(camera, video_mode, &color_coding);
  DC1394_ERR_RTN(err, "Invalid mode ID");

  err=dc1394_get_bytes_per_pixel(color_coding, &bpp);
  DC1394_ERR_RTN(err, "Invalid color mode ID");

  *quads=(uint32_t)(w*h*bpp/4);

  return err;
}

dc1394bool_t
IsFeatureBitSet(quadlet_t value, dc1394feature_t feature)
{

  if (feature >= DC1394_FEATURE_ZOOM) {
    if (feature >= DC1394_FEATURE_CAPTURE_SIZE) {
      feature+= 12;
    }
    feature-= DC1394_FEATURE_ZOOM;
  }
  else {
    feature-= DC1394_FEATURE_MIN;
  }
  
  value&=(0x80000000UL >> feature);
  
  if (value>0)
    return DC1394_TRUE;
  else
    return DC1394_FALSE;
}

dc1394error_t
_dc1394_get_format_from_mode(dc1394video_mode_t mode, uint32_t *format)
{
  dc1394error_t err=DC1394_SUCCESS;

  if ((mode>=DC1394_VIDEO_MODE_FORMAT0_MIN)&&(mode<=DC1394_VIDEO_MODE_FORMAT0_MAX)) {
    *format=DC1394_FORMAT0;
  }
  else if ((mode>=DC1394_VIDEO_MODE_FORMAT1_MIN)&&(mode<=DC1394_VIDEO_MODE_FORMAT1_MAX)) {
    *format=DC1394_FORMAT1;
  }
  else if ((mode>=DC1394_VIDEO_MODE_FORMAT2_MIN)&&(mode<=DC1394_VIDEO_MODE_FORMAT2_MAX)) {
    *format=DC1394_FORMAT2;
  }
  else if ((mode>=DC1394_VIDEO_MODE_FORMAT6_MIN)&&(mode<=DC1394_VIDEO_MODE_FORMAT6_MAX)) {
    *format=DC1394_FORMAT6;
  }
  else if ((mode>=DC1394_VIDEO_MODE_FORMAT7_MIN)&&(mode<=DC1394_VIDEO_MODE_FORMAT7_MAX)) {
    *format=DC1394_FORMAT7;
  }
  else {
    err=DC1394_INVALID_VIDEO_MODE;
    DC1394_ERR_RTN(err, "The supplied mode does not correspond to any format");
  }

  return err;
}

 
dc1394error_t
dc1394_video_set_iso_channel(dc1394camera_t *camera, uint32_t channel)
{
  dc1394error_t err;
  quadlet_t value_inq, value;
  int speed;

  err=GetCameraControlRegister(camera, REG_CAMERA_BASIC_FUNC_INQ, &value_inq);
  DC1394_ERR_RTN(err, "Could not get basic function register");

  err=GetCameraControlRegister(camera, REG_CAMERA_ISO_DATA, &value);
  DC1394_ERR_RTN(err, "Could not get ISO data");

  // check if 1394b is available and if we are now using 1394b
  if ((value_inq & 0x00800000)&&(value & 0x00008000)) {
    err=GetCameraControlRegister(camera, REG_CAMERA_ISO_DATA, &value);
    DC1394_ERR_RTN(err, "oops");
    speed=value & 0x7UL;
    err=SetCameraControlRegister(camera, REG_CAMERA_ISO_DATA,
				 (quadlet_t) ( ((channel & 0x3FUL) << 8) |
					       (speed & 0x7UL) |
					       (0x1 << 15) ));
    DC1394_ERR_RTN(err, "oops");
  }
  else { // fallback to legacy
    err=GetCameraControlRegister(camera, REG_CAMERA_ISO_DATA, &value);
    DC1394_ERR_RTN(err, "oops");
    speed=(value >> 24) & 0x3UL;
    if (speed>DC1394_ISO_SPEED_400-DC1394_ISO_SPEED_MIN) {
      fprintf(stderr,"(%s) line %d: an ISO speed >400Mbps was requested while the camera is in LEGACY mode\n",__FILE__,__LINE__);
      fprintf(stderr,"              Please set the operation mode to OPERATION_MODE_1394B before asking for\n");
      fprintf(stderr,"              1394b ISO speeds\n");
      return DC1394_FAILURE;
    }
    err=SetCameraControlRegister(camera, REG_CAMERA_ISO_DATA,
				 (quadlet_t) (((channel & 0xFUL) << 28) |
					      ((speed & 0x3UL) << 24) ));
    DC1394_ERR_RTN(err, "Could not set ISO data register");
  }
  
  return err;
}

// not used yet. example of thing we need to do for checking IIDC versions.
// problem: what about Point Grey cameras ??
/*
dc1394bool_t
_dc1394_iidc_check_video_mode(dc1394camera_t *camera, dc1394video_mode_t *mode)
{
  dc1394iidc_version_t min_version;
  switch (mode) {
  case DC1394_VIDEO_MODE_160x120_YUV444:
  case DC1394_VIDEO_MODE_320x240_YUV422:
  case DC1394_VIDEO_MODE_640x480_YUV411:
  case DC1394_VIDEO_MODE_640x480_YUV422:
  case DC1394_VIDEO_MODE_640x480_RGB8:
  case DC1394_VIDEO_MODE_640x480_MONO8:
    min_version=DC1394_IIDC_VERSION_1_04;
    break;
  case DC1394_VIDEO_MODE_640x480_MONO16:
    min_version=DC1394_IIDC_VERSION_1_30;
    break;
  case DC1394_VIDEO_MODE_800x600_YUV422:
  case DC1394_VIDEO_MODE_800x600_RGB8:
  case DC1394_VIDEO_MODE_800x600_MONO8:
  case DC1394_VIDEO_MODE_1024x768_YUV422:
  case DC1394_VIDEO_MODE_1024x768_RGB8:
  case DC1394_VIDEO_MODE_1024x768_MONO8:
    min_version=DC1394_IIDC_VERSION_1_20;
    break;
  case DC1394_VIDEO_MODE_800x600_MONO16:
  case DC1394_VIDEO_MODE_1024x768_MONO16:
    min_version=DC1394_IIDC_VERSION_1_30;
    break;
  case DC1394_VIDEO_MODE_1280x960_YUV422:
  case DC1394_VIDEO_MODE_1280x960_RGB8:
  case DC1394_VIDEO_MODE_1280x960_MONO8:
  case DC1394_VIDEO_MODE_1600x1200_YUV422:
  case DC1394_VIDEO_MODE_1600x1200_RGB8:
  case DC1394_VIDEO_MODE_1600x1200_MONO8:
    min_version=DC1394_IIDC_VERSION_1_20;
    break;
  case DC1394_VIDEO_MODE_1280x960_MONO16:
  case DC1394_VIDEO_MODE_1600x1200_MONO16:
    min_version=DC1394_IIDC_VERSION_1_30;
    break;
  case DC1394_VIDEO_MODE_EXIF:
  case DC1394_VIDEO_MODE_FORMAT7_0:
  case DC1394_VIDEO_MODE_FORMAT7_1:
  case DC1394_VIDEO_MODE_FORMAT7_2:
  case DC1394_VIDEO_MODE_FORMAT7_3:
  case DC1394_VIDEO_MODE_FORMAT7_4:
  case DC1394_VIDEO_MODE_FORMAT7_5:
  case DC1394_VIDEO_MODE_FORMAT7_6:
  case DC1394_VIDEO_MODE_FORMAT7_7:
    min_version=DC1394_IIDC_VERSION_1_20;
    break;
  }

  if (min_version>=camera->iidc_version)
    return DC1394_TRUE;
  else
    return DC1394_FALSE;

}
*/
