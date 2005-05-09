/*
 * 1394-Based Digital Camera Control Library
 * Color conversion functions, including Bayer pattern decoding
 * Copyright (C) Damien Douxchamps <ddouxchamps@users.sf.net>
 *
 * Written by Damien Douxchamps and Frederic Devernay
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

#include "dc1394_conversions.h"

// this should disappear...
extern void swab();

/**********************************************************************
 *
 *  CONVERSION FUNCTIONS TO UYVY 
 *
 **********************************************************************/

void
dc1394_YUV422_to_YUV422(uchar_t *src, uchar_t *dest, uint64_t NumPixels, uint_t byte_order)
{
  switch (byte_order) {
  case DC1394_BYTE_ORDER_YUYV:
    swab(src, dest, NumPixels << 1);
    break;
  case DC1394_BYTE_ORDER_UYVY:
    memcpy(dest,src, NumPixels<<1);
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }
}

void
dc1394_YUV411_to_YUV422(uchar_t *src, uchar_t *dest, uint64_t NumPixels, uint_t byte_order)
{
  register int i=NumPixels + (NumPixels >> 1)-1;
  register int j=(NumPixels << 1)-1;
  register int y0, y1, y2, y3, u, v;

  switch (byte_order) {
  case DC1394_BYTE_ORDER_YUYV:
    while (i >= 0) {
      y3 = src[i--];
      y2 = src[i--];
      v  = src[i--];
      y1 = src[i--];
      y0 = src[i--];
      u  = src[i--];

      dest[j--] = v;
      dest[j--] = y3;
      dest[j--] = u;
      dest[j--] = y2;
      
      dest[j--] = v;
      dest[j--] = y1;
      dest[j--] = u;
      dest[j--] = y0;
    }
    break;
  case DC1394_BYTE_ORDER_UYVY:
    while (i >= 0) {
      y3 = src[i--];
      y2 = src[i--];
      v  = src[i--];
      y1 = src[i--];
      y0 = src[i--];
      u  = src[i--];

      dest[j--] = y3;
      dest[j--] = v;
      dest[j--] = y2;
      dest[j--] = u;
      
      dest[j--] = y1;
      dest[j--] = v;
      dest[j--] = y0;
      dest[j--] = u;
    }
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }

}

void
dc1394_YUV444_to_YUV422(uchar_t *src, uchar_t *dest, uint64_t NumPixels, uint_t byte_order)
{
  register int i = NumPixels + (NumPixels << 1)-1;
  register int j = (NumPixels << 1)-1;
  register int y0, y1, u0, u1, v0, v1;

  switch (byte_order) {
  case DC1394_BYTE_ORDER_YUYV:
    while (i >= 0) {
      v1 = src[i--];
      y1 = src[i--];
      u1 = src[i--];
      v0 = src[i--];
      y0 = src[i--];
      u0 = src[i--];

      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y1;
      dest[j--] = (u0+u1) >> 1;
      dest[j--] = y0;
    }
    break;
  case DC1394_BYTE_ORDER_UYVY:
    while (i >= 0) {
      v1 = src[i--];
      y1 = src[i--];
      u1 = src[i--];
      v0 = src[i--];
      y0 = src[i--];
      u0 = src[i--];

      dest[j--] = y1;
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y0;
      dest[j--] = (u0+u1) >> 1;
    }
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }
}

void
dc1394_MONO8_to_YUV422(uchar_t *src, uchar_t *dest, 
		       uint_t src_width, uint_t src_height,
		       uint_t dest_pitch, uint_t byte_order)
{
  if ((src_width*2) == dest_pitch) {
    // do it the quick way
    register int i = src_width*src_height - 1;
    register int j = (src_width*src_height << 1) - 1;
    register int y0, y1;
    
    switch (byte_order) {
    case DC1394_BYTE_ORDER_YUYV:
      while (i >= 0) {
	y1 = src[i--];
	y0 = src[i--];
	dest[j--] = 128;
	dest[j--] = y1;
	dest[j--] = 128;
	dest[j--] = y0;
      }
      break;
    case DC1394_BYTE_ORDER_UYVY:
      while (i >= 0) {
	y1 = src[i--];
	y0 = src[i--];
	dest[j--] = y1;
	dest[j--] = 128;
	dest[j--] = y0;
	dest[j--] = 128;
      }
      break;
    default:
      fprintf(stderr,"Invalid overlay byte order\n");
      break;
    }
  } else { // src_width*2 != dest_pitch
    register int x, y;

    //assert ((dest_pitch - 2*src_width)==1);

    switch (byte_order) {
    case DC1394_BYTE_ORDER_YUYV:
      y=src_height;
      while (y--) {
	x=src_width;
	while (x--) {
	  *dest++ = *src++;
	  *dest++ = 128;
	}
	// padding required, duplicate last column
	*dest++ = *(src-1);
	*dest++ = 128;
      }
      break;
    case DC1394_BYTE_ORDER_UYVY:
      y=src_height;
      while (y--) {
	x=src_width;
	while (x--) {
	  *dest++ = 128;
	  *dest++ = *src++;
	}
	// padding required, duplicate last column
	*dest++ = 128;
	*dest++ = *(src-1);
      }
      break;
    default:
      fprintf(stderr,"Invalid overlay byte order\n");
      break;
    }
  }
}

