/*
 * Last edit by previous maintainer:
 * 2000/01/06 16:37:43, kusano
 *
 * Copyright (C) 1999 - 2005 Yoshi <yoshi@giganet.net>
 * Copyright (C) 2006 John M. Gabriele <jmg3000@gmail.com>
 * Copyright (C) 2007 James Adam <james@lazyatom.com>
 * Copyright (C) 2007 Jan Dvorak <jan.dvorak@kraxnet.cz>
 *
 * This program is distributed under the terms of the MIT license.
 * See the included MIT-LICENSE file for the terms of this license.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <ruby.h>
#include "extconf.h"

#ifdef HAVE_OPENGL_GLU_H
#include <OpenGL/glu.h>
#endif

#ifdef HAVE_GL_GLU_H
#include <GL/glu.h>
#endif

#include "glu-enums.h"
#include "conv.h"

#ifndef CALLBACK
#define CALLBACK
#endif

#ifdef HAVE_WINDOWS_H
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/* these two macros are cast to a 32 bit type in the places they are used */
#ifndef RARRAY_LENINT
#define RARRAY_LENINT RARRAY_LEN
#endif

/* -------------------------------------------------------------------- */

/* gets number of components for given format */
static inline int glformat_size(GLenum format)
{
  switch(format)
  {
    case GL_COLOR_INDEX:
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RED_INTEGER_EXT:
    case GL_GREEN_INTEGER_EXT:
    case GL_BLUE_INTEGER_EXT:
    case GL_ALPHA_INTEGER_EXT:
    case GL_STENCIL_INDEX:
    case GL_DEPTH_COMPONENT:
    case GL_LUMINANCE:
    case GL_LUMINANCE_INTEGER_EXT:
      return 1;
    
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE_ALPHA_INTEGER_EXT:
    case GL_422_EXT:
    case GL_422_REV_EXT:
    case GL_422_AVERAGE_EXT:
    case GL_422_REV_AVERAGE_EXT:
    case GL_YCRCB_422_SGIX:
    case GL_YCBCR_422_APPLE:
    case GL_YCBCR_MESA:
    case GL_DEPTH_STENCIL_NV:
    case GL_HILO_NV:
    case GL_DSDT_NV:
    case GL_DUDV_ATI:
    case GL_DU8DV8_ATI:
    case GL_FORMAT_SUBSAMPLE_24_24_OML:
      return 2;
    
    case GL_RGB:
    case GL_RGB_INTEGER_EXT:
    case GL_BGR_EXT:
    case GL_BGR_INTEGER_EXT:
    case GL_YCRCB_444_SGIX:
    case GL_DSDT_MAG_NV:
    case GL_FORMAT_SUBSAMPLE_244_244_OML:
      return 3;
    
    case GL_RGBA:
    case GL_RGBA_INTEGER_EXT:
    case GL_BGRA_EXT:
    case GL_BGRA_INTEGER_EXT:
    case GL_ABGR_EXT:
    case GL_CMYK_EXT:
    case GL_DSDT_MAG_VIB_NV:
      return 4;

    case GL_CMYKA_EXT:
      return 5;

    /* GL spec permits passing direct format size instead of enum (now obsolete) */
    case 1:
    case 2:
    case 3:
    case 4:
      return format;

    default:
      rb_raise(rb_eArgError, "Unknown GL format enum %i",format);
      return -1; /* not reached */
  }
}

 /* computes unit (pixel) size for given type and format */
static inline int gltype_glformat_unit_size(GLenum type,GLenum format)
{
  unsigned int format_size;
  
  format_size = glformat_size(format);
  
  switch(type)
  {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_BITMAP:
      return 1*format_size;

    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_HALF_FLOAT_ARB:
      return 2*format_size;

    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
      return 4*format_size;

    /* in packed formats all components are packed into/unpacked from single datatype,
       so number of components(format_size) doesn't matter for total size calculation */
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
      return 1;

    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_SHORT_8_8_APPLE:
    case GL_UNSIGNED_SHORT_8_8_REV_APPLE:
      return 2;

    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_24_8_NV:
    case GL_UNSIGNED_INT_S8_S8_8_8_NV:
    case GL_UNSIGNED_INT_8_8_S8_S8_REV_NV:
    case GL_UNSIGNED_INT_10F_11F_11F_REV_EXT:
    case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:
    case GL_FLOAT_32_UNSIGNED_INT_24_8_REV_NV:
      return 4;
  
    default:
      rb_raise(rb_eArgError, "Unknown GL type enum %i",type);
      return -1; /* not reached */
  }
}

static inline int GetDataSize(GLenum type,GLenum format,int num)
{
  int size;
  int unit_size;

  unit_size = gltype_glformat_unit_size(type,format);
  
  if (type==GL_BITMAP)
    size = unit_size*(num/8); /* FIXME account for alignment */
  else
    size = unit_size*num;

  return size;
}

/* Checks if data size of 'data' string confirms to passed format values */
/* 'num' is number of elements, each of size 'format' * 'type' */
static inline void CheckDataSize(GLenum type,GLenum format,int num,VALUE data)
{
  int size;

  size = GetDataSize(type,format,num);
  
  if (RSTRING_LEN(data) < size)
    rb_raise(rb_eArgError, "Length of specified data doesn't correspond to format and type parameters passed. Calculated length: %i",size);
}

/* -------------------------------------------------------------------- */
static inline VALUE allocate_buffer_with_string( long size )
{
    return rb_str_new(NULL, size);
}

#endif
