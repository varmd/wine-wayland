/*
 * Xinerama support
 *
 * Copyright 2006 Alexandre Julliard
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
#include <stdlib.h>



#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "rpc.h"
#include "winreg.h"
//#include "initguid.h"
//#include "devguid.h"
//#include "devpkey.h"
#include "setupapi.h"
#define WIN32_NO_STATUS
#include "winternl.h"
#include "wine/debug.h"
#include "wine/unicode.h"
#include "waylanddrv.h"

//WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);

static RECT virtual_screen_rect;

static MONITORINFOEXW default_monitor =
{
    sizeof(default_monitor),    /* cbSize */
    {0, 0, 1600, 900},             /* rcMonitor */
    { 0, 0, 1600, 900 },             /* rcWork */
    MONITORINFOF_PRIMARY,       /* dwFlags */
    { '\\','\\','.','\\','D','I','S','P','L','A','Y','1',0 }   /* szDevice */
};
//static const WCHAR monitor_deviceW[] = { '\\','\\','.','\\','D','I','S','P','L','A','Y','%','d',0 };

static MONITORINFOEXW *monitors;
static int nb_monitors;

static inline MONITORINFOEXW *get_primary(void)
{
    /* default to 0 if specified primary is invalid */
    return &monitors[0];
}

static inline HMONITOR index_to_monitor( int index )
{
    return (HMONITOR)(UINT_PTR)(index + 1);
}

static inline int monitor_to_index( HMONITOR handle )
{
    UINT_PTR index = (UINT_PTR)handle;
    if (index < 1 || index > nb_monitors) return -1;
    return index - 1;
}



static inline int query_screens(void)
{
    return 0;
}

POINT virtual_screen_to_root( INT x, INT y )
{
    POINT pt;
    pt.x = x - virtual_screen_rect.left;
    pt.y = y - virtual_screen_rect.top;
    return pt;
}

POINT root_to_virtual_screen( INT x, INT y )
{
    POINT pt;

    pt.x = x;
    pt.y = y;

    return pt;
}

RECT get_virtual_screen_rect(void)
{
    return virtual_screen_rect;
}

RECT get_primary_monitor_rect(void)
{
    return get_primary()->rcMonitor;
}

void xinerama_init( unsigned int width, unsigned int height )
{
    MONITORINFOEXW *primary;
    int i;
    RECT rect;

    SetRect( &rect, 0, 0, width, height );

    default_monitor.rcWork = default_monitor.rcMonitor = rect;

    nb_monitors = 1;
    monitors = &default_monitor;


    primary = get_primary();
    SetRectEmpty( &virtual_screen_rect );

    /* coordinates (0,0) have to point to the primary monitor origin */
    OffsetRect( &rect, -primary->rcMonitor.left, -primary->rcMonitor.top );
    for (i = 0; i < nb_monitors; i++)
    {
        OffsetRect( &monitors[i].rcMonitor, rect.left, rect.top );
        OffsetRect( &monitors[i].rcWork, rect.left, rect.top );
        UnionRect( &virtual_screen_rect, &virtual_screen_rect, &monitors[i].rcMonitor );
        /*
        TRACE( "monitor %p: %s work %s%s\n",
               index_to_monitor(i), wine_dbgstr_rect(&monitors[i].rcMonitor),
               wine_dbgstr_rect(&monitors[i].rcWork),
               (monitors[i].dwFlags & MONITORINFOF_PRIMARY) ? " (primary)" : "" );
        */
    }


}


/***********************************************************************
 *		WAYLANDDRV_GetMonitorInfo  (WAYLANDDRV.@)
 */
void WAYLANDDRV_UpdateDisplayDevices( const struct gdi_device_manager *device_manager, BOOL force, void *param ) {
    static int force_display_devices_refresh = TRUE;
    if (force || force_display_devices_refresh)
    {
        struct gdi_monitor gdi_monitor =
        {
            .rc_monitor = monitors[0].rcMonitor,
            .rc_work = monitors[0].rcWork,
            .state_flags = DISPLAY_DEVICE_ACTIVE | DISPLAY_DEVICE_ATTACHED,
        };
        device_manager->add_monitor( &gdi_monitor, param );
        force_display_devices_refresh = FALSE;
    }
}


/***********************************************************************
 *		WAYLANDDRV_EnumDisplayMonitors  (WAYLANDDRV.@)
 */
BOOL CDECL WAYLANDDRV_EnumDisplayMonitors( HDC hdc, LPRECT rect, MONITORENUMPROC proc, LPARAM lp )
{
    proc( index_to_monitor(0), 0, &monitors[0].rcMonitor, lp );
    return TRUE;
}
