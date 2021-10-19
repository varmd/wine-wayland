/*
 * X11 driver definitions
 *
 * Copyright 1996 Alexandre Julliard
 * Copyright 1999 Patrik Stridvall
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

#ifndef __WINE_WAYLANDDRV_H
#define __WINE_WAYLANDDRV_H

#ifndef __WINE_CONFIG_H
# error You must include config.h to use this header
#endif

#include <limits.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/input-event-codes.h>

//Wayland
#include <wayland-client.h>
#include <wayland-cursor.h>

typedef unsigned long Time;

#define FSHACK_TEST 1

//End Wayland

#define BOOL X_BOOL
#define BYTE X_BYTE
#define INT8 X_INT8
#define INT16 X_INT16
#define INT32 X_INT32
#define INT64 X_INT64

#undef BOOL
#undef BYTE
#undef INT8
#undef INT16
#undef INT32
#undef INT64
#undef LONG64

#undef Status  /* avoid conflict with wintrnl.h */
typedef int Status;
typedef int XVisualInfo;
typedef int XEvent;
typedef int XImage;
typedef int GC;
typedef int Display;
typedef int Window;
typedef int Drawable;
typedef int Colormap;

#include "windef.h"
#include "winbase.h"
#include "wingdi.h"
#include "winuser.h"
#include "wine/gdi_driver.h"
#include "wine/list.h"

#define MAX_DASHLEN 16

typedef struct {
    int shift;
    int scale;
    int max;
} ChannelShift;

typedef struct
{
    ChannelShift physicalRed, physicalGreen, physicalBlue;
    ChannelShift logicalRed, logicalGreen, logicalBlue;
} ColorShifts;

  /* X physical device */
typedef struct
{
    struct gdi_physdev dev;
    //GC            gc;          /* X Window GC */
    //Drawable      drawable;
    RECT          dc_rect;       /* DC rectangle relative to drawable */
    RECT         *bounds;        /* Graphics bounds */
    HRGN          region;        /* Device region (visible region & clip region) */
//    X_PHYSPEN     pen;
//    X_PHYSBRUSH   brush;
    int           depth;       /* bit depth of the DC */
    //ColorShifts  *color_shifts; /* color shifts of the DC */
    int           exposures;   /* count of graphics exposures operations */
} WAYLANDDRV_PDEVICE;


#define GAMMA_RAMP_SIZE 256

struct waylanddrv_gamma_ramp
{
    WORD red[GAMMA_RAMP_SIZE];
    WORD green[GAMMA_RAMP_SIZE];
    WORD blue[GAMMA_RAMP_SIZE];
};

static inline WAYLANDDRV_PDEVICE *get_waylanddrv_dev( PHYSDEV dev )
{
    return (WAYLANDDRV_PDEVICE *)dev;
}

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

void *fail_on_null(void *p, size_t size, char *file, int32_t line);

#define xstrdup(s) (fail_on_null(strdup(s), 0, __FILE__, __LINE__))



extern void set_surface_color_key( struct window_surface *window_surface, COLORREF color_key ) DECLSPEC_HIDDEN;
extern HRGN expose_surface( struct window_surface *window_surface, const RECT *rect ) DECLSPEC_HIDDEN;



//extern struct opengl_funcs *get_wgl_driver(UINT) DECLSPEC_HIDDEN;
extern const struct vulkan_funcs *get_vulkan_driver(UINT) DECLSPEC_HIDDEN;

/* X11 GDI palette driver */



/**************************************************************************
 * X11 USER driver
 */

struct waylanddrv_valuator_data
{
    double min;
    double max;
    int number;
};

struct waylanddrv_thread_data
{
    Display *display;

    HWND     grab_hwnd;            /* window that currently grabs the mouse */
    HWND     active_window;        /* active window */
    HWND     last_focus;           /* last window that had focus */
    

    unsigned long warp_serial;     /* serial number of last pointer warp request */
    //Window   clip_window;          /* window used for cursor clipping */
    HWND     clip_hwnd;            /* message window stored in desktop while clipping is active */
    DWORD    clip_reset;           /* time when clipping was last reset */
    HKL      kbd_layout;           /* active keyboard layout */
    

    struct waylanddrv_valuator_data x_rel_valuator;
    struct waylanddrv_valuator_data y_rel_valuator;

};

extern struct waylanddrv_thread_data *waylanddrv_init_thread_data(void) DECLSPEC_HIDDEN;
extern DWORD thread_data_tls_index DECLSPEC_HIDDEN;

