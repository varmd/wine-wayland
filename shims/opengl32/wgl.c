/* Window-specific OpenGL functions implementation.
 *
 * Copyright (c) 1999 Lionel Ulmer
 * Copyright (c) 2005 Raphael Junqueira
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>

#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "winreg.h"

#include "winternl.h"
#include "winnt.h"

#include "wine/wgl.h"
#include "wine/glu.h"
#include "wine/debug.h"
#include "wingdi.h"

/* handle management */

#define MAX_WGL_HANDLES 1024

enum wgl_handle_type
{
    HANDLE_PBUFFER = 0 << 12,
    HANDLE_CONTEXT = 1 << 12,
    HANDLE_CONTEXT_V3 = 3 << 12,
    HANDLE_TYPE_MASK = 15 << 12
};

struct opengl_context
{
    DWORD               tid;           /* thread that the context is current in */
    HDC                 draw_dc;       /* current drawing DC */
    HDC                 read_dc;       /* current reading DC */
    void     (CALLBACK *debug_callback)(GLenum, GLenum, GLuint, GLenum,
                                        GLsizei, const GLchar *, const void *); /* debug callback */
    const void         *debug_user;    /* debug user parameter */
    GLubyte            *extensions;    /* extension string */
    GLuint             *disabled_exts; /* indices of disabled extensions */
    struct wgl_context *drv_ctx;       /* driver context */
};

struct wgl_handle
{
    UINT                 handle;
    struct opengl_funcs *funcs;
    union
    {
        struct opengl_context *context;  /* for HANDLE_CONTEXT */
        struct wgl_pbuffer    *pbuffer;  /* for HANDLE_PBUFFER */
        struct wgl_handle     *next;     /* for free handles */
    } u;
};




/***********************************************************************
 *		wglDeleteContext (OPENGL32.@)
 */
BOOL WINAPI wglDeleteContext(HGLRC hglrc)
{

    return TRUE;
}

/***********************************************************************
 *		wglMakeCurrent (OPENGL32.@)
 */
BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
  return FALSE;
}

/***********************************************************************
 *		wglCreateContextAttribsARB
 *
 * Provided by the WGL_ARB_create_context extension.
 */
HGLRC WINAPI wglCreateContextAttribsARB( HDC hdc, HGLRC share, const int *attribs )
{
    return FALSE;

}

/***********************************************************************
 *		wglMakeContextCurrentARB
 *
 * Provided by the WGL_ARB_make_current_read extension.
 */
BOOL WINAPI wglMakeContextCurrentARB( HDC draw_hdc, HDC read_hdc, HGLRC hglrc )
{
    return FALSE;
}

/***********************************************************************
 *		wglGetCurrentReadDCARB
 *
 * Provided by the WGL_ARB_make_current_read extension.
 */
HDC WINAPI wglGetCurrentReadDCARB(void)
{
   return 0;

}

/***********************************************************************
 *		wglShareLists (OPENGL32.@)
 */
BOOL WINAPI wglShareLists(HGLRC hglrcSrc, HGLRC hglrcDst)
{
    BOOL ret = FALSE;

    return ret;
}

/***********************************************************************
 *		wglGetCurrentDC (OPENGL32.@)
 */
HDC WINAPI wglGetCurrentDC(void)
{
  return 0;

}


/***********************************************************************
 *		wglCreateContext (OPENGL32.@)
 */
HGLRC WINAPI wglCreateContext(HDC hdc)
{
    return NULL;
}

/***********************************************************************
 *		wglGetCurrentContext (OPENGL32.@)
 */
HGLRC WINAPI wglGetCurrentContext(void)
{
    return NtCurrentTeb()->glCurrentRC;
}

/***********************************************************************
 *		wglDescribePixelFormat (OPENGL32.@)
 */
INT WINAPI wglDescribePixelFormat(HDC hdc, INT format, UINT size, PIXELFORMATDESCRIPTOR *descr )
{

    return 0;

}

/***********************************************************************
 *		wglChoosePixelFormat (OPENGL32.@)
 */
INT WINAPI wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR* ppfd)
{
  return 0;
}

/***********************************************************************
 *		wglGetPixelFormat (OPENGL32.@)
 */
INT WINAPI wglGetPixelFormat(HDC hdc)
{

        return 0;

}

/***********************************************************************
 *		 wglSetPixelFormat(OPENGL32.@)
 */
BOOL WINAPI wglSetPixelFormat( HDC hdc, INT format, const PIXELFORMATDESCRIPTOR *descr )
{
  return FALSE;

}

/***********************************************************************
 *              wglSwapBuffers (OPENGL32.@)
 */
BOOL WINAPI DECLSPEC_HOTPATCH wglSwapBuffers( HDC hdc )
{
      return TRUE;
}

/***********************************************************************
 *		wglCreateLayerContext (OPENGL32.@)
 */
HGLRC WINAPI wglCreateLayerContext(HDC hdc,
				   int iLayerPlane) {

  return NULL;
}

/***********************************************************************
 *		wglDescribeLayerPlane (OPENGL32.@)
 */
BOOL WINAPI wglDescribeLayerPlane(HDC hdc,
				  int iPixelFormat,
				  int iLayerPlane,
				  UINT nBytes,
				  LPLAYERPLANEDESCRIPTOR plpd) {
  return FALSE;
}

/***********************************************************************
 *		wglGetLayerPaletteEntries (OPENGL32.@)
 */
int WINAPI wglGetLayerPaletteEntries(HDC hdc,
				     int iLayerPlane,
				     int iStart,
				     int cEntries,
				     const COLORREF *pcr) {


  return 0;
}

static BOOL filter_extensions(const char *extensions, GLubyte **exts_list, GLuint **disabled_exts);


