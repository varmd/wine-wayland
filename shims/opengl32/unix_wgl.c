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

#if 0
#pragma makedep unix
#endif

#include "config.h"

#include <stdarg.h>
#include <stdlib.h>

#include <pthread.h>

#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "windef.h"
#include "winbase.h"
#include "winreg.h"
#include "ntuser.h"

#include "unixlib.h"
#include "unix_private.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(opengl);

static const BOOL is_win64 = (sizeof(void *) > sizeof(int));

static BOOL is_wow64(void)
{
    return !!NtCurrentTeb()->WowTebOffset;
}

static pthread_mutex_t wgl_lock = PTHREAD_MUTEX_INITIALIZER;

/* handle management */

enum wgl_handle_type
{
    HANDLE_PBUFFER = 0 << 12,
    HANDLE_CONTEXT = 1 << 12,
    HANDLE_CONTEXT_V3 = 3 << 12,
    HANDLE_GLSYNC = 4 << 12,
    HANDLE_TYPE_MASK = 15 << 12,
};

struct opengl_context
{
    DWORD tid;                   /* thread that the context is current in */
    void (CALLBACK *debug_callback)( GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *, const void * ); /* debug callback */
    const void *debug_user;      /* debug user parameter */
    GLubyte *extensions;         /* extension string */
    GLuint *disabled_exts;       /* indices of disabled extensions */
    struct wgl_context *drv_ctx; /* driver context */
};

struct wgl_handle
{
    UINT handle;
    struct opengl_funcs *funcs;
    union
    {
        struct opengl_context *context; /* for HANDLE_CONTEXT */
        struct wgl_pbuffer *pbuffer;    /* for HANDLE_PBUFFER */
        struct wgl_handle *next;        /* for free handles */
    } u;
};

#define MAX_WGL_HANDLES 1024




