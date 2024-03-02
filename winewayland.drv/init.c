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

//WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);

static const struct user_driver_funcs waylanddrv_funcs;
unsigned int screen_bpp = 32;
unsigned int force_refresh = 0;

char *process_name = NULL;

/**********************************************************************
 *           WAYLANDDRV_wine_get_vulkan_driver
 */
static const struct vulkan_funcs * WAYLANDDRV_wine_get_vulkan_driver( UINT version )
{
    return get_vulkan_driver( version );
}

static const struct user_driver_funcs waylanddrv_funcs =
{

//    .dc_funcs.pCreateCompatibleDC = WAYLANDDRV_CreateCompatibleDC,  /* pCreateDC */
//    .dc_funcs.pCreateDC = WAYLANDDRV_CreateDC,  /* pCreateDC */
//    .dc_funcs.pDeleteDC = WAYLANDDRV_DeleteDC,  /* pDeleteDC */



//    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

   //.pActivateKeyboardLayout = WAYLANDDRV_ActivateKeyboardLayout,
    //.pChangeDisplaySettings = WAYLANDDRV_ChangeDisplaySettings,

    .pClipCursor = WAYLANDDRV_ClipCursor,
    //.pCreateDesktopWindow = WAYLANDDRV_CreateDesktopWindow,
    //.pCreateWindow = WAYLANDDRV_CreateWindow,
    //.pDestroyCursorIcon = WAYLANDDRV_DestroyCursorIcon,
    .pDestroyWindow = WAYLANDDRV_DestroyWindow,

    .pGetCurrentDisplaySettings = WAYLANDDRV_GetCurrentDisplaySettings,
    .pUpdateDisplayDevices = WAYLANDDRV_UpdateDisplayDevices,

    .pGetCursorPos = WAYLANDDRV_GetCursorPos,
    //.pGetKeyboardLayoutList = WAYLANDDRV_GetKeyboardLayoutList,
    .pGetKeyNameText = WAYLANDDRV_GetKeyNameText,
    .pMapVirtualKeyEx = WAYLANDDRV_MapVirtualKeyEx,

    .pProcessEvents = WAYLANDDRV_ProcessEvents,
    //.pSetCapture = WAYLANDDRV_SetCapture,
    .pSetCursor = WAYLANDDRV_SetCursor,
    //.pSetCursorPos = WAYLANDDRV_SetCursorPos,
    //.pSetFocus = WAYLANDDRV_SetFocus,


    //.pSetWindowText = WAYLANDDRV_SetWindowText,
    .pShowWindow = WAYLANDDRV_ShowWindow,
    .pSysCommand = WAYLANDDRV_SysCommand,
    .pToUnicodeEx = WAYLANDDRV_ToUnicodeEx,
    .pVkKeyScanEx = WAYLANDDRV_VkKeyScanEx,
    //.pWindowMessage = WAYLANDDRV_WindowMessage,
    .pWindowPosChanged = WAYLANDDRV_WindowPosChanged,
    .pWindowPosChanging = WAYLANDDRV_WindowPosChanging,
    .pwine_get_vulkan_driver = WAYLANDDRV_wine_get_vulkan_driver,   /* wine_get_vulkan_driver */

};



/***********************************************************************
 *           WAYLANDDRV process initialisation routine
 */

static NTSTATUS process_attach( void *arg )
{

  //TRACE("Entering wayland \n");

//  static WCHAR *current_exe = NULL;
  char *env_width, *env_height;

//  static WCHAR current_exepath[MAX_PATH] = {0};

  int screen_width = 1920;
  int screen_height = 1080;


  //TRACE("Setting driver \n");
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

  force_refresh = 1;

  return 0;
}



const unixlib_entry_t __wine_unix_call_funcs[] =
{
    process_attach,
};


C_ASSERT( ARRAYSIZE(__wine_unix_call_funcs) == unix_funcs_count );

#ifdef _WIN64

static NTSTATUS waylanddrv_wow64_init( void *arg )
{
    struct init_params params;

    return process_attach( &params );
}

const unixlib_entry_t __wine_unix_call_wow64_funcs[] =
{
    waylanddrv_wow64_init
};

C_ASSERT( ARRAYSIZE(__wine_unix_call_wow64_funcs) == unix_funcs_count );

#endif
