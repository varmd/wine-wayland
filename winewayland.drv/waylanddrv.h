/*
 * Wayland driver definitions
 *
 * Copyright 1996 Alexandre Julliard
 * Copyright 1999 Patrik Stridvall
 * Copyright 2020-2024 varmd
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

#ifndef __WINE_WAYLANDDRV_H
#define __WINE_WAYLANDDRV_H

#ifndef __WINE_CONFIG_H
# error You must include config.h to use this header
#endif

#include <limits.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>


#include "winternl.h"
#include "windef.h"
#include "winbase.h"

#include "ntgdi.h"
#include "wingdi.h"
#include "wine/gdi_driver.h"
#include "winuser.h"
#include "ntuser.h"

//Wayland
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <linux/input-event-codes.h>



//Wayland keyboard arrays
//https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

//End Wayland

#define MAX_DASHLEN 16

/* Externs for 6.22 */

extern LONG WAYLANDDRV_ChangeDisplaySettings(LPDEVMODEW displays, HWND hwnd, DWORD flags, LPVOID lpvoid);
extern BOOL WAYLANDDRV_UpdateDisplayDevices( const struct gdi_device_manager *device_manager,
  BOOL force, void *param );

extern BOOL WAYLANDDRV_GetCurrentDisplaySettings(LPCWSTR name, BOOL is_primary, LPDEVMODEW devmode);

extern BOOL WAYLANDDRV_CreateWindow(HWND hwnd);
extern void WAYLANDDRV_DestroyWindow(HWND hwnd);

extern UINT WAYLANDDRV_ShowWindow(HWND hwnd, INT cmd, RECT *rect, UINT swp);
extern LRESULT WAYLANDDRV_SysCommand(HWND hwnd, WPARAM wparam, LPARAM lparam);

extern BOOL WAYLANDDRV_WindowPosChanging(HWND hwnd, HWND insert_after, UINT swp_flags,
                                           const RECT *window_rect, const RECT *client_rect,
                                           RECT *visible_rect, struct window_surface **surface);
extern void WAYLANDDRV_WindowPosChanged(HWND hwnd, HWND insert_after, UINT swp_flags,
                                          const RECT *window_rect, const RECT *client_rect,
                                          const RECT *visible_rect, const RECT *valid_rects,
                                          struct window_surface *surface);

extern BOOL WAYLANDDRV_ClipCursor(const RECT *clip, BOOL reset);
extern BOOL WAYLANDDRV_GetCursorPos(LPPOINT pos);

extern void WAYLANDDRV_SetCursor(HWND hwnd, HCURSOR cursor);

extern SHORT WAYLANDDRV_VkKeyScanEx(WCHAR wChar, HKL hkl);
extern UINT WAYLANDDRV_MapVirtualKeyEx(UINT wCode, UINT wMapType, HKL hkl);
extern INT WAYLANDDRV_ToUnicodeEx(UINT virtKey, UINT scanCode, const BYTE *lpKeyState,
                                    LPWSTR bufW, int bufW_size, UINT flags, HKL hkl);

extern INT WAYLANDDRV_GetKeyNameText(LONG lparam, LPWSTR buffer, INT size);

extern NTSTATUS WAYLANDDRV_ProcessEvents( DWORD mask );



/* Externs for 6.22 */

/* FSR */
extern void fsr_set_real_mode(int width, int height);
extern void fsr_set_current_mode(int width, int height);
extern BOOL fsr_matches_real_mode(int w, int h);
extern BOOL fsr_matches_current_mode(int w, int h);
extern void fsr_real_to_user(POINT *pos);
extern void fsr_user_to_real(POINT *pos);

static inline void reset_bounds( RECT *bounds )
{
    bounds->left = bounds->top = INT_MAX;
    bounds->right = bounds->bottom = INT_MIN;
}

static inline void add_bounds_rect( RECT *bounds, const RECT *rect )
{
    if (rect->left >= rect->right || rect->top >= rect->bottom) return;
    bounds->left   = min( bounds->left, rect->left );
    bounds->top    = min( bounds->top, rect->top );
    bounds->right  = max( bounds->right, rect->right );
    bounds->bottom = max( bounds->bottom, rect->bottom );
}

#define xstrdup(s) (fail_on_null(strdup(s), 0, __FILE__, __LINE__))

extern const struct vulkan_funcs *get_vulkan_driver(UINT);

extern unsigned int screen_bpp;
extern unsigned int force_refresh;

extern HMODULE waylanddrv_module;
extern char *process_name;

extern void wine_vk_surface_destroy( HWND hwnd );

extern RECT get_virtual_screen_rect(void);
extern RECT get_primary_monitor_rect(void);

extern void xinerama_init( unsigned int width, unsigned int height );

extern BOOL is_desktop_fullscreen(void);

static inline BOOL is_window_rect_mapped( const RECT *rect )
{
    RECT virtual_rect = get_virtual_screen_rect();
    return (rect->left < virtual_rect.right &&
            rect->top < virtual_rect.bottom &&
            max( rect->right, rect->left + 1 ) > virtual_rect.left &&
            max( rect->bottom, rect->top + 1 ) > virtual_rect.top);
}

static inline BOOL is_window_rect_fullscreen( const RECT *rect )
{
    RECT primary_rect = get_primary_monitor_rect();
    return (rect->left <= primary_rect.left && rect->right >= primary_rect.right &&
            rect->top <= primary_rect.top && rect->bottom >= primary_rect.bottom);
}

static inline BOOL intersect_rect( RECT *dst, const RECT *src1, const RECT *src2 )
{
    dst->left   = max( src1->left, src2->left );
    dst->top    = max( src1->top, src2->top );
    dst->right  = min( src1->right, src2->right );
    dst->bottom = min( src1->bottom, src2->bottom );
    return !IsRectEmpty( dst );
}

/* string helpers */

static inline void ascii_to_unicode( WCHAR *dst, const char *src, size_t len )
{
    while (len--) *dst++ = (unsigned char)*src++;
}

static inline UINT asciiz_to_unicode( WCHAR *dst, const char *src )
{
    WCHAR *p = dst;
    while ((*p++ = *src++));
    return (p - dst) * sizeof(WCHAR);
}

#endif  /* __WINE_WAYLANDDRV_H */
