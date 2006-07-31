/*
 * 1394-Based Digital Camera Control Library
 * Copyright (C) 2000 SMART Technologies Inc.
 *
 * Written by Gord Peters <GordPeters@smarttech.com>
 * Additions by Chris Urmson <curmson@ri.cmu.edu>
 * Additions by Damien Douxchamps <ddouxchamps@users.sf.net>
 * Additions by David Moore <dcm@acm.org>
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
#ifndef __DC1394_MACOSX_H__
#define __DC1394_MACOSX_H__

#include "dc1394_control.h"
#include "dc1394_capture_macosx.h"
#include <IOKit/firewire/IOFireWireLib.h>

typedef enum {
  BUFFER_EMPTY = 0,
  BUFFER_FILLED = 1,
} buffer_status;

typedef struct _buffer_info {
  dc1394camera_t * camera;
  int              i;
  buffer_status    status;
  NuDCLRef         dcl;
  NuDCLRef         dcl2;
  struct timeval   filltime;
} buffer_info;

typedef struct __dc1394_capture
{
  unsigned int             frame_width;
  unsigned int             frame_height;
  unsigned int             quadlets_per_frame;
  unsigned int             quadlets_per_packet;
  unsigned int             num_frames;
  int                      frame_pages;
  unsigned int             current;
  /* components needed for the DMA based video capture */
  dc1394ring_buffer_policy_t      ring_buffer_policy;
  IOFireWireLibIsochChannelRef    chan;
  IOFireWireLibRemoteIsochPortRef rem_port;
  IOFireWireLibLocalIsochPortRef  loc_port;
  IOFireWireLibNuDCLPoolRef       dcl_pool;
  IOVirtualRange           databuf;
  buffer_info *            buffers;
  CFRunLoopRef             run_loop;
  CFStringRef              run_loop_mode;
  dc1394capture_callback_t callback;
  void *                   callback_user_data;
} dc1394capture_t;

typedef struct __dc1394_camera_macosx
{
  dc1394camera_t          camera;
  IOFireWireLibDeviceRef  iface;
  UInt32                  generation;
  dc1394capture_t         capture;
} dc1394camera_macosx_t;

#endif
