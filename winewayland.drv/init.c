/*
 * Wayland graphics driver initialisation functions
 *
 * Copyright 1996 Alexandre Julliard
 * Copyright 2020 varmd
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

#include "config.h"

#include <stdarg.h>
#include <string.h>

#include "windef.h"
#include "winbase.h"
#include "winreg.h"

#include "waylanddrv.h"

#include "wine/debug.h"
#include <stdlib.h>

WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);

Display *gdi_display;  /* display to use for all GDI functions */

static int palette_size = 16777216;



static const struct user_driver_funcs waylanddrv_funcs;

ColorShifts global_palette_default_shifts = { {0,0,0,}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} };

#if 0
/**********************************************************************
 *	     device_init
 *
 * Perform initializations needed upon creation of the first device.
 */
static BOOL WINAPI device_init( INIT_ONCE *once, void *param, void **context )
{

     ;


    return TRUE;
}
#endif

static inline void push_dc_driver2( PHYSDEV *dev, PHYSDEV physdev, const struct gdi_dc_funcs *funcs )
{

  while ((*dev)->funcs->priority > funcs->priority) dev = &(*dev)->next;
    physdev->funcs = funcs;
    physdev->next = *dev;
    physdev->hdc = (*dev)->hdc;
    *dev = physdev;
}

static WAYLANDDRV_PDEVICE *create_x11_physdev( void )
{
    WAYLANDDRV_PDEVICE *physDev;


    if (!(physDev = calloc( 1, sizeof(*physDev) ))) return NULL;
    return physDev;


}

/**********************************************************************
 *	     WAYLANDDRV_CreateDC
 */
static BOOL CDECL WAYLANDDRV_CreateDC( PHYSDEV *pdev, LPCWSTR device,
                             LPCWSTR output,
  const DEVMODEW* initData )
{

    WAYLANDDRV_PDEVICE *physDev = create_x11_physdev( /*root_window*/ );

    if (!physDev) return FALSE;


    physDev->depth         = 32;
    //physDev->color_shifts  = &global_palette_default_shifts;
    physDev->dc_rect       = get_virtual_screen_rect();



    OffsetRect( &physDev->dc_rect, -physDev->dc_rect.left, -physDev->dc_rect.top );




    if(!&physDev->dev) {
      TRACE( "NO dev \n" );
      exit(1);
    }

    push_dc_driver2( pdev, &physDev->dev, &waylanddrv_funcs.dc_funcs );

    return TRUE;
}


/**********************************************************************
 *	     WAYLANDDRV_CreateCompatibleDC
 */
static BOOL CDECL WAYLANDDRV_CreateCompatibleDC( PHYSDEV orig, PHYSDEV *pdev )
{
    WAYLANDDRV_PDEVICE *physDev = create_x11_physdev( /*stock_bitmap_pixmap*/ );

    if (!physDev) return FALSE;

    physDev->depth  = 1;
    SetRect( &physDev->dc_rect, 0, 0, 1, 1 );
    push_dc_driver( pdev, &physDev->dev, &waylanddrv_funcs.dc_funcs );

    return TRUE;
}


/**********************************************************************
 *	     WAYLANDDRV_DeleteDC
 */
static BOOL CDECL WAYLANDDRV_DeleteDC( PHYSDEV dev )
{
    WAYLANDDRV_PDEVICE *physDev = get_waylanddrv_dev( dev );

    free( physDev );
    return TRUE;
}


void add_device_bounds( WAYLANDDRV_PDEVICE *dev, const RECT *rect )
{
    RECT rc;

    if (!dev->bounds) return;
    if (dev->region && GetRgnBox( dev->region, &rc ))
    {
        if (IntersectRect( &rc, &rc, rect )) add_bounds_rect( dev->bounds, &rc );
    }
    else add_bounds_rect( dev->bounds, rect );
}

/***********************************************************************
 *           GetDeviceCaps    (WAYLANDDRV.@)
 */
static INT CDECL WAYLANDDRV_GetDeviceCaps( PHYSDEV dev, INT cap )
{
    switch(cap)
    {
    case HORZRES:
    {
        RECT primary_rect = get_primary_monitor_rect();
        return primary_rect.right - primary_rect.left;
    }
    case VERTRES:
    {
        RECT primary_rect = get_primary_monitor_rect();
        return primary_rect.bottom - primary_rect.top;
    }
    case DESKTOPHORZRES:
    {
        RECT virtual_rect = get_virtual_screen_rect();
        return virtual_rect.right - virtual_rect.left;
    }
    case DESKTOPVERTRES:
    {
        RECT virtual_rect = get_virtual_screen_rect();
        return virtual_rect.bottom - virtual_rect.top;
    }
    case BITSPIXEL:
        return 32;
    case SIZEPALETTE:
        return palette_size;
    default:
        dev = GET_NEXT_PHYSDEV( dev, pGetDeviceCaps );
        return dev->funcs->pGetDeviceCaps( dev, cap );
    }
}