const GLubyte * WINAPI glGetStringi(GLenum name, GLuint index)
{
  return NULL;
}



/***********************************************************************
 *		wglGetProcAddress (OPENGL32.@)
 */
PROC WINAPI wglGetProcAddress( LPCSTR name )
{
    return NULL;


}

/***********************************************************************
 *		wglRealizeLayerPalette (OPENGL32.@)
 */
BOOL WINAPI wglRealizeLayerPalette(HDC hdc,
				   int iLayerPlane,
				   BOOL bRealize) {


  return FALSE;
}

/***********************************************************************
 *		wglSetLayerPaletteEntries (OPENGL32.@)
 */
int WINAPI wglSetLayerPaletteEntries(HDC hdc,
				     int iLayerPlane,
				     int iStart,
				     int cEntries,
				     const COLORREF *pcr) {


  return 0;
}

/***********************************************************************
 *		wglGetDefaultProcAddress (OPENGL32.@)
 */
PROC WINAPI wglGetDefaultProcAddress( LPCSTR name )
{

    return NULL;
}

/***********************************************************************
 *		wglSwapLayerBuffers (OPENGL32.@)
 */
BOOL WINAPI wglSwapLayerBuffers(HDC hdc,
				UINT fuPlanes) {


  return TRUE;
}

/***********************************************************************
 *		wglBindTexImageARB
 *
 * Provided by the WGL_ARB_render_texture extension.
 */
BOOL WINAPI wglBindTexImageARB( HPBUFFERARB handle, int buffer )
{
   return FALSE;
}

/***********************************************************************
 *		wglReleaseTexImageARB
 *
 * Provided by the WGL_ARB_render_texture extension.
 */
BOOL WINAPI wglReleaseTexImageARB( HPBUFFERARB handle, int buffer )
{
    return FALSE;
}

/***********************************************************************
 *		wglSetPbufferAttribARB
 *
 * Provided by the WGL_ARB_render_texture extension.
 */
BOOL WINAPI wglSetPbufferAttribARB( HPBUFFERARB handle, const int *attribs )
{
  return FALSE;

}

/***********************************************************************
 *		wglCreatePbufferARB
 *
 * Provided by the WGL_ARB_pbuffer extension.
 */
HPBUFFERARB WINAPI wglCreatePbufferARB( HDC hdc, int format, int width, int height, const int *attribs )
{
   return NULL;
}

/***********************************************************************
 *		wglGetPbufferDCARB
 *
 * Provided by the WGL_ARB_pbuffer extension.
 */
HDC WINAPI wglGetPbufferDCARB( HPBUFFERARB handle )
{
  return 0;

}

/***********************************************************************
 *		wglReleasePbufferDCARB
 *
 * Provided by the WGL_ARB_pbuffer extension.
 */
int WINAPI wglReleasePbufferDCARB( HPBUFFERARB handle, HDC hdc )
{
   return 0;
}

/***********************************************************************
 *		wglDestroyPbufferARB
 *
 * Provided by the WGL_ARB_pbuffer extension.
 */
BOOL WINAPI wglDestroyPbufferARB( HPBUFFERARB handle )
{
  return FALSE;

}

/***********************************************************************
 *		wglQueryPbufferARB
 *
 * Provided by the WGL_ARB_pbuffer extension.
 */
BOOL WINAPI wglQueryPbufferARB( HPBUFFERARB handle, int attrib, int *value )
{
  return FALSE;

}



/***********************************************************************
 *		wglUseFontBitmapsA (OPENGL32.@)
 */
BOOL WINAPI wglUseFontBitmapsA(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
    return FALSE;
}

/***********************************************************************
 *		wglUseFontBitmapsW (OPENGL32.@)
 */
BOOL WINAPI wglUseFontBitmapsW(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
    return FALSE;
}



/***********************************************************************
 *		wglUseFontOutlinesA (OPENGL32.@)
 */
BOOL WINAPI wglUseFontOutlinesA(HDC hdc,
				DWORD first,
				DWORD count,
				DWORD listBase,
				FLOAT deviation,
				FLOAT extrusion,
				int format,
				LPGLYPHMETRICSFLOAT lpgmf)
{
    return FALSE;
}

/***********************************************************************
 *		wglUseFontOutlinesW (OPENGL32.@)
 */
BOOL WINAPI wglUseFontOutlinesW(HDC hdc,
				DWORD first,
				DWORD count,
				DWORD listBase,
				FLOAT deviation,
				FLOAT extrusion,
				int format,
				LPGLYPHMETRICSFLOAT lpgmf)
{
    return FALSE;
}

/***********************************************************************
 *              glDebugEntry (OPENGL32.@)
 */
GLint WINAPI glDebugEntry( GLint unknown1, GLint unknown2 )
{
    return 0;
}

/***********************************************************************
 *              glGetString (OPENGL32.@)
 */
const GLubyte * WINAPI glGetString( GLenum name )
{
   return NULL;
}



/***********************************************************************
 *      glDebugMessageCallback
 */
void WINAPI glDebugMessageCallback( GLDEBUGPROC callback, const void *userParam )
{

}

/***********************************************************************
 *      glDebugMessageCallbackAMD
 */
void WINAPI glDebugMessageCallbackAMD( GLDEBUGPROCAMD callback, void *userParam )
{

}

/***********************************************************************
 *      glDebugMessageCallbackARB
 */
void WINAPI glDebugMessageCallbackARB( GLDEBUGPROCARB callback, const void *userParam )
{

}

/***********************************************************************
 *           OpenGL initialisation routine
 */
BOOL WINAPI DllMain( HINSTANCE hinst, DWORD reason, LPVOID reserved )
{

    return TRUE;
}
