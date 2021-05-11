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

//Wayland
#include <wayland-client.h>

static struct wl_compositor *wayland_compositor = NULL;
typedef unsigned long Time;
//#define OPENGL_TEST 1

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

#define WINE_XDND_VERSION 5



  /* X physical pen */


  /* X physical brush */
typedef struct
{
    int          style;
    int          fillStyle;
    int          pixel;
    //Pixmap       pixmap;
} X_PHYSBRUSH;

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

/* Wine driver X11 functions */

extern BOOL WAYLANDDRV_Arc( PHYSDEV dev, INT left, INT top, INT right,
                        INT bottom, INT xstart, INT ystart, INT xend, INT yend ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_Chord( PHYSDEV dev, INT left, INT top, INT right, INT bottom,
                          INT xstart, INT ystart, INT xend, INT yend ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_Ellipse( PHYSDEV dev, INT left, INT top, INT right, INT bottom ) DECLSPEC_HIDDEN;
extern INT WAYLANDDRV_EnumICMProfiles( PHYSDEV dev, ICMENUMPROCW proc, LPARAM lparam ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_ExtFloodFill( PHYSDEV dev, INT x, INT y, COLORREF color, UINT fillType ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_FillPath( PHYSDEV dev ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_GetDeviceGammaRamp( PHYSDEV dev, LPVOID ramp ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_GetICMProfile( PHYSDEV dev, LPDWORD size, LPWSTR filename ) DECLSPEC_HIDDEN;
extern DWORD WAYLANDDRV_GetImage( PHYSDEV dev, BITMAPINFO *info,
                              struct gdi_image_bits *bits, struct bitblt_coords *src ) DECLSPEC_HIDDEN;
extern COLORREF WAYLANDDRV_GetNearestColor( PHYSDEV dev, COLORREF color ) DECLSPEC_HIDDEN;
extern UINT WAYLANDDRV_GetSystemPaletteEntries( PHYSDEV dev, UINT start, UINT count, LPPALETTEENTRY entries ) DECLSPEC_HIDDEN;




extern BOOL WAYLANDDRV_Polygon( PHYSDEV dev, const POINT* pt, INT count ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_PolyPolygon( PHYSDEV dev, const POINT* pt, const INT* counts, UINT polygons) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_PolyPolyline( PHYSDEV dev, const POINT* pt, const DWORD* counts, DWORD polylines) DECLSPEC_HIDDEN;
extern DWORD WAYLANDDRV_PutImage( PHYSDEV dev, HRGN clip, BITMAPINFO *info,
                              const struct gdi_image_bits *bits, struct bitblt_coords *src,
                              struct bitblt_coords *dst, DWORD rop ) DECLSPEC_HIDDEN;
extern UINT WAYLANDDRV_RealizeDefaultPalette( PHYSDEV dev ) DECLSPEC_HIDDEN;
extern UINT WAYLANDDRV_RealizePalette( PHYSDEV dev, HPALETTE hpal, BOOL primary ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_Rectangle(PHYSDEV dev, INT left, INT top, INT right, INT bottom) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_RoundRect( PHYSDEV dev, INT left, INT top, INT right, INT bottom,
                              INT ell_width, INT ell_height ) DECLSPEC_HIDDEN;
extern HBRUSH WAYLANDDRV_SelectBrush( PHYSDEV dev, HBRUSH hbrush, const struct brush_pattern *pattern ) DECLSPEC_HIDDEN;
extern HPEN WAYLANDDRV_SelectPen( PHYSDEV dev, HPEN hpen, const struct brush_pattern *pattern ) DECLSPEC_HIDDEN;
extern COLORREF WAYLANDDRV_SetDCBrushColor( PHYSDEV dev, COLORREF crColor ) DECLSPEC_HIDDEN;
extern COLORREF WAYLANDDRV_SetDCPenColor( PHYSDEV dev, COLORREF crColor ) DECLSPEC_HIDDEN;
extern void WAYLANDDRV_SetDeviceClipping( PHYSDEV dev, HRGN rgn ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_SetDeviceGammaRamp( PHYSDEV dev, LPVOID ramp ) DECLSPEC_HIDDEN;
extern COLORREF WAYLANDDRV_SetPixel( PHYSDEV dev, INT x, INT y, COLORREF color ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_StretchBlt( PHYSDEV dst_dev, struct bitblt_coords *dst,
                               PHYSDEV src_dev, struct bitblt_coords *src, DWORD rop ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_StrokeAndFillPath( PHYSDEV dev ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_StrokePath( PHYSDEV dev ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_UnrealizePalette( HPALETTE hpal ) DECLSPEC_HIDDEN;

/* X11 driver internal functions */



extern DWORD copy_image_bits( BITMAPINFO *info, BOOL is_r8g8b8, XImage *image,
                              const struct gdi_image_bits *src_bits, struct gdi_image_bits *dst_bits,
                              struct bitblt_coords *coords, const int *mapping, unsigned int zeropad_mask ) DECLSPEC_HIDDEN;
/*
extern Pixmap create_pixmap_from_image( HDC hdc, const BITMAPINFO *info,
                                        const struct gdi_image_bits *bits, UINT coloruse ) DECLSPEC_HIDDEN;
extern DWORD get_pixmap_image( Pixmap pixmap, int width, int height, const XVisualInfo *vis,
                               BITMAPINFO *info, struct gdi_image_bits *bits ) DECLSPEC_HIDDEN;
*/

/*

extern struct window_surface *create_surface( Window window, const XVisualInfo *vis, const RECT *rect,
                                              COLORREF color_key, BOOL use_alpha ) DECLSPEC_HIDDEN;

*/

extern void set_surface_color_key( struct window_surface *window_surface, COLORREF color_key ) DECLSPEC_HIDDEN;
extern HRGN expose_surface( struct window_surface *window_surface, const RECT *rect ) DECLSPEC_HIDDEN;

extern RGNDATA *WAYLANDDRV_GetRegionData( HRGN hrgn, HDC hdc_lptodp ) DECLSPEC_HIDDEN;
extern BOOL add_extra_clipping_region( WAYLANDDRV_PDEVICE *dev, HRGN rgn ) DECLSPEC_HIDDEN;
extern void restore_clipping_region( WAYLANDDRV_PDEVICE *dev ) DECLSPEC_HIDDEN;
extern void add_device_bounds( WAYLANDDRV_PDEVICE *dev, const RECT *rect ) DECLSPEC_HIDDEN;

//extern void execute_rop( WAYLANDDRV_PDEVICE *physdev, Pixmap src_pixmap, GC gc, const RECT *visrect, DWORD rop ) DECLSPEC_HIDDEN;

extern BOOL WAYLANDDRV_SetupGCForPatBlt( WAYLANDDRV_PDEVICE *physDev, GC gc, BOOL fMapColors ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_SetupGCForBrush( WAYLANDDRV_PDEVICE *physDev ) DECLSPEC_HIDDEN;
extern INT WAYLANDDRV_XWStoDS( HDC hdc, INT width ) DECLSPEC_HIDDEN;
extern INT WAYLANDDRV_YWStoDS( HDC hdc, INT height ) DECLSPEC_HIDDEN;

extern BOOL client_side_graphics DECLSPEC_HIDDEN;
extern BOOL client_side_with_render DECLSPEC_HIDDEN;
extern BOOL shape_layered_windows DECLSPEC_HIDDEN;


extern struct opengl_funcs *get_wgl_driver(UINT) DECLSPEC_HIDDEN;
extern const struct vulkan_funcs *get_vulkan_driver(UINT) DECLSPEC_HIDDEN;

/* X11 GDI palette driver */

#define WAYLANDDRV_PALETTE_FIXED    0x0001 /* read-only colormap - have to use XAllocColor (if not virtual) */
#define WAYLANDDRV_PALETTE_VIRTUAL  0x0002 /* no mapping needed - pixel == pixel color */

#define WAYLANDDRV_PALETTE_PRIVATE  0x1000 /* private colormap, identity mapping */

extern UINT16 WAYLANDDRV_PALETTE_PaletteFlags DECLSPEC_HIDDEN;

extern int *WAYLANDDRV_PALETTE_PaletteToXPixel DECLSPEC_HIDDEN;
extern int *WAYLANDDRV_PALETTE_XPixelToPalette DECLSPEC_HIDDEN;
extern ColorShifts WAYLANDDRV_PALETTE_default_shifts DECLSPEC_HIDDEN;

extern int WAYLANDDRV_PALETTE_mapEGAPixel[16] DECLSPEC_HIDDEN;

extern int WAYLANDDRV_PALETTE_Init(void) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_IsSolidColor(COLORREF color) DECLSPEC_HIDDEN;

extern COLORREF WAYLANDDRV_PALETTE_ToLogical(WAYLANDDRV_PDEVICE *physDev, int pixel) DECLSPEC_HIDDEN;
extern int WAYLANDDRV_PALETTE_ToPhysical(WAYLANDDRV_PDEVICE *physDev, COLORREF color) DECLSPEC_HIDDEN;
extern COLORREF WAYLANDDRV_PALETTE_GetColor( WAYLANDDRV_PDEVICE *physDev, COLORREF color ) DECLSPEC_HIDDEN;

/* GDI escapes */

#define WAYLANDDRV_ESCAPE 6789
enum waylanddrv_escape_codes
{
    WAYLANDDRV_SET_DRAWABLE,     /* set current drawable for a DC */
    WAYLANDDRV_GET_DRAWABLE,     /* get current drawable for a DC */
    WAYLANDDRV_START_EXPOSURES,  /* start graphics exposures */
    WAYLANDDRV_END_EXPOSURES,    /* end graphics exposures */
    WAYLANDDRV_FLUSH_GL_DRAWABLE, /* flush changes made to the gl drawable */
    WAYLANDDRV_FLUSH_GDI_DISPLAY /* flush the gdi display */
};

struct waylanddrv_escape_set_drawable
{
    enum waylanddrv_escape_codes code;         /* escape code (WAYLANDDRV_SET_DRAWABLE) */
    Drawable                 drawable;     /* X drawable */
    int                      mode;         /* ClipByChildren or IncludeInferiors */
    RECT                     dc_rect;      /* DC rectangle relative to drawable */
};

struct waylanddrv_escape_get_drawable
{
    enum waylanddrv_escape_codes code;         /* escape code (WAYLANDDRV_GET_DRAWABLE) */
    Drawable                 drawable;     /* X drawable */
    Drawable                 gl_drawable;  /* GL drawable */
    int                      pixel_format; /* internal GL pixel format */
};

struct waylanddrv_escape_flush_gl_drawable
{
    enum waylanddrv_escape_codes code;         /* escape code (WAYLANDDRV_FLUSH_GL_DRAWABLE) */
    Drawable                 gl_drawable;  /* GL drawable */
    BOOL                     flush;        /* flush X11 before copying */
};

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
    //XEvent  *current_event;        /* event currently being processed */
    HWND     grab_hwnd;            /* window that currently grabs the mouse */
    HWND     active_window;        /* active window */
    HWND     last_focus;           /* last window that had focus */
    //XIM      xim;                  /* input method */
    HWND     last_xic_hwnd;        /* last xic window */
//    XFontSet font_set;             /* international text drawing font set */
    //Window   selection_wnd;        /* window used for selection interactions */
    unsigned long warp_serial;     /* serial number of last pointer warp request */
    //Window   clip_window;          /* window used for cursor clipping */
    HWND     clip_hwnd;            /* message window stored in desktop while clipping is active */
    DWORD    clip_reset;           /* time when clipping was last reset */
    HKL      kbd_layout;           /* active keyboard layout */
    enum { xi_unavailable = -1, xi_unknown, xi_disabled, xi_enabled } xi2_state; /* XInput2 state */
    void    *xi2_devices;          /* list of XInput2 devices (valid when state is enabled) */
    int      xi2_device_count;
    struct waylanddrv_valuator_data x_rel_valuator;
    struct waylanddrv_valuator_data y_rel_valuator;
    int      xi2_core_pointer;     /* XInput2 core pointer id */
    int      xi2_current_slave;    /* Current slave driving the Core pointer */
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
extern BOOL usexrandr DECLSPEC_HIDDEN;
extern BOOL usexvidmode DECLSPEC_HIDDEN;
//extern BOOL ximInComposeMode DECLSPEC_HIDDEN;
extern BOOL use_take_focus DECLSPEC_HIDDEN;
extern BOOL use_primary_selection DECLSPEC_HIDDEN;
extern BOOL use_system_cursors DECLSPEC_HIDDEN;
extern BOOL show_systray DECLSPEC_HIDDEN;
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
//extern int xrender_error_base DECLSPEC_HIDDEN;
extern HMODULE waylanddrv_module DECLSPEC_HIDDEN;
extern char *process_name DECLSPEC_HIDDEN;
extern Display *clipboard_display DECLSPEC_HIDDEN;

/* atoms */

/* X11 event driver */

typedef BOOL (*waylanddrv_event_handler)( HWND hwnd /*, XEvent *event */ );

extern void WAYLANDDRV_register_event_handler( int type, waylanddrv_event_handler handler, const char *name ) DECLSPEC_HIDDEN;

extern BOOL WAYLANDDRV_ButtonPress( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_ButtonRelease( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_MotionNotify( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_EnterNotify( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_KeyEvent( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_KeymapNotify( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_DestroyNotify( HWND hwnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_SelectionRequest( HWND hWnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_SelectionClear( HWND hWnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_MappingNotify( HWND hWnd ) DECLSPEC_HIDDEN;
extern BOOL WAYLANDDRV_GenericEvent( HWND hwnd ) DECLSPEC_HIDDEN;



/* X11 driver private messages, must be in the range 0x80001000..0x80001fff */
enum waylanddrv_window_messages
{
    WM_WAYLANDDRV_UPDATE_CLIPBOARD = 0x80001000,
    WM_WAYLANDDRV_SET_WIN_REGION,
    WM_WAYLANDDRV_RESIZE_DESKTOP,
    WM_WAYLANDDRV_SET_CURSOR,
    WM_WAYLANDDRV_CLIP_CURSOR
};

/* _NET_WM_STATE properties that we keep track of */
enum waylanddrv_net_wm_state
{
    NET_WM_STATE_FULLSCREEN,
    NET_WM_STATE_ABOVE,
    NET_WM_STATE_MAXIMIZED,
    NET_WM_STATE_SKIP_PAGER,
    NET_WM_STATE_SKIP_TASKBAR,
    NB_NET_WM_STATES
};

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
    //XIC         xic;            /* X input context */
    BOOL        managed : 1;    /* is window managed? */
    BOOL        mapped : 1;     /* is window mapped? (in either normal or iconic state) */
    BOOL        iconic : 1;     /* is window in iconic state? */
    BOOL        embedded : 1;   /* is window an XEMBED client? */
    BOOL        shaped : 1;     /* is window using a custom region shape? */
    BOOL        layered : 1;    /* is window layered and with valid attributes? */
    BOOL        use_alpha : 1;  /* does window use an alpha channel? */
    int         wm_state;       /* current value of the WM_STATE property */
    DWORD       net_wm_state;   /* bit mask of active waylanddrv_net_wm_state values */
    //Window      embedder;       /* window id of embedder */
    unsigned long configure_serial; /* serial number of last configure request */
    struct window_surface *surface;
    //Pixmap         icon_pixmap;
    //Pixmap         icon_mask;
    unsigned long *icon_bits;
    unsigned int   icon_size;
};

//extern struct waylanddrv_win_data *get_win_data( HWND hwnd ) DECLSPEC_HIDDEN;
extern void release_win_data( struct waylanddrv_win_data *data ) DECLSPEC_HIDDEN;
extern Window WAYLANDDRV_get_whole_window( HWND hwnd ) DECLSPEC_HIDDEN;

extern void sync_gl_drawable( HWND hwnd ) DECLSPEC_HIDDEN;
extern void set_gl_drawable_parent( HWND hwnd, HWND parent ) DECLSPEC_HIDDEN;
extern void destroy_gl_drawable( HWND hwnd ) DECLSPEC_HIDDEN;
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
//extern void WAYLANDDRV_InitKeyboard( Display *display ) DECLSPEC_HIDDEN;
extern DWORD CDECL WAYLANDDRV_MsgWaitForMultipleObjectsEx( DWORD count, const HANDLE *handles, DWORD timeout,
//extern void CDECL WAYLANDDRV_MsgWaitForMultipleObjectsEx( DWORD count, const HANDLE *handles, DWORD timeout,
                                                       DWORD mask, DWORD flags ) DECLSPEC_HIDDEN;

//typedef int (*waylanddrv_error_callback)( Display *display, XErrorEvent *event, void *arg );

//extern void WAYLANDDRV_expect_error( Display *display, waylanddrv_error_callback callback, void *arg ) DECLSPEC_HIDDEN;
extern int WAYLANDDRV_check_error(void) DECLSPEC_HIDDEN;
extern void WAYLANDDRV_X_to_window_rect( struct waylanddrv_win_data *data, RECT *rect ) DECLSPEC_HIDDEN;
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
