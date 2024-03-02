/* Automatically generated from http://www.opengl.org/registry files; DO NOT EDIT! */

#if 0
#pragma makedep unix
#endif

#include <stdarg.h>
#include <stddef.h>

#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "windef.h"
#include "winbase.h"
#include "wingdi.h"

#include "unixlib.h"
#include "unix_private.h"

#include "wine/debug.h"

#ifdef _WIN64
WINE_DEFAULT_DEBUG_CHANNEL(opengl);
#endif

extern NTSTATUS thread_attach( void *args ) ;
extern NTSTATUS process_detach( void *args ) ;
extern NTSTATUS wgl_wglCopyContext( void *args ) ;
extern NTSTATUS wgl_wglCreateContext( void *args ) ;
extern NTSTATUS wgl_wglDeleteContext( void *args ) ;
extern NTSTATUS wgl_wglGetProcAddress( void *args ) ;
extern NTSTATUS wgl_wglMakeCurrent( void *args ) ;
extern NTSTATUS wgl_wglShareLists( void *args ) ;
extern NTSTATUS gl_glGetIntegerv( void *args ) ;
extern NTSTATUS gl_glGetString( void *args ) ;
extern NTSTATUS ext_glDebugMessageCallback( void *args ) ;
extern NTSTATUS ext_glDebugMessageCallbackAMD( void *args ) ;
extern NTSTATUS ext_glDebugMessageCallbackARB( void *args ) ;
extern NTSTATUS ext_glGetStringi( void *args ) ;
extern NTSTATUS ext_wglBindTexImageARB( void *args ) ;
extern NTSTATUS ext_wglCreateContextAttribsARB( void *args ) ;
extern NTSTATUS ext_wglCreatePbufferARB( void *args ) ;
extern NTSTATUS ext_wglDestroyPbufferARB( void *args ) ;
extern NTSTATUS ext_wglGetPbufferDCARB( void *args ) ;
extern NTSTATUS ext_wglMakeContextCurrentARB( void *args ) ;
extern NTSTATUS ext_wglQueryPbufferARB( void *args ) ;
extern NTSTATUS ext_wglReleasePbufferDCARB( void *args ) ;
extern NTSTATUS ext_wglReleaseTexImageARB( void *args ) ;
extern NTSTATUS ext_wglSetPbufferAttribARB( void *args ) ;





extern NTSTATUS wow64_wgl_wglCreateContext( void *args ) ;
extern NTSTATUS wow64_wgl_wglDeleteContext( void *args ) ;
extern NTSTATUS wow64_wgl_wglGetProcAddress( void *args ) ;
extern NTSTATUS wow64_wgl_wglMakeCurrent( void *args ) ;
extern NTSTATUS wow64_gl_glGetString( void *args ) ;
extern NTSTATUS wow64_ext_glClientWaitSync( void *args ) ;
extern NTSTATUS wow64_ext_glDeleteSync( void *args ) ;
extern NTSTATUS wow64_ext_glFenceSync( void *args ) ;
extern NTSTATUS wow64_ext_glGetBufferPointerv( void *args ) ;
extern NTSTATUS wow64_ext_glGetBufferPointervARB( void *args ) ;
extern NTSTATUS wow64_ext_glGetNamedBufferPointerv( void *args ) ;
extern NTSTATUS wow64_ext_glGetNamedBufferPointervEXT( void *args ) ;
extern NTSTATUS wow64_ext_glGetStringi( void *args ) ;
extern NTSTATUS wow64_ext_glGetSynciv( void *args ) ;
extern NTSTATUS wow64_ext_glIsSync( void *args ) ;
extern NTSTATUS wow64_ext_glMapBuffer( void *args ) ;
extern NTSTATUS wow64_ext_glMapBufferARB( void *args ) ;
extern NTSTATUS wow64_ext_glMapBufferRange( void *args ) ;
extern NTSTATUS wow64_ext_glMapNamedBuffer( void *args ) ;
extern NTSTATUS wow64_ext_glMapNamedBufferEXT( void *args ) ;
extern NTSTATUS wow64_ext_glMapNamedBufferRange( void *args ) ;
extern NTSTATUS wow64_ext_glMapNamedBufferRangeEXT( void *args ) ;
extern NTSTATUS wow64_ext_glPathGlyphIndexRangeNV( void *args ) ;
extern NTSTATUS wow64_ext_glUnmapBuffer( void *args ) ;
extern NTSTATUS wow64_ext_glUnmapBufferARB( void *args ) ;
extern NTSTATUS wow64_ext_glUnmapNamedBuffer( void *args ) ;
extern NTSTATUS wow64_ext_glUnmapNamedBufferEXT( void *args ) ;
extern NTSTATUS wow64_ext_glWaitSync( void *args ) ;
extern NTSTATUS wow64_ext_wglCreateContextAttribsARB( void *args ) ;
extern NTSTATUS wow64_ext_wglCreatePbufferARB( void *args ) ;
extern NTSTATUS wow64_ext_wglGetExtensionsStringARB( void *args ) ;
extern NTSTATUS wow64_ext_wglGetExtensionsStringEXT( void *args ) ;
extern NTSTATUS wow64_ext_wglGetPbufferDCARB( void *args ) ;
extern NTSTATUS wow64_ext_wglMakeContextCurrentARB( void *args ) ;
extern NTSTATUS wow64_ext_wglQueryCurrentRendererStringWINE( void *args ) ;
extern NTSTATUS wow64_ext_wglQueryRendererStringWINE( void *args ) ;

