/*
 * Wayland graphics driver initialisation functions
 *
 * Copyright 1996 Alexandre Julliard
 * Copyright 2020-2022 varmd
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

#define NONAMELESSSTRUCT
#define NONAMELESSUNION

#include "config.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "windef.h"
#include "winbase.h"
#include "winreg.h"

#include "waylanddrv.h"

#include "wine/debug.h"
#include "unixlib.h"

WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);




char *process_name = NULL;

static struct user_driver_funcs waylanddrv_funcs =
{


     //.pActivateKeyboardLayout = WAYLANDDRV_ActivateKeyboardLayout,

    .pClipCursor = WAYLANDDRV_ClipCursor,
    //.pCreateDesktopWindow = WAYLANDDRV_CreateDesktopWindow,
    .pCreateWindow = WAYLANDDRV_CreateWindow,
    //.pDestroyCursorIcon = WAYLANDDRV_DestroyCursorIcon,
    .pDestroyWindow = WAYLANDDRV_DestroyWindow,
    .pUpdateDisplayDevices = WAYLANDDRV_UpdateDisplayDevices,

    .pGetCursorPos = WAYLANDDRV_GetCursorPos,
    //.pGetKeyboardLayoutList = WAYLANDDRV_GetKeyboardLayoutList,
    .pGetKeyNameText = WAYLANDDRV_GetKeyNameText,
    .pMapVirtualKeyEx = WAYLANDDRV_MapVirtualKeyEx,

    //.pProcessEvents = WAYLANDDRV_ProcessEvents,

    .pSetCursor = WAYLANDDRV_SetCursor,
    //.pSetCursorPos = WAYLANDDRV_SetCursorPos,

    .pShowWindow = WAYLANDDRV_ShowWindow,
    .pSysCommand = WAYLANDDRV_SysCommand,
    .pToUnicodeEx = WAYLANDDRV_ToUnicodeEx,
    .pVkKeyScanEx = WAYLANDDRV_VkKeyScanEx,
    //.pWindowMessage = WAYLANDDRV_WindowMessage,

    .pCreateWindowSurface = WAYLANDDRV_CreateWindowSurface,
    //TODO
    .pWindowPosChanged = WAYLANDDRV_WindowPosChanged,
    //.pWindowPosChanging = WAYLANDDRV_WindowPosChanging,

    .pVulkanInit = WAYLANDDRV_VulkanInit

};



/***********************************************************************
 *           WAYLANDDRV process initialisation routine
 */

static NTSTATUS process_attach( void *arg )
{

  WCHAR file_name[MAX_PATH]={0};



  //TRACE("Entering wayland %s \n", debugstr_wn(arg, lstrlenW( arg )) );
  TRACE("Entering wayland for %s \n", arg );

  //TODO figure out why it hangs
  if(strcmp(arg, "rundll32.exe") == 0) {
    TRACE("Skipping wayland for rundll32.exe \n" );
    return 0;
  }
  if(strcmp(arg, "wineboot.exe") == 0) {
    TRACE("Skipping wayland for wineboot.exe \n" );
    return 0;
  }

  //  static WCHAR *current_exe = NULL;
  char *env_width, *env_height;
  char *event_thread_disabled = getenv( "WINE_VK_NO_EVENT_THREAD" );


  int screen_width = 1920;
  int screen_height = 1080;

  if(event_thread_disabled) {
    waylanddrv_funcs.pProcessEvents = WAYLANDDRV_ProcessEvents;
  }

  __wine_set_user_driver( &waylanddrv_funcs, WINE_GDI_DRIVER_VERSION );


  env_width = getenv( "WINE_VK_WAYLAND_WIDTH" );
  env_height = getenv( "WINE_VK_WAYLAND_HEIGHT" );

  if(env_width) {
    screen_width = atoi(env_width);
  }
  if(env_height) {
    screen_height = atoi(env_height);
  }

  xinerama_init( screen_width , screen_height);



  return 0;
}


static NTSTATUS read_events(LPVOID arg)
{
  int count = 0;
  char *event_thread_disabled = getenv( "WINE_VK_NO_EVENT_THREAD" );
  //LPSTR strptr = *(LPSTR*)arg;
  //int value = *(int*)arg; // Dereference the pointer to get the value
 //int *value = (int*)arg;
 //crashes or zero
 //int value = (int) arg;
 //TRACE("Entering wayland event thread for %d \n", value );



  #if 0

  TRACE("Entering wayland event thread for %s \n", arg );


  if(strcmp(arg, "rundll32.exe") == 0) {
    TRACE("Skipping wayland event thread for %s \n", arg );
    return 0;
  }
  if(strcmp(arg, "wineboot.exe") == 0) {
    TRACE("Skipping wayland event thread for %s \n", arg );
    return 0;
  }

 #endif
  //Driver does not work with thread exiting
  if(event_thread_disabled) {
    while(1)
      sleep(100000000);
  }

  while(!wayland_display_ready || !wl_event_queue || !wayland_display) {
    usleep(20000); //0.02 seconds
    count++;
    if(count > 10200) //probably not a wayland app
      return 0;
  }

  TRACE("Found wl_event_queue and wayland_display %d\n", count);
  while (wl_display_dispatch_queue(wayland_display,
                                     wl_event_queue) != -1) {
    continue;
  }

  return STATUS_UNSUCCESSFUL;
}


const unixlib_entry_t __wine_unix_call_funcs[] =
{
    process_attach,
    read_events,
};


C_ASSERT( ARRAYSIZE(__wine_unix_call_funcs) == unix_funcs_count );



#ifdef _WIN64

static NTSTATUS waylanddrv_wow64_init( void *arg )
{
    struct init_params params;

    return process_attach( &params );
}

static NTSTATUS waylanddrv_read_events( void *arg )
{
  return read_events( arg );
}

const unixlib_entry_t __wine_unix_call_wow64_funcs[] =
{
    waylanddrv_wow64_init,
    waylanddrv_read_events
};

C_ASSERT( ARRAYSIZE(__wine_unix_call_wow64_funcs) == unix_funcs_count );

#endif