/**********************************************************************
 *           WAYLANDDRV_wine_get_vulkan_driver
 */
static const struct vulkan_funcs * WAYLANDDRV_wine_get_vulkan_driver( UINT version )
{
    return get_vulkan_driver( version );
}



static const struct user_driver_funcs waylanddrv_funcs =
{


    .dc_funcs.pCreateCompatibleDC = WAYLANDDRV_CreateCompatibleDC,                    /* pCreateDC */
    .dc_funcs.pCreateDC = WAYLANDDRV_CreateDC,                    /* pCreateDC */
    .dc_funcs.pDeleteDC = WAYLANDDRV_DeleteDC,                    /* pDeleteDC */

    .dc_funcs.pGetDeviceCaps = WAYLANDDRV_GetDeviceCaps,


    .pwine_get_vulkan_driver = WAYLANDDRV_wine_get_vulkan_driver,   /* wine_get_vulkan_driver */
    .dc_funcs.priority = GDI_PRIORITY_GRAPHICS_DRV,

   .pActivateKeyboardLayout = WAYLANDDRV_ActivateKeyboardLayout,
    //.pBeep = WAYLANDDRV_Beep,
    .pChangeDisplaySettingsEx = WAYLANDDRV_ChangeDisplaySettingsEx,
    .pClipCursor = WAYLANDDRV_ClipCursor,
    //.pCreateDesktopWindow = WAYLANDDRV_CreateDesktopWindow,
    .pCreateWindow = WAYLANDDRV_CreateWindow,
    //.pDestroyCursorIcon = WAYLANDDRV_DestroyCursorIcon,
    .pDestroyWindow = WAYLANDDRV_DestroyWindow,
    .pEnumDisplaySettingsEx = WAYLANDDRV_EnumDisplaySettingsEx,
    .pUpdateDisplayDevices = WAYLANDDRV_UpdateDisplayDevices,
    .pGetCursorPos = WAYLANDDRV_GetCursorPos,
    //.pGetKeyboardLayoutList = WAYLANDDRV_GetKeyboardLayoutList,
    .pGetKeyNameText = WAYLANDDRV_GetKeyNameText,
    .pMapVirtualKeyEx = WAYLANDDRV_MapVirtualKeyEx,
    .pMsgWaitForMultipleObjectsEx = WAYLANDDRV_MsgWaitForMultipleObjectsEx,
    //.pRegisterHotKey = WAYLANDDRV_RegisterHotKey,
    //.pSetCapture = WAYLANDDRV_SetCapture,
    .pSetCursor = WAYLANDDRV_SetCursor,
    //.pSetCursorPos = WAYLANDDRV_SetCursorPos,
    //.pSetFocus = WAYLANDDRV_SetFocus,

    //.pSetParent = WAYLANDDRV_SetParent,
    //.pSetWindowRgn = WAYLANDDRV_SetWindowRgn,
    //.pSetWindowStyle = WAYLANDDRV_SetWindowStyle,
    //.pSetWindowText = WAYLANDDRV_SetWindowText,
    .pShowWindow = WAYLANDDRV_ShowWindow,
    .pSysCommand = WAYLANDDRV_SysCommand,
    //.pEnumDisplayMonitors = WAYLANDDRV_EnumDisplayMonitors,
    //.pGetMonitorInfo = WAYLANDDRV_GetMonitorInfo,

    .pToUnicodeEx = WAYLANDDRV_ToUnicodeEx,
    //.pUnregisterHotKey = WAYLANDDRV_UnregisterHotKey,
    //.pUpdateClipboard = WAYLANDDRV_UpdateClipboard,
    //.pUpdateLayeredWindow = WAYLANDDRV_UpdateLayeredWindow,
    .pVkKeyScanEx = WAYLANDDRV_VkKeyScanEx,
    //.pWindowMessage = WAYLANDDRV_WindowMessage,
    .pWindowPosChanged = WAYLANDDRV_WindowPosChanged,
    .pWindowPosChanging = WAYLANDDRV_WindowPosChanging,
};

void init_user_driver(void)
{
  __wine_set_user_driver( &waylanddrv_funcs, WINE_GDI_DRIVER_VERSION );
}