void
dc1394_MONO16_to_YUV422(uchar_t *src, uchar_t *dest, uint64_t NumPixels, uint_t bits, uint_t byte_order)
{
  register int i = (NumPixels << 1)-1;
  register int j = (NumPixels << 1)-1;
  register int y0, y1;

  switch (byte_order) {
  case DC1394_BYTE_ORDER_YUYV:
    while (i >= 0) {
      y1 = src[i--];
      y1 = (y1 + (((int)src[i--])<<8))>>(bits-8);
      y0 = src[i--];
      y0 = (y0 + (((int)src[i--])<<8))>>(bits-8);
      dest[j--] = 128;
      dest[j--] = y1;
      dest[j--] = 128;
      dest[j--] = y0;
    }
    break;
  case DC1394_BYTE_ORDER_UYVY:
    while (i >= 0) {
      y1 = src[i--];
      y1 = (y1 + (((int)src[i--])<<8))>>(bits-8);
      y0 = src[i--];
      y0 = (y0 + (((int)src[i--])<<8))>>(bits-8);
      dest[j--] = y1;
      dest[j--] = 128;
      dest[j--] = y0;
      dest[j--] = 128;
    }
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }

}

void
dc1394_MONO16_to_MONO8(uchar_t *src, uchar_t *dest, uint64_t NumPixels, uint_t bits)
{
  register int i = (NumPixels<<1)-1;
  register int j = NumPixels-1;
  register int y;

  while (i >= 0) {
    y = src[i--];
    dest[j--] = (y + (src[i--]<<8))>>(bits-8);
  }
}

void
dc1394_RGB8_to_YUV422(uchar_t *src, uchar_t *dest, uint64_t NumPixels, uint_t byte_order)
{
  register int i = NumPixels + ( NumPixels << 1 )-1;
  register int j = (NumPixels << 1)-1;
  register int y0, y1, u0, u1, v0, v1 ;
  register int r, g, b;

  switch (byte_order) {
  case DC1394_BYTE_ORDER_YUYV:
    while (i >= 0) {
      b = (uchar_t) src[i--];
      g = (uchar_t) src[i--];
      r = (uchar_t) src[i--];
      RGB2YUV (r, g, b, y0, u0 , v0);
      b = (uchar_t) src[i--];
      g = (uchar_t) src[i--];
      r = (uchar_t) src[i--];
      RGB2YUV (r, g, b, y1, u1 , v1);
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y0;
      dest[j--] = (u0+u1) >> 1;
      dest[j--] = y1;
    }
    break;
  case DC1394_BYTE_ORDER_UYVY:
    while (i >= 0) {
      b = (uchar_t) src[i--];
      g = (uchar_t) src[i--];
      r = (uchar_t) src[i--];
      RGB2YUV (r, g, b, y0, u0 , v0);
      b = (uchar_t) src[i--];
      g = (uchar_t) src[i--];
      r = (uchar_t) src[i--];
      RGB2YUV (r, g, b, y1, u1 , v1);
      dest[j--] = y0;
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y1;
      dest[j--] = (u0+u1) >> 1;
    }
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }
}