static inline struct waylanddrv_thread_data *waylanddrv_thread_data(void)
{
    DWORD err = GetLastError();  /* TlsGetValue always resets last error */
    struct waylanddrv_thread_data *data = TlsGetValue( thread_data_tls_index );
    SetLastError( err );
    return data;
}

/* retrieve the thread display, or NULL if not created yet */
static inline Display *thread_display(void)
{
    struct waylanddrv_thread_data *data = waylanddrv_thread_data();
    if (!data) return NULL;
    return data->display;
}



static inline size_t get_property_size( int format, unsigned long count )
{
    /* format==32 means long, which can be 64 bits... */
    if (format == 32) return count * sizeof(long);
    return count * (format / 8);
}

extern Colormap default_colormap DECLSPEC_HIDDEN;

//extern Window root_window DECLSPEC_HIDDEN;
extern BOOL clipping_cursor DECLSPEC_HIDDEN;
extern unsigned int screen_bpp DECLSPEC_HIDDEN;
extern BOOL use_xkb DECLSPEC_HIDDEN;


extern BOOL use_take_focus DECLSPEC_HIDDEN;
extern BOOL use_primary_selection DECLSPEC_HIDDEN;
extern BOOL use_system_cursors DECLSPEC_HIDDEN;

extern BOOL grab_pointer DECLSPEC_HIDDEN;
extern BOOL grab_fullscreen DECLSPEC_HIDDEN;
extern BOOL usexcomposite DECLSPEC_HIDDEN;
extern BOOL managed_mode DECLSPEC_HIDDEN;
extern BOOL decorated_mode DECLSPEC_HIDDEN;
extern BOOL private_color_map DECLSPEC_HIDDEN;
extern int primary_monitor DECLSPEC_HIDDEN;
extern int copy_default_colors DECLSPEC_HIDDEN;
extern int alloc_system_colors DECLSPEC_HIDDEN;
extern int default_display_frequency DECLSPEC_HIDDEN;

extern HMODULE waylanddrv_module DECLSPEC_HIDDEN;
extern char *process_name DECLSPEC_HIDDEN;


//fshack
extern BOOL fs_hack_enabled(void) DECLSPEC_HIDDEN;
extern BOOL fs_hack_mapping_required(void) DECLSPEC_HIDDEN;
extern void fs_hack_set_real_mode(int width, int height) DECLSPEC_HIDDEN;
extern void fs_hack_set_current_mode(int width, int height) DECLSPEC_HIDDEN;
extern BOOL fs_hack_matches_real_mode(int w, int h) DECLSPEC_HIDDEN;
extern BOOL fs_hack_matches_current_mode(int w, int h) DECLSPEC_HIDDEN;
extern POINT fs_hack_current_mode(void) DECLSPEC_HIDDEN;
extern POINT fs_hack_real_mode(void) DECLSPEC_HIDDEN;
extern void fs_hack_user_to_real(POINT *pos) DECLSPEC_HIDDEN;
extern void fs_hack_real_to_user(POINT *pos) DECLSPEC_HIDDEN;
extern void fs_hack_real_to_user_relative(double *x, double *y) DECLSPEC_HIDDEN;

typedef BOOL (*waylanddrv_event_handler)( HWND hwnd /*, XEvent *event */ );

extern void WAYLANDDRV_register_event_handler( int type, waylanddrv_event_handler handler, const char *name ) DECLSPEC_HIDDEN;