NTSTATUS wgl_wglCopyContext( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS wgl_wglCreateContext( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS wgl_wglDeleteContext( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS wgl_wglGetProcAddress( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS wgl_wglMakeCurrent( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS wgl_wglShareLists( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS gl_glGetIntegerv( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS gl_glGetString( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_glDebugMessageCallback( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_glDebugMessageCallbackAMD( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_glDebugMessageCallbackARB( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_glGetStringi( void *args )
{
  return STATUS_SUCCESS;
}

NTSTATUS ext_wglBindTexImageARB( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_wglCreateContextAttribsARB( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_wglCreatePbufferARB( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_wglDestroyPbufferARB( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_wglGetPbufferDCARB( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_wglMakeContextCurrentARB( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_wglQueryPbufferARB( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_wglReleasePbufferDCARB( void *args )
{

    return STATUS_SUCCESS;
}

NTSTATUS ext_wglReleaseTexImageARB( void *args )
{
  return STATUS_SUCCESS;
}

NTSTATUS ext_wglSetPbufferAttribARB( void *args )
{
  return STATUS_SUCCESS;
}

NTSTATUS thread_attach( void *args )
{
  return STATUS_SUCCESS;
}

NTSTATUS process_detach( void *args )
{
    return STATUS_SUCCESS;
}

#ifdef _WIN64

typedef ULONG PTR32;

extern NTSTATUS ext_glClientWaitSync( void *args ) ;
extern NTSTATUS ext_glDeleteSync( void *args ) ;
extern NTSTATUS ext_glFenceSync( void *args ) ;
extern NTSTATUS ext_glGetBufferPointerv( void *args ) ;
extern NTSTATUS ext_glGetBufferPointervARB( void *args ) ;
extern NTSTATUS ext_glGetNamedBufferPointerv( void *args ) ;
extern NTSTATUS ext_glGetNamedBufferPointervEXT( void *args ) ;
extern NTSTATUS ext_glGetSynciv( void *args ) ;
extern NTSTATUS ext_glIsSync( void *args ) ;
extern NTSTATUS ext_glMapBuffer( void *args ) ;

extern NTSTATUS ext_glUnmapBuffer( void *args ) ;
extern NTSTATUS ext_glUnmapBufferARB( void *args ) ;
extern NTSTATUS ext_glUnmapNamedBuffer( void *args ) ;
extern NTSTATUS ext_glUnmapNamedBufferEXT( void *args ) ;

extern NTSTATUS ext_glMapBufferARB( void *args ) ;
extern NTSTATUS ext_glMapBufferRange( void *args ) ;
extern NTSTATUS ext_glMapNamedBuffer( void *args ) ;
extern NTSTATUS ext_glMapNamedBufferEXT( void *args ) ;
extern NTSTATUS ext_glMapNamedBufferRange( void *args ) ;
extern NTSTATUS ext_glMapNamedBufferRangeEXT( void *args ) ;
extern NTSTATUS ext_glPathGlyphIndexRangeNV( void *args ) ;
extern NTSTATUS ext_glWaitSync( void *args ) ;
extern NTSTATUS ext_wglGetExtensionsStringARB( void *args ) ;
extern NTSTATUS ext_wglGetExtensionsStringEXT( void *args ) ;
extern NTSTATUS ext_wglQueryCurrentRendererStringWINE( void *args ) ;
extern NTSTATUS ext_wglQueryRendererStringWINE( void *args ) ;

struct wow64_string_entry
{
    const char *str;
    PTR32 wow64_str;
};


NTSTATUS wow64_wgl_wglCreateContext( void *args )
{
   return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_wglCreateContextAttribsARB( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_wglCreatePbufferARB( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_wgl_wglDeleteContext( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_wgl_wglMakeCurrent( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_wglMakeContextCurrentARB( void *args )
{
   return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_wglGetPbufferDCARB( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_wgl_wglGetProcAddress( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_gl_glGetString( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glGetStringi( void *args )
{
   return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glPathGlyphIndexRangeNV( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_wglGetExtensionsStringARB( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_wglGetExtensionsStringEXT( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_wglQueryCurrentRendererStringWINE( void *args )
{
   return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_wglQueryRendererStringWINE( void *args )
{
   return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glClientWaitSync( void *args )
{
   return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glDeleteSync( void *args )
{
   return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glFenceSync( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glGetSynciv( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glIsSync( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glWaitSync( void *args )
{
    return STATUS_SUCCESS;
}


NTSTATUS wow64_ext_glGetBufferPointerv( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glGetBufferPointervARB( void *args )
{
    return STATUS_SUCCESS;
}

static NTSTATUS wow64_gl_get_named_buffer_pointer_v( void *args, NTSTATUS (*gl_get_named_buffer_pointer_v64)(void *) )
{

    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glGetNamedBufferPointerv( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glGetNamedBufferPointervEXT( void *args )
{
    return STATUS_SUCCESS;
}

static NTSTATUS wow64_gl_map_buffer( void *args, NTSTATUS (*gl_map_buffer64)(void *) )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glMapBuffer( void *args )
{
  return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glMapBufferARB( void *args )
{
  return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glMapBufferRange( void *args )
{

return STATUS_SUCCESS;
}

static NTSTATUS wow64_gl_map_named_buffer( void *args, NTSTATUS (*gl_map_named_buffer64)(void *) )
{

 return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glMapNamedBuffer( void *args )
{
  return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glMapNamedBufferEXT( void *args )
{
  return STATUS_SUCCESS;
}



NTSTATUS wow64_ext_glMapNamedBufferRange( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glMapNamedBufferRangeEXT( void *args )
{
  return STATUS_SUCCESS;
}

static NTSTATUS wow64_gl_unmap_buffer( void *args, NTSTATUS (*gl_unmap_buffer64)(void *) )
{

    NTSTATUS status;



    return status;
}

NTSTATUS wow64_ext_glUnmapBuffer( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glUnmapBufferARB( void *args )
{
    return STATUS_SUCCESS;
}

static NTSTATUS wow64_gl_unmap_named_buffer( void *args, NTSTATUS (*gl_unmap_named_buffer64)(void *) )
{


    NTSTATUS status;
    return STATUS_SUCCESS;

}

NTSTATUS wow64_ext_glUnmapNamedBuffer( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_ext_glUnmapNamedBufferEXT( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_thread_attach( void *args )
{
    return STATUS_SUCCESS;
}

NTSTATUS wow64_process_detach( void *args )
{


    return STATUS_SUCCESS;
}

#endif