void
dc1394_RGB16_to_YUV422(uchar_t *src, uchar_t *dest, uint64_t NumPixels, uint_t byte_order)
{
  register int i = ( (NumPixels + ( NumPixels << 1 )) << 1 ) -1;
  register int j = (NumPixels << 1)-1;
  register int y0, y1, u0, u1, v0, v1 ;
  register int r, g, b;

  switch (byte_order) {
  case DC1394_BYTE_ORDER_YUYV:
    while (i >= 0) {
      i--;
      b = (uchar_t) src[i--];
      i--;
      g = (uchar_t) src[i--];
      i--;
      r = (uchar_t) src[i--];
      i--;
      RGB2YUV (r, g, b, y0, u0 , v0);
      b = (uchar_t) src[i--];
      i--;
      g = (uchar_t) src[i--];
      i--;
      r = (uchar_t) src[i--];
      RGB2YUV (r, g, b, y1, u1 , v1);
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y0;
      dest[j--] = (u0+u1) >> 1;
      dest[j--] = y1;
    } 
    break;
  case DC1394_BYTE_ORDER_UYVY:
    while (i >= 0) {
      i--;
      b = (uchar_t) src[i--];
      i--;
      g = (uchar_t) src[i--];
      i--;
      r = (uchar_t) src[i--];
      i--;
      RGB2YUV (r, g, b, y0, u0 , v0);
      b = (uchar_t) src[i--];
      i--;
      g = (uchar_t) src[i--];
      i--;
      r = (uchar_t) src[i--];
      RGB2YUV (r, g, b, y1, u1 , v1);
      dest[j--] = y0;
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y1;
      dest[j--] = (u0+u1) >> 1;
    }
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }
}

/**********************************************************************
 *
 *  CONVERSION FUNCTIONS TO RGB 24bpp 
 *
 **********************************************************************/

void
dc1394_RGB16_to_RGB8(uchar_t *src, uchar_t *dest, uint64_t NumPixels)
{
  register int i = ((NumPixels + ( NumPixels << 1 )) << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;

  while (i >= 0) {
    i--;
    dest[j--]=src[i--];
    i--;
    dest[j--]=src[i--];
    i--;
    dest[j--]=src[i--];
  }
}


void
dc1394_YUV444_to_RGB8(uchar_t *src, uchar_t *dest, uint64_t NumPixels)
{
  register int i = NumPixels + ( NumPixels << 1 ) -1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;
  register int y, u, v;
  register int r, g, b;

  while (i >= 0) {
    v = (uchar_t) src[i--] - 128;
    y = (uchar_t) src[i--];
    u = (uchar_t) src[i--] - 128;
    YUV2RGB (y, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;  
  }
}

void
dc1394_YUV422_to_RGB8(uchar_t *src, uchar_t *dest, uint64_t NumPixels)
{
  register int i = (NumPixels << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;
  register int y0, y1, u, v;
  register int r, g, b;

  while (i >= 0) {
    y1 = (uchar_t) src[i--];
    v  = (uchar_t) src[i--] - 128;
    y0 = (uchar_t) src[i--];
    u  = (uchar_t) src[i--] - 128;
    YUV2RGB (y1, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
    YUV2RGB (y0, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
  }
}


void
dc1394_YUV411_to_RGB8(uchar_t *src, uchar_t *dest, uint64_t NumPixels)
{
  register int i = NumPixels + ( NumPixels >> 1 )-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y0, y1, y2, y3, u, v;
  register int r, g, b;
  
  while (i >= 0) {
    y3 = (uchar_t) src[i--];
    y2 = (uchar_t) src[i--];
    v  = (uchar_t) src[i--] - 128;
    y1 = (uchar_t) src[i--];
    y0 = (uchar_t) src[i--];
    u  = (uchar_t) src[i--] - 128;
    YUV2RGB (y3, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
    YUV2RGB (y2, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
    YUV2RGB (y1, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
    YUV2RGB (y0, u, v, r, g, b);
    dest[j--] = b;
    dest[j--] = g;
    dest[j--] = r;
  }
}

void
dc1394_MONO8_to_RGB8(uchar_t *src, uchar_t *dest, uint64_t NumPixels)
{
  register int i = NumPixels-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y;

  while (i >= 0) {
    y = (uchar_t) src[i--];
    dest[j--] = y;
    dest[j--] = y;
    dest[j--] = y;
  }
}

void
dc1394_MONO16_to_RGB8(uchar_t *src, uchar_t *dest, uint64_t NumPixels, uint_t bits)
{
  register int i = (NumPixels << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y;

  while (i > 0) {
    y = src[i--];
    y = (y + (src[i--]<<8))>>(bits-8);
    dest[j--] = y;
    dest[j--] = y;
    dest[j--] = y;
  }
}

// change a 16bit stereo image (8bit/channel) into two 8bit images on top
// of each other
void
dc1394_deinterlace_stereo(uchar_t *src, uchar_t *dest, uint64_t NumPixels)
{
  register int i = NumPixels-1;
  register int j = (NumPixels>>1)-1;
  register int k = NumPixels-1;

  while (i >= 0) {
    dest[k--] = src[i--];
    dest[j--] = src[i--];
  }
}