extern BOOL WAYLANDDRV_ButtonPress( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_ButtonRelease( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_MotionNotify( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_EnterNotify( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_KeyEvent( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_KeymapNotify( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_DestroyNotify( HWND hwnd ) DECLSPEC_HIDDEN;


extern BOOL WAYLANDDRV_GenericEvent( HWND hwnd ) DECLSPEC_HIDDEN;


/* waylanddrv private window data */
struct waylanddrv_win_data
{
    Display    *display;        /* display connection for the thread owning the window */
    int vis;            /* X visual used by this window */
    Colormap    colormap;       /* colormap if non-default visual */
    HWND        hwnd;           /* hwnd that this private data belongs to */
    Window      whole_window;   /* X window for the complete window */
    Window      client_window;  /* X window for the client area */
    RECT        window_rect;    /* USER window rectangle relative to parent */
    RECT        whole_rect;     /* X window rectangle for the whole window relative to parent */
    RECT        client_rect;    /* client area relative to parent */

    BOOL        managed : 1;    /* is window managed? */
    BOOL        mapped : 1;     /* is window mapped? (in either normal or iconic state) */
    BOOL        iconic : 1;     /* is window in iconic state? */
    
    BOOL        shaped : 1;     /* is window using a custom region shape? */
    BOOL        layered : 1;    /* is window layered and with valid attributes? */
    BOOL        use_alpha : 1;  /* does window use an alpha channel? */
    int         wm_state;       /* current value of the WM_STATE property */
    DWORD       net_wm_state;   /* bit mask of active waylanddrv_net_wm_state values */

    unsigned long configure_serial; /* serial number of last configure request */
    struct window_surface *surface;

    unsigned long *icon_bits;
    unsigned int   icon_size;
};

//extern struct waylanddrv_win_data *get_win_data( HWND hwnd ) DECLSPEC_HIDDEN;
extern void release_win_data( struct waylanddrv_win_data *data ) DECLSPEC_HIDDEN;
extern Window WAYLANDDRV_get_whole_window( HWND hwnd ) DECLSPEC_HIDDEN;



extern void wine_vk_surface_destroy( HWND hwnd ) DECLSPEC_HIDDEN;

extern void wait_for_withdrawn_state( HWND hwnd, BOOL set ) DECLSPEC_HIDDEN;
extern void update_user_time( Time time ) DECLSPEC_HIDDEN;


extern void make_window_embedded( struct waylanddrv_win_data *data ) DECLSPEC_HIDDEN;
extern Window create_client_window( HWND hwnd, const XVisualInfo *visual ) DECLSPEC_HIDDEN;


extern BOOL update_clipboard( HWND hwnd ) DECLSPEC_HIDDEN;

static inline void mirror_rect( const RECT *window_rect, RECT *rect )
{
    int width = window_rect->right - window_rect->left;
    int tmp = rect->left;
    rect->left = width - rect->right;
    rect->right = width - tmp;
}



extern void WAYLANDDRV_InitClipboard(void) DECLSPEC_HIDDEN;
extern void CDECL WAYLANDDRV_SetFocus( HWND hwnd ) DECLSPEC_HIDDEN;
extern void set_window_cursor( Window window, HCURSOR handle ) DECLSPEC_HIDDEN;
extern void sync_window_cursor( Window window ) DECLSPEC_HIDDEN;
extern LRESULT clip_cursor_notify( HWND hwnd, HWND new_clip_hwnd ) DECLSPEC_HIDDEN;
extern void ungrab_clipping_window(void) DECLSPEC_HIDDEN;
extern void reset_clipping_window(void) DECLSPEC_HIDDEN;
extern BOOL clip_fullscreen_window( HWND hwnd, BOOL reset ) DECLSPEC_HIDDEN;
extern void move_resize_window( HWND hwnd, int dir ) DECLSPEC_HIDDEN;


extern POINT virtual_screen_to_root( INT x, INT y ) DECLSPEC_HIDDEN;
extern POINT root_to_virtual_screen( INT x, INT y ) DECLSPEC_HIDDEN;
extern RECT get_virtual_screen_rect(void) DECLSPEC_HIDDEN;
extern RECT get_primary_monitor_rect(void) DECLSPEC_HIDDEN;
extern void xinerama_init( unsigned int width, unsigned int height ) DECLSPEC_HIDDEN;

struct waylanddrv_mode_info
{
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    unsigned int refresh_rate;
};

extern void WAYLANDDRV_init_desktop( unsigned int width, unsigned int height ) DECLSPEC_HIDDEN;
extern void WAYLANDDRV_resize_desktop(unsigned int width, unsigned int height) DECLSPEC_HIDDEN;
extern BOOL is_desktop_fullscreen(void) DECLSPEC_HIDDEN;
extern BOOL create_desktop_win_data( Window win ) DECLSPEC_HIDDEN;
extern void WAYLANDDRV_Settings_AddDepthModes(void) DECLSPEC_HIDDEN;
extern void WAYLANDDRV_Settings_AddOneMode(unsigned int width, unsigned int height, unsigned int bpp, unsigned int freq) DECLSPEC_HIDDEN;
unsigned int WAYLANDDRV_Settings_GetModeCount(void) DECLSPEC_HIDDEN;
void WAYLANDDRV_Settings_Init(void) DECLSPEC_HIDDEN;
struct waylanddrv_mode_info *WAYLANDDRV_Settings_SetHandlers(const char *name,
                                                     int (*pNewGCM)(void),
                                                     LONG (*pNewSCM)(int),
                                                     unsigned int nmodes,
                                                     int reserve_depths) DECLSPEC_HIDDEN;




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

#endif  /* __WINE_WAYLANDDRV_H */
