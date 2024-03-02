/* WAYLANDDRV Vulkan+Wayland Implementation
 *
 * Copyright 2017 Roderick Colenbrander
 * Copyright 2018-2022 varmd (github.com/varmd)
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
#include <dlfcn.h>

#include "winternl.h"
#include "windef.h"
#include "winbase.h"
#include "wingdi.h"
#include "wine/gdi_driver.h"
#include "winuser.h"

//TODO
//#include "wine/heap.h"
#include "wine/server.h"
#include "wine/debug.h"

#include "wine/unixlib.h"
#include "wine/vulkan.h"
#include "wine/vulkan_driver.h"


#include "waylanddrv.h"

#define VK_NO_PROTOTYPES
#define WINE_VK_HOST

//latest version is 5
#define WINE_WAYLAND_SEAT_VERSION 5

#include "wayland-protocols/xdg-shell-client-protocol.h"
#include "wayland-protocols/pointer-constraints-unstable-v1-client-protocol.h"
#include "wayland-protocols/relative-pointer-unstable-v1-client-protocol.h"

WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);

#ifndef SONAME_LIBVULKAN
#define SONAME_LIBVULKAN ""
#endif

#ifdef _WIN64
  #define HAS_FSR 1
#endif

struct wl_compositor *wayland_compositor = NULL;
unsigned int global_wayland_confine = 0;
unsigned int global_wayland_full = 0;
unsigned long global_sx = 0;
unsigned long global_sy = 0;

int global_cursor_set = 0;
int global_cursor_gdi_fd = 0;
int global_cursor_last_change = 0;
int global_cursor_height = 0;
int global_cursor_width = 0;
int global_custom_cursors = 0;
HCURSOR global_last_cursor_handle = NULL;
void *global_cursor_shm_data = NULL;
struct wl_shm_pool *global_cursor_pool = NULL;
struct cursor_cache *global_cursor_cache[32768] = {0};
int global_wait_for_configure = 0;
int global_is_vulkan = 0;

int global_hide_cursor = 0;
int global_disable_clip_cursor = 0;
int global_fullscreen_grab_cursor = 0;
int global_last_cursor_change = 0;
int global_is_cursor_visible = 1;
int global_is_always_fullscreen = 0;
int global_fsr = 0;
int global_fsr_set = 0;

RECT global_vulkan_rect;
int global_vulkan_rect_flag = 0;

int global_gdi_fd = 0;
int global_gdi_size = 0;
int global_gdi_position_changing = 0;
int global_gdi_lb_hold = 0;

void *global_shm_data = NULL;
struct wl_buffer *global_wl_buffer = NULL;
struct wl_shm_pool *global_wl_pool = NULL;

HWND global_vulkan_hwnd = NULL;
HWND global_update_hwnd = NULL;
INPUT global_input;

//wl_output
struct wl_output *global_first_wl_output = NULL;
int global_output_width = 0;
int global_output_height = 0;

//Wayland defs
struct xdg_wm_base *wm_base = NULL;
static struct wl_seat *wayland_seat = NULL;
static struct wl_pointer *wayland_pointer = NULL;
static struct wl_keyboard *wayland_keyboard = NULL;
static struct zwp_pointer_constraints_v1 *pointer_constraints = NULL;
static struct zwp_relative_pointer_manager_v1 *relative_pointer_manager = NULL;
struct zwp_locked_pointer_v1 *locked_pointer = NULL;
struct zwp_confined_pointer_v1 *confined_pointer = NULL;
struct zwp_relative_pointer_v1 *relative_pointer = NULL;

struct wl_display *wayland_display = NULL;
struct wl_cursor_theme *wayland_cursor_theme;
struct wl_cursor       *wayland_default_cursor;
struct wl_surface *wayland_cursor_surface;
uint32_t wayland_serial_id = 0;
struct wl_shm *wayland_cursor_shm;
struct wl_shm *global_shm;
struct wl_subcompositor *wayland_subcompositor;

struct wayland_window {

	struct wl_surface *surface;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;

  HWND pointer_to_hwnd;
	int test;
	int height;
	int width;
};

struct wayland_window *vulkan_window = NULL;

struct wayland_window *gdi_window = NULL;

struct wl_surface_win_data
{
    HWND           hwnd;           /* hwnd that this private data belongs to */
    struct wl_subsurface  *wayland_subsurface;
    struct wl_surface     *wayland_surface;
    struct wayland_window     *wayland_window;
};

int is_buffer_busy = 0;
UINT desktop_tid = 0;


VkInstance *global_vk_instance = NULL;



#define ZWP_RELATIVE_POINTER_MANAGER_V1_VERSION 1


static const struct vulkan_funcs vulkan_funcs;
//Wayland

/*
  Examples
  https://github.com/SaschaWillems/Vulkan/blob/b4fb49504e714ecbd4485dfe98514a47b4e9c2cc/external/vulkan/vulkan_wayland.h
*/
#include "keycodes-inc.c"

/***********************************************************************
 *           WAYLAND_ToUnicodeEx
 */
INT WAYLANDDRV_ToUnicodeEx( UINT virt, UINT scan, const BYTE *state,
                               LPWSTR buf, int size, UINT flags, HKL hkl )
{



    WCHAR buffer[2];
    BOOL shift = state[VK_SHIFT] & 0x80;
    BOOL ctrl = state[VK_CONTROL] & 0x80;
    BOOL numlock = state[VK_NUMLOCK] & 0x01;

    buffer[0] = buffer[1] = 0;

    if (scan & 0x8000) return 0;  /* key up */

    /* FIXME: hardcoded layout */

    if (!ctrl)
    {
        switch (virt)
        {
        case VK_BACK:       buffer[0] = '\b'; break;
        case VK_OEM_1:      buffer[0] = shift ? ':' : ';'; break;
        case VK_OEM_2:      buffer[0] = shift ? '?' : '/'; break;
        case VK_OEM_3:      buffer[0] = shift ? '~' : '`'; break;
        case VK_OEM_4:      buffer[0] = shift ? '{' : '['; break;
        case VK_OEM_5:      buffer[0] = shift ? '|' : '\\'; break;
        case VK_OEM_6:      buffer[0] = shift ? '}' : ']'; break;
        case VK_OEM_7:      buffer[0] = shift ? '"' : '\''; break;
        case VK_OEM_COMMA:  buffer[0] = shift ? '<' : ','; break;
        case VK_OEM_MINUS:  buffer[0] = shift ? '_' : '-'; break;
        case VK_OEM_PERIOD: buffer[0] = shift ? '>' : '.'; break;
        case VK_OEM_PLUS:   buffer[0] = shift ? '+' : '='; break;
        case VK_RETURN:     buffer[0] = '\r'; break;
        case VK_SPACE:      buffer[0] = ' '; break;
        case VK_TAB:        buffer[0] = '\t'; break;
        case VK_MULTIPLY:   buffer[0] = '*'; break;
        case VK_ADD:        buffer[0] = '+'; break;
        case VK_SUBTRACT:   buffer[0] = '-'; break;
        case VK_DIVIDE:     buffer[0] = '/'; break;
        default:
            if (virt >= '0' && virt <= '9')
            {
                buffer[0] = shift ? ")!@#$%^&*("[virt - '0'] : virt;
                break;
            }
            if (virt >= 'A' && virt <= 'Z')
            {
                buffer[0] =  shift || (state[VK_CAPITAL] & 0x01) ? virt : virt + 'a' - 'A';
                break;
            }
            if (virt >= VK_NUMPAD0 && virt <= VK_NUMPAD9 && numlock && !shift)
            {
                buffer[0] = '0' + virt - VK_NUMPAD0;
                break;
            }
            if (virt == VK_DECIMAL && numlock && !shift)
            {
                buffer[0] = '.';
                break;
            }
            break;
        }
    }
    else /* Control codes */
    {
        if (virt >= 'A' && virt <= 'Z')
            buffer[0] = virt - 'A' + 1;
        else
        {
            switch (virt)
            {
            case VK_OEM_4:
                buffer[0] = 0x1b;
                break;
            case VK_OEM_5:
                buffer[0] = 0x1c;
                break;
            case VK_OEM_6:
                buffer[0] = 0x1d;
                break;
            case VK_SUBTRACT:
                buffer[0] = 0x1e;
                break;
            }
        }
    }

    lstrcpynW( buf, buffer, size );
    //TRACE( "returning %d / %s\n", strlenW( buffer ), debugstr_wn(buf, strlenW( buffer )));
    return lstrlenW( buffer );
}


/***********************************************************************
 *           WAYLAND_MapVirtualKeyEx
 */
UINT WAYLANDDRV_MapVirtualKeyEx( UINT code, UINT maptype, HKL hkl )
{
    UINT ret = 0;
    const char *s;
    char key;

    //TRACE( "code=%d %x, maptype=%d, hkl %p \n", code, code, maptype, hkl );

    switch (maptype)
    {
    case MAPVK_VK_TO_VSC_EX:
    case MAPVK_VK_TO_VSC:
        /* vkey to scancode */
        switch (code)
        {
        //case VK_LSHIFT:
        //case VK_RSHIFT:
        //case VK_SHIFT:
        //    code = VK_SHIFT;
        //    break;
        case VK_CONTROL:
            code = VK_CONTROL;
            break;
        case VK_MENU:
            code = VK_LMENU;
            break;
        }
        if (code < ( sizeof(vkey_to_scancode) / sizeof(vkey_to_scancode[0]) ) ) ret = vkey_to_scancode[code];
        break;
    case MAPVK_VSC_TO_VK:
    case MAPVK_VSC_TO_VK_EX:
        /* scancode to vkey */
        ret = scancode_to_vkey( code );
        if (maptype == MAPVK_VSC_TO_VK)
            switch (ret)
            {
            //case VK_LSHIFT:
            //case VK_RSHIFT:
            //case VK_SHIFT:
            //    ret = VK_SHIFT; break;
            case VK_LCONTROL:
            case VK_RCONTROL:
                ret = VK_CONTROL; break;
            case VK_LMENU:
            case VK_RMENU:
                ret = VK_MENU; break;
            }
        break;
    case MAPVK_VK_TO_CHAR:

        if ((code >= 0x30 && code <= 0x39) || (code >= 0x41 && code <= 0x5a))
        {
            key = code;
            if (code >= 0x41)
                key += 0x20;
            ret = toupper(key);
            //TRACE( "returning char code of %d \n", ret );
        } else {
          s = vkey_to_name( code );
          if (s && (strlen( s ) == 1))
              ret = s[0];
          else
              ret = 0;
        }

        break;
    default:
        FIXME( "Unknown maptype %d\n", maptype );
        break;
    }
    //TRACE( "returning 0x%04x   %x %d \n", ret, ret, ret );
    return ret;
}


/***********************************************************************
 *           WAYLAND_GetKeyboardLayout
 */
HKL WAYLANDDRV_GetKeyboardLayout( DWORD thread_id )
{
    return (HKL)MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

}

/***********************************************************************
 *           WAYLAND_VkKeyScanEx
 */
SHORT WAYLANDDRV_VkKeyScanEx( WCHAR ch, HKL hkl )
{
    //TRACE("%s \n", debugstr_w(ch));

    SHORT ret = -1;
    if (ch < sizeof(char_vkey_map) / sizeof(char_vkey_map[0])) ret = char_vkey_map[ch];
    return ret;
}


/***********************************************************************
 *           WAYLAND_GetKeyNameText
 */
INT WAYLANDDRV_GetKeyNameText( LONG lparam, LPWSTR buffer, INT size )
{
    int scancode, vkey;
    const char *name;
    char key[2];
    DWORD len;

    scancode = (lparam >> 16) & 0x1FF;
    vkey = scancode_to_vkey( scancode );


    TRACE( "scancode is %d %d\n", scancode, vkey);

    if (lparam & (1 << 25))
    {
        /* Caller doesn't care about distinctions between left and
           right keys. */
        switch (vkey)
        {
        case VK_LSHIFT:
        case VK_RSHIFT:
            vkey = VK_SHIFT; break;
        case VK_LCONTROL:
        case VK_RCONTROL:
            vkey = VK_CONTROL; break;
        case VK_LMENU:
        case VK_RMENU:
            vkey = VK_MENU; break;
        }
    }

    if (scancode & 0x100) vkey |= 0x100;

    if ((vkey >= 0x30 && vkey <= 0x39) || (vkey >= 0x41 && vkey <= 0x5a))
    {
        key[0] = vkey;
        if (vkey >= 0x41)
            key[0] += 0x20;
        key[1] = 0;
        name = key;
    }
    else
    {
        name = vkey_to_name( vkey );
    }

    RtlUTF8ToUnicodeN( buffer, size * sizeof(WCHAR), &len, name, strlen( name ) + 1 );
    len /= sizeof(WCHAR);
    if (len) len--;

    if (!len)
    {
        char name[16];
        len = sprintf( name, "Key 0x%02x", vkey );
        len = min( len + 1, size );
        ascii_to_unicode( buffer, name, len );
        if (len) buffer[--len] = 0;
    }

//    TRACE( "lparam 0x%08x -> %s\n", lparam, debugstr_w( buffer ));
    return len;
}


/***********************************************************************
 *		GetCursorPos (WAYLANDDRV.@)
 */

BOOL WAYLANDDRV_GetCursorPos(LPPOINT pos)
{

    if(global_wayland_confine) {
      return TRUE;
    }

    if(!global_sx) {
      pos->x = 0;
      pos->y = 0;
      return TRUE;
    }

    pos->x = global_sx;
    pos->y = global_sy;

    //TRACE( "Global pointer at %d \n", pos->x, pos->y );

    return TRUE;


}

//End Wayland keyboard arrays and funcs

#define VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR 1000006000;

typedef struct VkWaylandSurfaceCreateInfoKHR {
    VkStructureType                   sType;
    const void*                       pNext;
    VkWaylandSurfaceCreateFlagsKHR flags;
    struct wl_display*                display;
    struct wl_surface*                surface;
} VkWaylandSurfaceCreateInfoKHR;


static VkResult (*pvkCreateInstance)(const VkInstanceCreateInfo *, const VkAllocationCallbacks *, VkInstance *);
static VkResult (*pvkCreateSwapchainKHR)(VkDevice, const VkSwapchainCreateInfoKHR *, const VkAllocationCallbacks *, VkSwapchainKHR *);
static VkResult (*pvkCreateWaylandSurfaceKHR)(VkInstance, const VkWaylandSurfaceCreateInfoKHR *, const VkAllocationCallbacks *, VkSurfaceKHR *);

static void (*pvkDestroyInstance)(VkInstance, const VkAllocationCallbacks *);
static void (*pvkDestroySurfaceKHR)(VkInstance, VkSurfaceKHR, const VkAllocationCallbacks *);
static void (*pvkDestroySwapchainKHR)(VkDevice, VkSwapchainKHR, const VkAllocationCallbacks *);

static VkResult (*pvkEnumerateInstanceExtensionProperties)(const char *, uint32_t *, VkExtensionProperties *);

static void * (*pvkGetDeviceProcAddr)(VkDevice, const char *);
static void * (*pvkGetInstanceProcAddr)(VkInstance, const char *);



static VkBool32 (*pvkGetPhysicalDeviceWaylandPresentationSupportKHR)(VkPhysicalDevice, uint32_t, struct wl_display *);
static VkResult (*pvkGetSwapchainImagesKHR)(VkDevice, VkSwapchainKHR, uint32_t *, VkImage *);
static VkResult (*pvkQueuePresentKHR)(VkQueue, const VkPresentInfoKHR *);

static void *vulkan_handle;

static void wine_vk_init(void)
{

    vulkan_handle = dlopen(SONAME_LIBVULKAN, RTLD_NOW);

    if (!vulkan_handle)
    {
        ERR("Failed to load vulkan library\n");
        return;
    }

#define LOAD_FUNCPTR(f) if (!(p##f = dlsym(vulkan_handle, #f))) goto fail;
#define LOAD_OPTIONAL_FUNCPTR(f) p##f = dlsym(vulkan_handle, #f);
    LOAD_FUNCPTR(vkCreateInstance);
    LOAD_FUNCPTR(vkCreateSwapchainKHR);
    LOAD_FUNCPTR(vkCreateWaylandSurfaceKHR);
    LOAD_FUNCPTR(vkDestroyInstance);
    LOAD_FUNCPTR(vkDestroySurfaceKHR);
    LOAD_FUNCPTR(vkDestroySwapchainKHR);
    LOAD_FUNCPTR(vkEnumerateInstanceExtensionProperties);
    LOAD_FUNCPTR(vkGetDeviceProcAddr);
    LOAD_FUNCPTR(vkGetInstanceProcAddr);


    LOAD_FUNCPTR(vkGetPhysicalDeviceWaylandPresentationSupportKHR);
    LOAD_FUNCPTR(vkGetSwapchainImagesKHR);
    LOAD_FUNCPTR(vkQueuePresentKHR);

#undef LOAD_FUNCPTR
#undef LOAD_OPTIONAL_FUNCPTR

    return;

fail:
    TRACE("VULKAN closed \n");
    dlclose(vulkan_handle);
    vulkan_handle = NULL;
    return;
}



static struct wl_surface_win_data *wl_surface_data_context[32768] = {0};

static inline int context_wl_idx( struct wl_surface *wl_surface  )
{
    return LOWORD( wl_surface ) >> 1;
}

static struct wl_surface_win_data *alloc_wl_win_data( struct wl_surface *surface, HWND hwnd, struct wayland_window *window )
{
    struct wl_surface_win_data *data;

    if ((data = calloc(1, sizeof(*data) ) ))
    {
        TRACE("Surface %d \n", context_wl_idx( surface ));

        data->hwnd = hwnd;
        //data->wayland_subsurface = surface;
        data->wayland_surface = surface;
        data->wayland_window = window;
        wl_surface_data_context[context_wl_idx(surface)] = data;

    }
    return data;
}


/***********************************************************************
 *           free_win_data
 */
static void free_wl_win_data( struct wl_surface_win_data *data )
{
    wl_surface_data_context[context_wl_idx( data->wayland_surface )] = NULL;
    //LeaveCriticalSection( &win_data_section );
    free( data );
}

/***********************************************************************
 *           get_wl_win_data
 *
 * Lock and return the data structure associated with a window.
 */
static struct wl_surface_win_data *get_wl_win_data( struct wl_surface *surface )
{
    struct wl_surface_win_data *data;

    if (!surface) return NULL;
    if ((data = wl_surface_data_context[context_wl_idx(surface)])) {
      return data;
    }
    return NULL;
}

//Cursors and cursor cache
struct cursor_cache
{
    HCURSOR handle;           /* cursor that this cache belongs to */
    uint32_t *cached_pixels;
    int width;
    int height;
    int xhotspot;
    int yhotspot;
};

static inline int cursor_idx( HCURSOR handle  )
{
    return LOWORD( handle ) >> 1;
}

static void alloc_cursor_cache( HCURSOR handle )
{
    struct cursor_cache *data;

    if ((data = calloc(1, sizeof(*data))))
    {
        global_cursor_cache[cursor_idx(handle)] = data;

    }
}

//End Cursor cache

//GDI win data

struct gdi_win_data
{
    HWND           hwnd;           /* hwnd that this private data belongs to */
    HWND           parent;         /* parent hwnd for child windows */
    RECT           window_rect;    /* USER window rectangle relative to parent */
    RECT           whole_rect;     /* X window rectangle for the whole window relative to parent */
    RECT           client_rect;    /* client area relative to parent */
    struct wayland_window *window;         /* native window wrapper that forwards calls to the desktop process */
    struct window_surface *surface;

    struct wl_subsurface  *wayland_subsurface;
    struct wl_surface     *wayland_surface;
    void                  *shm_data;
    struct wl_shm_pool    *wl_pool;
    struct wl_buffer      *buffer;
    int                   gdi_fd;
    int                   surface_changed;
    int                   size_changed;
    int                   window_width;
    int                   window_height;
    int                   buffer_busy;
    int                   size;
};

static struct gdi_win_data *win_data_context[32768];

static void set_surface_region( struct window_surface *window_surface, HRGN win_region );

struct gdi_window_surface
{
    struct window_surface header;
    HWND                  hwnd;
    struct wayland_window *window;

    struct wl_subsurface  *wayland_subsurface;
    struct wl_surface     *wayland_surface;
    void                  *shm_data;
    struct wl_shm_pool    *wl_pool;
    int                   gdi_fd;
    RECT                  bounds;
    RGNDATA              *region_data;
    HRGN                  region;
    BYTE                  alpha;
    COLORREF              color_key;
    void                 *bits;
    CRITICAL_SECTION      crit;
    BITMAPINFO            info;   /* variable size, must be last */

};

// listeners
static struct gdi_win_data *get_win_data( HWND hwnd );


static void buffer_release(void *data, struct wl_buffer *buffer) {

  HWND hwnd = data;
  struct gdi_win_data *hwnd_data;

  if ( hwnd != NULL) {
    hwnd_data = get_win_data( hwnd );
    if(hwnd_data)
      hwnd_data->buffer_busy = 0;
  }


  is_buffer_busy = 0;
  //wl_buffer_destroy(buffer);
}

static const struct wl_buffer_listener buffer_listener = {
	buffer_release
};

void wayland_pointer_enter_cb(void *data,
		struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface,
		wl_fixed_t sx, wl_fixed_t sy)
{

  HWND temp;

  #if 0
  struct wl_surface_win_data *hwnd_data;

  TRACE("Surface %p \n", surface );
  TRACE("Surface %d \n", context_wl_idx( surface ));

  if ( hwnd_data = get_wl_win_data( surface )) {
    TRACE("Data found \n");
    TRACE("Current hwnd is %p and surface %p %p \n", hwnd_data->hwnd, hwnd_data->wayland_subsurface, surface);
  }
  #endif

  wayland_serial_id = serial;


  temp = wl_surface_get_user_data(surface);
  if(temp) {

    TRACE("Current hwnd is %p and surface %p \n", temp, surface);
    global_update_hwnd = temp;

    if (NtUserGetAncestor(temp, GA_PARENT) == NtUserGetDesktopWindow()) {
      NtUserSetFocus(temp);
      NtUserSetActiveWindow(temp);
      NtUserSetForegroundWindow(temp);
    }

  } else if (vulkan_window != NULL && vulkan_window->surface == surface && global_vulkan_hwnd != NULL) {

      //TRACE("Current vulkan hwnd is %p and surface %p \n", global_vulkan_hwnd, surface);

      NtUserSetActiveWindow( global_vulkan_hwnd );
      NtUserSetForegroundWindow( global_vulkan_hwnd );
      NtUserSetFocus(global_vulkan_hwnd);
  }
  global_last_cursor_change = 0;

  //Remove cursor if it's hidden on alt-tab
  if(!global_is_cursor_visible) {
    wl_pointer_set_cursor(wayland_pointer, wayland_serial_id, NULL, 0, 0);
  }

}

void wayland_pointer_leave_cb(void *data,
		struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface)
{

}

void wayland_pointer_motion_cb_vulkan(void *data,
		struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{

  POINT point;
  POINT client_point;

  if(global_wayland_confine) {
    return;
  }

  global_input.mi.dx = wl_fixed_to_int(sx);
  global_input.mi.dy = wl_fixed_to_int(sy);


  //It takes some time before correct rect is returned
  if(global_vulkan_rect_flag < 3) {
    global_vulkan_rect_flag++;
    NtUserGetWindowRect(global_vulkan_hwnd, &global_vulkan_rect);
  }

  //SetWindowPos crashes Unity if used elsewhere
  if(global_vulkan_rect.left != 0 || global_vulkan_rect.top != 0) {
    NtUserSetWindowPos( global_vulkan_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOSENDCHANGING);
    NtUserGetWindowRect(global_vulkan_hwnd, &global_vulkan_rect);
  }

  if(global_fsr) {

    //RECT title_rect;

    client_point.x = global_vulkan_rect.left;
    client_point.y = global_vulkan_rect.top;
    if(client_point.x != 0 || client_point.y != 0) {
      fsr_user_to_real(&client_point);
    }
    global_input.mi.dx = global_input.mi.dx + client_point.x;
    global_input.mi.dy = global_input.mi.dy + client_point.y;

    point.x = global_input.mi.dx;
    point.y = global_input.mi.dy;

    fsr_real_to_user(&point);
    global_input.mi.dx = point.x;
    global_input.mi.dy = point.y;

    #if 0
    TRACE("Motion (x y - x y %d %d %d %d) %s %s  \n",
      wl_fixed_to_int(sx),
      wl_fixed_to_int(sy),
      global_input.mi.dx,
      global_input.mi.dy,
      wine_dbgstr_rect( &global_vulkan_rect ),
      wine_dbgstr_rect( &title_rect )
    );
    #endif

  } else {
    global_input.mi.dx = global_input.mi.dx + global_vulkan_rect.left;
    global_input.mi.dy = global_input.mi.dy + global_vulkan_rect.top;
  }

  global_sx = global_input.mi.dx;
  global_sy = global_input.mi.dy;

  SERVER_START_REQ( send_hardware_message )
    {
      req->win        = wine_server_user_handle( global_vulkan_hwnd );
      req->flags      = 0;
      req->input.type = INPUT_MOUSE;
      req->input.mouse.x     = global_input.mi.dx;
      req->input.mouse.y     = global_input.mi.dy;
      req->input.mouse.data  = 0;
      req->input.mouse.flags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
      req->input.mouse.time  = 0;
      req->input.mouse.info  = 0;
      wine_server_call( req );
    }
  SERVER_END_REQ;

}

int global_last_sx, global_last_sy = 0;


void wayland_pointer_motion_cb(void *data,
		struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{

  HWND hwnd;
  RECT rect;

  if(global_vulkan_hwnd) {
    return wayland_pointer_motion_cb_vulkan(data, pointer, time, sx, sy);
  }


  #if 0
  if(global_gdi_position_changing > 0) {
    if(global_gdi_position_changing == 1) {
      global_last_sx = 0;
      global_last_sy = 0;
      global_gdi_position_changing = 2;
      global_sx = wl_fixed_to_int(sx);
      global_sy = wl_fixed_to_int(sy);
      return;
    } else if(global_gdi_position_changing == 2) {
      global_last_sx = wl_fixed_to_int(sx) - global_sx;
      global_last_sy = wl_fixed_to_int(sy) - global_sy;
    }

  }
  #endif

  global_input.mi.dx = wl_fixed_to_int(sx);
  global_input.mi.dy = wl_fixed_to_int(sy);

  global_sx = global_input.mi.dx;
  global_sy = global_input.mi.dy;




  hwnd = global_update_hwnd;


  NtUserGetWindowRect(hwnd, &rect);

  global_input.mi.dx = global_input.mi.dx + rect.left;
  global_input.mi.dy = global_input.mi.dy + rect.top;

  #if 0

  TRACE("Motion x y %d %d %s hwnd %p \n",
    wl_fixed_to_int(sx), wl_fixed_to_int(sy),
    wine_dbgstr_rect( &rect ),
    hwnd);
  #endif

  #if 0
  if(global_gdi_position_changing == 2) {
    global_input.mi.dx = global_last_sx + rect.left;
    global_input.mi.dx = global_last_sy + rect.top;

    TRACE("Rel. Motion x y %d %d  \n", global_last_sx, global_last_sy );
  }
  #endif

  //TRACE("Motion x y %d %d \n", global_sx, global_sy);

  SERVER_START_REQ( send_hardware_message )
  {
    req->win        = wine_server_user_handle( hwnd );
    req->flags      = 0;
    req->input.type = INPUT_MOUSE;
    req->input.mouse.x     = global_input.mi.dx;
    req->input.mouse.y     = global_input.mi.dy;
    req->input.mouse.data  = 0;
    req->input.mouse.flags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    req->input.mouse.time  = 0;
    req->input.mouse.info  = 0;
    wine_server_call( req );
  }
  SERVER_END_REQ;

}

void wayland_pointer_button_cb_vulkan(void *data,
		struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button,
		uint32_t state)
{

  HWND hwnd;


  INPUT input;
  input.type = INPUT_MOUSE;

  input.mi.dx          = (int)global_sx;
  input.mi.dy          = (int)global_sy;
  input.mi.mouseData   = 0;
  input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

  if(global_wayland_confine) {
    input.mi.dx = 0;
    input.mi.dy = 0;
    input.mi.dwFlags = 0;
  }

  hwnd = global_vulkan_hwnd;


  //TRACE("Button code %p \n", button);

  switch (button)
	{
	case BTN_LEFT:
    if(state == WL_POINTER_BUTTON_STATE_PRESSED) {
      input.mi.dwFlags  |= MOUSEEVENTF_LEFTDOWN;
    } else if(state == WL_POINTER_BUTTON_STATE_RELEASED) {
      input.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
    }
		break;

	case BTN_MIDDLE:
		if(state == WL_POINTER_BUTTON_STATE_PRESSED)
      input.mi.dwFlags     |= MOUSEEVENTF_MIDDLEDOWN;
    else if(state == WL_POINTER_BUTTON_STATE_RELEASED)
      input.mi.dwFlags     |= MOUSEEVENTF_MIDDLEUP;
		break;

	case BTN_RIGHT:
		if(state == WL_POINTER_BUTTON_STATE_PRESSED)
      input.mi.dwFlags     |= MOUSEEVENTF_RIGHTDOWN;
    else if(state == WL_POINTER_BUTTON_STATE_RELEASED)
      input.mi.dwFlags     |= MOUSEEVENTF_RIGHTUP;
		break;

  case BTN_EXTRA:
  case BTN_FORWARD:
    TRACE("Forward Click \n");
		if(state == WL_POINTER_BUTTON_STATE_PRESSED)
      input.mi.dwFlags     |= MOUSEEVENTF_XDOWN;
    else if(state == WL_POINTER_BUTTON_STATE_RELEASED)
      input.mi.dwFlags     |= MOUSEEVENTF_XUP;

    input.mi.mouseData = XBUTTON1;
		break;

  case BTN_BACK:
  case BTN_SIDE:
    TRACE("Back Click \n");
		if(state == WL_POINTER_BUTTON_STATE_PRESSED)
      input.mi.dwFlags     |= MOUSEEVENTF_XDOWN;
    else if(state == WL_POINTER_BUTTON_STATE_RELEASED)
      input.mi.dwFlags     |= MOUSEEVENTF_XUP;

    input.mi.mouseData = XBUTTON2;
		break;


	default:
		break;
	}



    SERVER_START_REQ( send_hardware_message )
  {
    req->win        = wine_server_user_handle( hwnd );
    req->flags      = 0;
    req->input.type = input.type;
    req->input.mouse.x     = input.mi.dx;
    req->input.mouse.y     = input.mi.dy;
    req->input.mouse.data  = 0;
    req->input.mouse.flags = input.mi.dwFlags;
    req->input.mouse.time  = 0;
    req->input.mouse.info  = 0;

    wine_server_call( req );
  }
  SERVER_END_REQ;



}

void wayland_pointer_button_cb(void *data,
		struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button,
		uint32_t state)
{

  HWND hwnd;
  RECT rect;
  INPUT input;

  //Support running without WINE_VK_VULKAN_ONLY
  if(global_vulkan_hwnd) {
    return wayland_pointer_button_cb_vulkan(data, pointer, serial, time, button, state);
  }

  input.type = INPUT_MOUSE;
  input.mi.dx          = (int)global_sx;
  input.mi.dy          = (int)global_sy;
  input.mi.mouseData   = 0;
  input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;



  if(global_wayland_confine) {
    input.mi.dx = 0;
    input.mi.dy = 0;
    input.mi.dwFlags = 0;
  }


  switch (button)
	{
  	case BTN_LEFT:
      if(state == WL_POINTER_BUTTON_STATE_PRESSED) {
        input.mi.dwFlags  |= MOUSEEVENTF_LEFTDOWN;
        global_gdi_lb_hold = 1;
      } else if(state == WL_POINTER_BUTTON_STATE_RELEASED) {
        input.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
        global_gdi_lb_hold = 0;
      }
  		break;

  	case BTN_MIDDLE:
  		if(state == WL_POINTER_BUTTON_STATE_PRESSED)
        input.mi.dwFlags     |= MOUSEEVENTF_MIDDLEDOWN;
      else if(state == WL_POINTER_BUTTON_STATE_RELEASED)
        input.mi.dwFlags     |= MOUSEEVENTF_MIDDLEUP;
  		break;

  	case BTN_RIGHT:
  		if(state == WL_POINTER_BUTTON_STATE_PRESSED) {
        input.mi.dwFlags     |= MOUSEEVENTF_RIGHTDOWN;
      } else if(state == WL_POINTER_BUTTON_STATE_RELEASED) {
        input.mi.dwFlags     |= MOUSEEVENTF_RIGHTUP;
      }
  		break;


  	default:
  		break;
	}

  hwnd = global_update_hwnd;

  NtUserGetWindowRect(global_update_hwnd, &rect);

  TRACE("Click x y %d %d %s \n", input.mi.dx, input.mi.dy, wine_dbgstr_rect( &rect ));

  input.mi.dx = input.mi.dx + rect.left;
  input.mi.dy = input.mi.dy + rect.top;

  TRACE("Click x y %d %d %s \n", input.mi.dx, input.mi.dy, wine_dbgstr_rect( &rect ));

  SERVER_START_REQ( send_hardware_message )
  {
    req->win        = wine_server_user_handle( hwnd );
    req->flags      = 0;
    req->input.type = input.type;
    req->input.mouse.x     = input.mi.dx;
    req->input.mouse.y     = input.mi.dy;
    req->input.mouse.data  = 0;
    req->input.mouse.flags = input.mi.dwFlags;
    req->input.mouse.time  = 0;
    req->input.mouse.info  = 0;

    wine_server_call( req );
  }
  SERVER_END_REQ;

}

static void wayland_pointer_frame_cb(void *data, struct wl_pointer *wl_pointer) {
  //do nothing

}
static void wayland_pointer_axis_source_cb(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source)	{
  TRACE("Pointer axis source  \n");
  //do nothing
}
static void wayland_pointer_axis_stop_cb(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis)	{
  TRACE("Pointer axis stop  \n");
  //do nothing
}


static void wayland_pointer_axis_discrete_cb(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete) {
  TRACE("Motion Wheel discrete %d \n", discrete);
}

//Mouse wheel
void wayland_pointer_axis_cb(void *data,
		struct wl_pointer *pointer, uint32_t time, uint32_t axis,
		wl_fixed_t value)
{

  SERVER_START_REQ( send_hardware_message )
  {
        req->win        = wine_server_user_handle( global_vulkan_hwnd );
        req->flags      = 0;
        req->input.type = INPUT_MOUSE;

        req->input.mouse.x     = global_sx;
        req->input.mouse.y     = global_sy;
        req->input.mouse.data  = value > 0 ? -WHEEL_DELTA : WHEEL_DELTA;
        req->input.mouse.flags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_WHEEL;

        if(global_wayland_confine) {
          req->input.mouse.flags = MOUSEEVENTF_WHEEL;
          req->input.mouse.x     = 0;
          req->input.mouse.y     = 0;
        }
        req->input.mouse.time  = 0;
        req->input.mouse.info  = 0;

        wine_server_call( req );
  }
  SERVER_END_REQ;
}

//relative pointer for locked surface
static void
relative_pointer_handle_motion(void *data, struct zwp_relative_pointer_v1 *pointer,
			       uint32_t utime_hi,
			       uint32_t utime_lo,
			       wl_fixed_t dx,
			       wl_fixed_t dy,
			       wl_fixed_t dx_unaccel,
			       wl_fixed_t dy_unaccel)
{

    INPUT input;

    input.type = INPUT_MOUSE;

    input.mi.mouseData   = 0;

    input.mi.time        = 0;
    input.mi.dwExtraInfo = 0;

    input.mi.dwFlags     = MOUSEEVENTF_MOVE;

    //Slows mouse
    #if 0
    if(global_fsr) {
      POINT fsr_point;
      double x, y;
      x = wl_fixed_to_double(dx);
      y = wl_fixed_to_double(dy);
      fsr_real_to_user_relative(&x, &y);
      input.mi.dx = x;
      input.mi.dy = y;
    } else {
      input.mi.dx = wl_fixed_to_double(dx);
      input.mi.dy = wl_fixed_to_double(dy);
    }
    #endif
    input.mi.dx = wl_fixed_to_double(dx);
    input.mi.dy = wl_fixed_to_double(dy);

    __wine_send_input( global_vulkan_hwnd, &input, NULL );
}

static const struct zwp_relative_pointer_v1_listener relative_pointer_listener = {
	relative_pointer_handle_motion,
};
//relative pointer for locked surface

void grab_wayland_screen(void) {
  if(!global_wayland_confine) {

    global_wayland_confine = 1;
    locked_pointer = zwp_pointer_constraints_v1_lock_pointer( pointer_constraints,  vulkan_window->surface, wayland_pointer, NULL,ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);

    relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(relative_pointer_manager, wayland_pointer);
    zwp_relative_pointer_v1_add_listener(relative_pointer, &relative_pointer_listener, NULL);

    wl_surface_commit(vulkan_window->surface);
  }
}

void ungrab_wayland_screen(void) {
  if(global_wayland_confine) {

    if(locked_pointer)
      zwp_locked_pointer_v1_destroy(locked_pointer);
    if(relative_pointer)
      zwp_relative_pointer_v1_destroy(relative_pointer);

    locked_pointer = NULL;
    relative_pointer = NULL;

    global_wayland_confine = 0;

  }
}


void wayland_keyboard_keymap_cb(void *data,
		struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size)
{
}

void wayland_keyboard_enter_cb(void *data,
		struct wl_keyboard *keyboard, uint32_t serial,
		struct wl_surface *surface, struct wl_array *keys)
{
  TRACE( "keyboard_event: Entered \n" );
}

void wayland_keyboard_leave_cb(void *data,
		struct wl_keyboard *keyboard, uint32_t serial,
		struct wl_surface *surface)
{
}



//https://stackoverflow.com/questions/8161741/handling-keyboard-input-in-win32-wm-char-or-wm-keydown-wm-keyup
//https://stackoverflow.com/questions/44897991/wm-keydown-repeat-count
void wayland_keyboard_key_cb (void *data, struct wl_keyboard *keyboard,
		    uint32_t serial, uint32_t time, uint32_t keycode,
		    uint32_t state)
{
  INPUT input;
  HWND hwnd;

  //TRACE( "keyboard_event: %u keycode \n",  keycode );

  //if ((unsigned int)keycode >= sizeof(keycode_to_vkey)/sizeof(keycode_to_vkey[0]) || !keycode_to_vkey[keycode]) {
        //TRACE( "keyboard_event: code %u unmapped key, ignoring \n",  keycode );
  //}

  input.type             = INPUT_KEYBOARD;
  input.ki.wVk         = keycode_to_vkey[(unsigned int)keycode];
  if(!input.ki.wVk) {
    return;
  }
  input.ki.wScan       = vkey_to_scancode[(int)input.ki.wVk];
  input.ki.time        = 0;
  input.ki.dwExtraInfo = 0;
  input.ki.dwFlags     = (input.ki.wScan & 0x100) ? KEYEVENTF_EXTENDEDKEY : 0;


  //TRACE("keyboard_event: code %u vkey %x scan %x meta %x \n",
  //                        keycode, input.ki.wVk, input.ki.wScan, state );


  input.type = INPUT_KEYBOARD;
  hwnd = global_update_hwnd;

  if(global_vulkan_hwnd) {
    hwnd = global_vulkan_hwnd;
  }

  if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
    input.ki.dwFlags |= KEYEVENTF_KEYUP;
  }

  __wine_send_input( hwnd, &input, NULL );

  if(state != WL_KEYBOARD_KEY_STATE_RELEASED) {
    return;
  }

	switch (keycode)
	{


    case KEY_F11:

      if(!global_wayland_full) {
        global_wait_for_configure = 1;
        xdg_toplevel_set_fullscreen(vulkan_window->xdg_toplevel, NULL);
        wl_surface_commit(vulkan_window->surface);
        wl_display_flush (wayland_display);
        while(global_wait_for_configure) {
          sleep(0.3);
          wl_display_dispatch(wayland_display);
        }
        global_wayland_full = 1;
      }

      break;

    case KEY_F10:

      if(!global_wayland_confine) {
        global_wayland_confine = 1;

        if(vulkan_window) {
          TRACE("Enabling grab \n");
          grab_wayland_screen();
          TRACE("Enabling grab done \n");

        } else if(gdi_window) {
          confined_pointer = zwp_pointer_constraints_v1_confine_pointer( pointer_constraints,  gdi_window->surface, wayland_pointer, NULL, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);


          relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(relative_pointer_manager, wayland_pointer);
          zwp_relative_pointer_v1_add_listener(relative_pointer, &relative_pointer_listener, NULL);
          wl_surface_commit(gdi_window->surface);

        }

        //hide cursor
        wl_pointer_set_cursor(wayland_pointer, wayland_serial_id, NULL, 0, 0);
        wl_surface_commit(wayland_cursor_surface);

    } else {
      zwp_locked_pointer_v1_destroy(locked_pointer);
	    zwp_relative_pointer_v1_destroy(relative_pointer);
	    locked_pointer = NULL;
	    relative_pointer = NULL;
      global_wayland_confine = 0;
    }


    break;


    case KEY_F8: //lock pointer
      if(vulkan_window)
        xdg_toplevel_set_minimized(vulkan_window->xdg_toplevel);
      else if(gdi_window && gdi_window->xdg_toplevel)
        xdg_toplevel_set_minimized(gdi_window->xdg_toplevel);
    break;
    case KEY_F9: //lock pointer



    if(!global_wayland_confine) {

      global_wayland_confine = 1;


      locked_pointer = zwp_pointer_constraints_v1_lock_pointer( pointer_constraints,  vulkan_window->surface, wayland_pointer, NULL,ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);



      relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(relative_pointer_manager, wayland_pointer);
      zwp_relative_pointer_v1_add_listener(relative_pointer, &relative_pointer_listener, NULL);

      wl_pointer_set_cursor(wayland_pointer, wayland_serial_id,
        NULL, 0, 0);


      wl_surface_commit(vulkan_window->surface);



    } else {
      if(confined_pointer)
        zwp_confined_pointer_v1_destroy(confined_pointer);
      if(relative_pointer)
        zwp_relative_pointer_v1_destroy(relative_pointer);
	    locked_pointer = NULL;
	    relative_pointer = NULL;
      global_wayland_confine = 0;
    }


    break; //end F9

    default:

    break;

	}


}

void wayland_keyboard_modifiers_cb(void *data,
		struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed,
		uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
}

void wayland_keyboard_repeat_info(void* data, struct wl_keyboard *wl_keyboard, int rate, int delay)
{
}


static void seat_handle_name(void *data, struct wl_seat *wl_seat,
		 const char *name)
{
}


static const struct wl_pointer_listener pointer_listener_gdi =
      {   wayland_pointer_enter_cb,
          wayland_pointer_leave_cb,
          wayland_pointer_motion_cb,
          wayland_pointer_button_cb,
          wayland_pointer_axis_cb,
          wayland_pointer_frame_cb,
          wayland_pointer_axis_source_cb,
          wayland_pointer_axis_stop_cb,
          wayland_pointer_axis_discrete_cb,
      };

static const struct wl_pointer_listener pointer_listener_vulkan =
      {   wayland_pointer_enter_cb,
          wayland_pointer_leave_cb,
          wayland_pointer_motion_cb_vulkan,
          wayland_pointer_button_cb_vulkan,
          wayland_pointer_axis_cb,
          wayland_pointer_frame_cb,
          wayland_pointer_axis_source_cb,
          wayland_pointer_axis_stop_cb,
          wayland_pointer_axis_discrete_cb,
      };

static void seat_caps_cb(void *data, struct wl_seat *seat, enum wl_seat_capability caps)
{
	char *is_vulkan;
	char *env_hide_cursor;
	char *env_fullscreen_grab_cursor;
	char *env_no_clip_cursor;
	char *env_use_custom_cursors;
	char *env_use_fsr;


  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !wayland_pointer)
	{

    wayland_pointer = wl_seat_get_pointer(seat);
    is_vulkan = getenv( "WINE_VK_VULKAN_ONLY" );

    //Some games want to use their cursor
    env_hide_cursor = getenv( "WINE_VK_HIDE_CURSOR" );

    //Some games want to grab cursor when ClipCursor is passed fullscreen rect
    env_fullscreen_grab_cursor = getenv( "WINE_VK_FULLSCREEN_GRAB_CURSOR" );

    //Some games need ClipCursor disabled
    env_no_clip_cursor = getenv( "WINE_VK_NO_CLIP_CURSOR" );

    //Some games use custom cursors
    env_use_custom_cursors = getenv( "WINE_VK_USE_CUSTOM_CURSORS" );

    //Upscale
    env_use_fsr = getenv( "WINE_VK_USE_FSR" );

    if(env_hide_cursor) {
      global_hide_cursor = 1;
    }

    if(env_no_clip_cursor) {
      global_disable_clip_cursor = 1;
    }

    if(env_fullscreen_grab_cursor) {
      global_fullscreen_grab_cursor = 1;
    }

    if(env_use_custom_cursors) {
      global_custom_cursors = 1;
    }

    if(env_use_fsr) {
      global_fsr = 1;
      global_fsr_set = 1;
      global_is_always_fullscreen = 1; //enable fullscreen for FSR
    }

    if(!is_vulkan && !global_is_vulkan) {
      wl_pointer_add_listener(wayland_pointer, &pointer_listener_gdi, NULL);
    } else {
      wl_pointer_add_listener(wayland_pointer, &pointer_listener_vulkan, NULL);
    }

	}
	else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && wayland_pointer)
	{
		wl_pointer_destroy(wayland_pointer);
		wayland_pointer = NULL;
	}

	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !wayland_keyboard)
	{
		wayland_keyboard = wl_seat_get_keyboard(seat);
		static const struct wl_keyboard_listener keyboard_listener =
		{   wayland_keyboard_keymap_cb,
        wayland_keyboard_enter_cb,
        wayland_keyboard_leave_cb,
        wayland_keyboard_key_cb,
				wayland_keyboard_modifiers_cb,
				wayland_keyboard_repeat_info,
				};

		wl_keyboard_add_listener(wayland_keyboard, &keyboard_listener, NULL);
	}
	else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && wayland_keyboard)
	{
		wl_keyboard_destroy(wayland_keyboard);
		wayland_keyboard = NULL;
	}
}

void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
}

struct wl_shm_listener shm_listener = {
	shm_format
};


static void xdg_wm_base_ping(void *data, struct xdg_wm_base *wm_base, uint32_t serial)
{
    xdg_wm_base_pong(wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    xdg_wm_base_ping,
};


//wl output

static void
display_handle_geometry(void *data,
			struct wl_output *wl_output,
			int x,
			int y,
			int physical_width,
			int physical_height,
			int subpixel,
			const char *make,
			const char *model,
			int transform)
{
	//Do nothing
}

static void
display_handle_mode(void *data,
		    struct wl_output *wl_output,
		    uint32_t flags,
		    int width,
		    int height,
		    int refresh)
{

	if (global_first_wl_output && wl_output == global_first_wl_output && (flags & WL_OUTPUT_MODE_CURRENT)) {
		global_output_width = width;
		global_output_height = height;
    TRACE("Found output with WxH %d %d \n", global_output_width, global_output_height);
	}
}

static void display_handle_done(void *data, struct wl_output *wl_output)
{

}

static void display_handle_scale(void *data, struct wl_output *wl_output,
                                int32_t scale)
{
}

static const struct wl_output_listener output_listener = {
	display_handle_geometry,
	display_handle_mode,
	display_handle_done,
	display_handle_scale,
};


static void registry_add_object (void *data, struct wl_registry *registry, uint32_t name, const char *wl_interface, uint32_t version) {

  const char *cursor_theme;
  const char *cursor_size_str;
  int cursor_size = 32;

	if (!strcmp(wl_interface,"wl_compositor")) {
		wayland_compositor = wl_registry_bind (registry, name, &wl_compositor_interface, 4);
    //Sway calls wl_shm before wl_compositor
    if(wayland_compositor && !wayland_cursor_surface) {
      wayland_cursor_surface = wl_compositor_create_surface(wayland_compositor);
    }
	} else if (strcmp(wl_interface, "wl_subcompositor") == 0) {
		wayland_subcompositor = wl_registry_bind(registry,
					 name, &wl_subcompositor_interface, 1);
	}
  else if (strcmp(wl_interface, "xdg_wm_base") == 0) {
		wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 2);
		xdg_wm_base_add_listener(wm_base, &xdg_wm_base_listener, NULL);
	}
  else if (!strcmp(wl_interface, "wl_seat"))
	{
		wayland_seat = (struct wl_seat *) wl_registry_bind(registry, name, &wl_seat_interface, WINE_WAYLAND_SEAT_VERSION);

		static const struct wl_seat_listener seat_listener =
		{ seat_caps_cb, seat_handle_name };
		wl_seat_add_listener(wayland_seat, &seat_listener, data);
	} else if (strcmp(wl_interface, "zwp_pointer_constraints_v1") == 0) {
      pointer_constraints = wl_registry_bind(registry, name,
                         &zwp_pointer_constraints_v1_interface,
                         1);
    } else if (strcmp(wl_interface, "zwp_relative_pointer_manager_v1") == 0) {
		  relative_pointer_manager = wl_registry_bind(registry, name,
		    &zwp_relative_pointer_manager_v1_interface, 1);
    } else if (strcmp(wl_interface, "wl_shm") == 0) {


      cursor_theme = getenv("XCURSOR_THEME");
      cursor_size_str = getenv("XCURSOR_SIZE");
      if (cursor_size_str) {
        cursor_size = atoi(cursor_size_str);
      }
      if (!cursor_theme) {
        cursor_theme = NULL;
      }

      global_shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
      wl_shm_add_listener(global_shm, &shm_listener, NULL);

		  wayland_cursor_shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
		  wayland_cursor_theme = wl_cursor_theme_load(cursor_theme, cursor_size, wayland_cursor_shm);

		  wayland_default_cursor = 	wl_cursor_theme_get_cursor(wayland_cursor_theme, "left_ptr");
      //Sway calls wl_shm before wl_compositor
      if(wayland_compositor && !wayland_cursor_surface)
        wayland_cursor_surface = wl_compositor_create_surface(wayland_compositor);

    } else if (strcmp(wl_interface, "wl_output") == 0) {
		  global_first_wl_output = wl_registry_bind(registry, name, &wl_output_interface, 2);
		  wl_output_add_listener(global_first_wl_output, &output_listener, NULL);
	}
}

static void registry_remove_object (void *data, struct wl_registry *registry, uint32_t name) {

}
static struct wl_registry_listener registry_listener = {&registry_add_object, &registry_remove_object};




static void
handle_xdg_surface_configure(void *data, struct xdg_surface *surface,
			 uint32_t serial)
{
  TRACE( "Surface configured \n" );
  global_wait_for_configure = 0;
  xdg_surface_ack_configure(surface, serial);
}


static void
handle_xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
			  int32_t width, int32_t height,
			  struct wl_array *states)
{
	//do nothing
}

static void
handle_xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel)
{

}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	handle_xdg_toplevel_configure,
	handle_xdg_toplevel_close,
};

static const struct xdg_surface_listener xdg_surface_listener = {
	handle_xdg_surface_configure
};


/* store the display fd into the message queue */
static void set_queue_display_fd( int esync_fd )
{
    HANDLE handle;
    static int done = 0;
    int ret;

    if(done) {
      return;
    }

    done = 1;

    if (wine_server_fd_to_handle( esync_fd, GENERIC_READ | SYNCHRONIZE, 0, &handle ))
    {
        TRACE( "waylanddrv: Can't allocate handle for display fd\n" );
        exit(1);
    }
    SERVER_START_REQ( set_queue_fd )
    {
        req->handle = wine_server_obj_handle( handle );
        ret = wine_server_call( req );
    }
    SERVER_END_REQ;
    if (ret)
    {
        MESSAGE( "waylanddrv: Can't store handle for display fd\n" );
        exit(1);
    }
    NtClose( handle );
}

static void create_wayland_display (void) {
  int fd = 0;
  char *env_is_always_fullscreen;
  struct wl_registry *registry = NULL;

  if(desktop_tid)
    return;

  desktop_tid = GetCurrentThreadId();

  wayland_display = wl_display_connect (NULL);
  if(!wayland_display) {
    TRACE("wayland display is not working \n");
    exit(1);
    return;
  }

  //Automate fullscreen
  env_is_always_fullscreen = getenv( "WINE_VK_ALWAYS_FULLSCREEN" );

  if(env_is_always_fullscreen) {
    TRACE("Is always fullscreen \n");
    global_is_always_fullscreen = 1;
  }

  registry = wl_display_get_registry (wayland_display);
  wl_registry_add_listener (registry, &registry_listener, NULL);

  wl_display_roundtrip (wayland_display);
  wl_display_roundtrip (wayland_display);
  wl_display_roundtrip (wayland_display);

  fd = wl_display_get_fd(wayland_display);
  if(fd) {
    set_queue_display_fd(fd);
  }
  TRACE("Created wayland display thread id %d \n", desktop_tid);
}

//todo add delete
static struct wayland_window *create_wayland_window (HWND hwnd, int32_t width, int32_t height) {

  //struct wl_region *region;
  struct wayland_window *window = malloc(sizeof(struct wayland_window));

  global_wait_for_configure = 1;

	window->surface = wl_compositor_create_surface (wayland_compositor);
  window->xdg_surface = xdg_wm_base_get_xdg_surface(wm_base, window->surface);
	xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);

	window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
	xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener, window);

  /*
  region = wl_compositor_create_region(wayland_compositor);
  wl_region_add(region, 0, 0, width, height);
  wl_surface_set_opaque_region(window->surface, region);
  */

  window->pointer_to_hwnd = hwnd;

  window->width = width;
  window->height = height;

  if(global_is_always_fullscreen)
    xdg_toplevel_set_fullscreen(window->xdg_toplevel, NULL);

  wl_surface_commit(window->surface);
  wl_display_flush (wayland_display);
  while(global_wait_for_configure) {
    sleep(0.3);
    wl_display_dispatch(wayland_display);
  }

  alloc_wl_win_data(window->surface, hwnd, window);

  TRACE("Created wayland window %p \n", window);

  return window;

}

static void delete_wayland_window (struct wayland_window *window) {

  struct wl_surface_win_data *data;
  data = get_wl_win_data(window->surface);
  free_wl_win_data(data);

  TRACE("Deleting wayland window %p \n", window);

  if (window->xdg_toplevel)
		xdg_toplevel_destroy(window->xdg_toplevel);
	if (window->xdg_surface)
		xdg_surface_destroy(window->xdg_surface);

	wl_surface_destroy (window->surface);
  wl_display_dispatch(wayland_display);


  window = NULL;
  desktop_tid = 0;

}


static void draw_gdi_wayland_window (struct wayland_window *window) {

    RECT rect;
    struct wl_region *region;
    int screen_width = 1600;
    int screen_height = 900;
    char *env_width = NULL;
    char *env_height = NULL;
    struct wl_buffer *buffer = NULL;
    int stride = 0;

    if(!wayland_display) {
      return;
    }

    if(is_buffer_busy) {
      return;
    }

    TRACE( "Creating/Resetting main gdi wayland surface \n" );

    env_width = getenv( "WINE_VK_WAYLAND_WIDTH" );
    env_height = getenv( "WINE_VK_WAYLAND_HEIGHT" );

    NtUserGetWindowRect(window->pointer_to_hwnd, &rect);

    if(env_width) {
      screen_width = atoi(env_width);
    }
    if(env_height) {
      screen_height = atoi(env_height);
    }

    TRACE( "creating gdi window for hwnd %p wxh %dx%d \n", window->pointer_to_hwnd, screen_width, screen_height );

    stride = screen_width * 4; // 4 bytes per pixel
    global_gdi_size = stride * screen_height;

    if(!global_gdi_fd) {
      TRACE( "creating gdi fd \n" );

      global_gdi_fd = memfd_create("wine-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
      if (global_gdi_fd >= 0) {
        fcntl(global_gdi_fd, F_ADD_SEALS, F_SEAL_SHRINK);
      } else {
        exit(1);
      }
      posix_fallocate(global_gdi_fd, 0, global_gdi_size);

    }


    //MAP_SHARED
    if(!global_shm_data) {

      void *shm_data = mmap(NULL, global_gdi_size, PROT_READ | PROT_WRITE, MAP_SHARED, global_gdi_fd, 0);



      if (shm_data == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %m\n");
        close(global_gdi_fd);
        return;

      } else {
        global_shm_data = shm_data;
      }

      TRACE( "creating wl_shm_data \n" );
    }






    if(!global_wl_pool) {
      TRACE( "creating wl_pool \n" );
      global_wl_pool = wl_shm_create_pool(global_shm, global_gdi_fd, global_gdi_size);
    }


    is_buffer_busy = 1;

    buffer = wl_shm_pool_create_buffer(global_wl_pool, 0, screen_width, screen_height, stride, WL_SHM_FORMAT_ARGB8888);
    wl_buffer_add_listener(buffer, &buffer_listener, NULL);
    wl_surface_attach(window->surface, buffer, 0, 0);



    region = wl_compositor_create_region(wayland_compositor);
    wl_region_add(region, 0, 0, 1, 1);
    wl_surface_set_input_region(window->surface, region);

    wl_surface_damage(window->surface, 0, 0, screen_width, screen_height);
    wl_surface_commit(window->surface);

}




/***********************************************************************
 *		ClipCursor (WAYLANDDRV.@)
 */
BOOL WAYLANDDRV_ClipCursor( const RECT *clip, BOOL reset )
{
    RECT virtual_rect = get_virtual_screen_rect();

    if(!global_is_vulkan || global_disable_clip_cursor) {
      return TRUE;
    }

    if (!clip ) {
      TRACE( "Release Mouse Capture Called \n" );

      if(global_wayland_confine) {


        struct wl_cursor_image *image;
        struct wl_buffer *buffer;

        zwp_locked_pointer_v1_destroy(locked_pointer);
        zwp_relative_pointer_v1_destroy(relative_pointer);
        locked_pointer = NULL;
        relative_pointer = NULL;

        global_wayland_confine = 0;

        //show mouse if it's not hidden by env variable
        if(!global_hide_cursor  && !global_custom_cursors) {
          image = wayland_default_cursor->images[0];
          buffer = wl_cursor_image_get_buffer(image);
          wl_pointer_set_cursor(wayland_pointer, wayland_serial_id,
            wayland_cursor_surface,
            image->hotspot_x,
            image->hotspot_y);
            wl_surface_attach(wayland_cursor_surface, buffer, 0, 0);
            wl_surface_damage(wayland_cursor_surface, 0, 0,
            image->width, image->height);
          wl_surface_commit(wayland_cursor_surface);
        }

      }

      return TRUE;

    }

    // we are clipping if the clip rectangle is smaller than the screen
    if (clip->left > virtual_rect.left || clip->right < virtual_rect.right ||
        clip->top > virtual_rect.top || clip->bottom < virtual_rect.bottom)
    {
        TRACE( "Set Mouse Capture %s \n", wine_dbgstr_rect(clip) );
        grab_wayland_screen();



    } else // if currently clipping, check if we should switch to fullscreen clipping
    {

      if ( global_fullscreen_grab_cursor && !global_is_cursor_visible ) { //grab cursor if clip equals to desktop
        TRACE( "Set Mouse Capture - fullscreen grab \n" );
        grab_wayland_screen();
        return TRUE;
      } else if ( global_fullscreen_grab_cursor && global_is_cursor_visible ) {
        TRACE( "Remove Mouse Capture - fullscreen grab \n" );
        ungrab_wayland_screen();
        TRACE( "Remove Mouse Capture - fullscreen grab done \n" );
        return TRUE;
      }

      //Release grab instead
      if(!global_hide_cursor) {

        TRACE( "Release Mouse Capture #2 \n" );
        ungrab_wayland_screen();

      }
    }


    return TRUE;


}



static uint32_t *get_bitmap_argb( HDC hdc, HBITMAP color, HBITMAP mask, unsigned int *width, unsigned int *height )
{
    char buffer[FIELD_OFFSET( BITMAPINFO, bmiColors[256] )];
    BITMAPINFO *info = (BITMAPINFO *)buffer;
    BITMAP bm;
    uint32_t *ptr, *bits = NULL;
    unsigned char *mask_bits = NULL;
    int i, j;
    BOOL has_alpha = FALSE;
    int red, green, blue, alpha;
    int cClrBits = 0;
    unsigned int width_bytes = 0;

    if (!color) return NULL;

    if (!NtGdiExtGetObjectW( color, sizeof(bm), &bm )) return NULL;
    info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info->bmiHeader.biWidth = bm.bmWidth;
    info->bmiHeader.biHeight = -bm.bmHeight;
    info->bmiHeader.biPlanes = 1;
    info->bmiHeader.biBitCount = 32;
    info->bmiHeader.biCompression = BI_RGB;
    info->bmiHeader.biSizeImage = bm.bmWidth * bm.bmHeight * 4;
    info->bmiHeader.biXPelsPerMeter = 0;
    info->bmiHeader.biYPelsPerMeter = 0;
    info->bmiHeader.biClrUsed = 0;
    info->bmiHeader.biClrImportant = 0;



    // Convert the color format to a count of bits.
    cClrBits = (WORD)(bm.bmPlanes * bm.bmBitsPixel);
    if (cClrBits == 1)
        cClrBits = 1;
    else if (cClrBits <= 4)
        cClrBits = 4;
    else if (cClrBits <= 8)
        cClrBits = 8;
    else if (cClrBits <= 16)
        cClrBits = 16;
    else if (cClrBits <= 24)
        cClrBits = 24;
    else cClrBits = 32;

    TRACE("Got %d format for cursor \n", cClrBits);


    if (!(bits = malloc( bm.bmWidth * bm.bmHeight * sizeof(unsigned int) )))
        goto failed;
//    if (!GetDIBits( hdc, color, 0, bm.bmHeight, bits, info, DIB_RGB_COLORS )) goto failed;
    if (!NtGdiGetDIBitsInternal( hdc, color, 0, bm.bmHeight,
      bits, info, DIB_RGB_COLORS , 0, 0)) goto failed;

    *width = bm.bmWidth;
    *height = bm.bmHeight;

    for (i = 0; i < bm.bmWidth * bm.bmHeight; i++)
        if ((has_alpha = (bits[i] & 0xff000000) != 0)) break;

    ptr = bits;

    if (!has_alpha)
    {
        TRACE("No alpha channel for cursor \n");
        width_bytes = (bm.bmWidth + 31) / 32 * 4;
        /* generate alpha channel from the mask */
        info->bmiHeader.biBitCount = 1;
        info->bmiHeader.biSizeImage = width_bytes * bm.bmHeight;
        if (!(mask_bits = malloc( info->bmiHeader.biSizeImage ))) goto failed;
        if (!NtGdiGetDIBitsInternal( hdc, mask, 0, bm.bmHeight, mask_bits,
           info, DIB_RGB_COLORS, 0, 0 )) goto failed;



        for (i = 0; i < bm.bmHeight; i++) {
            for (j = 0; j < bm.bmWidth; j++, ptr++) {
                  if (!((mask_bits[i * width_bytes + j / 8] << (j % 8)) & 0x80)) *ptr |= 0xff000000;

            }
        }

        free( mask_bits );
    }



    ptr = bits;
    for (i = 0; i < bm.bmWidth * bm.bmHeight; i++, ptr++) {
      red   = (*ptr >> 16) & 0xff;
      green = (*ptr >> 8) & 0xff;
      blue  = (*ptr >> 0) & 0xff;
      alpha   = (*ptr >> 24);


      if(!alpha || alpha < 26) {
        *ptr = 0x00000000;
      } else if ( (red + green + blue < 0x40) && alpha < 26 ) {
        *ptr = 0x00000000;
      } else if(*ptr) {
        //*ptr |= 0x50000000;
        //*ptr = blue | green << 8 | red << 16 | alpha << 24;
      }


    }




    return bits;

failed:
    free( bits );
    free( mask_bits );
    *width = *height = 0;
    return NULL;
}

static void set_custom_cursor( HCURSOR handle ) {

    unsigned int width = 0, height = 0;
    unsigned int xhotspot = 0, yhotspot = 0;
    ICONINFO info;
    struct wl_buffer *buffer;
    uint32_t *dest_pixels  = NULL;
    uint32_t *src_pixels  = NULL;
    uint32_t *bits = NULL;
    HDC hdc = NULL;
    int stride = 0;
    int size = 0;
    int y, x;

    char sprint_buffer[200];
    struct cursor_cache *cached_cursor;

    if ((cached_cursor = global_cursor_cache[cursor_idx(handle)])) {

      bits = global_cursor_cache[cursor_idx( handle )]->cached_pixels;
      width = global_cursor_cache[cursor_idx( handle )]->width;
      height = global_cursor_cache[cursor_idx( handle )]->height;
      xhotspot = global_cursor_cache[cursor_idx( handle )]->xhotspot;
      yhotspot = global_cursor_cache[cursor_idx( handle )]->yhotspot;

      //TRACE("Cursor cache hit w h %d %d %d \n", width, height, cursor_idx(handle));

    } else {

      //TRACE("Cursor cache miss w h %p %d \n", handle, cursor_idx(handle));

      if (!NtUserGetIconInfo( handle, &info, NULL, NULL, NULL, 0 ))
        return;


      hdc = NtGdiCreateCompatibleDC( 0 );

      bits = get_bitmap_argb( hdc, info.hbmColor, info.hbmMask, &width, &height);

      if(!bits) {
        return;
      }
      if(width < 1) {
        return;
      }



      /* make sure hotspot is valid */
      if (info.xHotspot >= width || info.yHotspot >= height)
      {
          info.xHotspot = width / 2;
          info.yHotspot = height / 2;
      }

      TRACE("Cursor cache set w h %p %d - %d %d \n", handle,
        cursor_idx(handle),
        info.xHotspot,
        info.yHotspot
      );
      alloc_cursor_cache(handle);


      global_cursor_cache[cursor_idx( handle )]->cached_pixels = bits;
      global_cursor_cache[cursor_idx( handle )]->handle = handle;
      global_cursor_cache[cursor_idx( handle )]->width = width;
      global_cursor_cache[cursor_idx( handle )]->height = height;
      global_cursor_cache[cursor_idx( handle )]->xhotspot = info.xHotspot;
      global_cursor_cache[cursor_idx( handle )]->yhotspot = info.yHotspot;
      xhotspot = info.xHotspot;
      yhotspot = info.yHotspot;

      NtGdiDeleteObjectApp( hdc );
    }


  //TRACE("Cursor width is %d %d\n", width, height);



  stride = width * 4; // 4 bytes per pixel
  size = stride * height;

  if(width != global_cursor_width || height != global_cursor_height) {

    if(global_cursor_gdi_fd) {
      close(global_cursor_gdi_fd);
    }

    sprintf(sprint_buffer, "wine-shared-cursor-%d", width);

    TRACE( "creating gdi fd %s \n", sprint_buffer);

    global_cursor_gdi_fd = memfd_create(sprint_buffer, MFD_CLOEXEC | MFD_ALLOW_SEALING);
    if (global_cursor_gdi_fd >= 0) {
      fcntl(global_cursor_gdi_fd, F_ADD_SEALS, F_SEAL_SHRINK);
    } else {
      exit(1);
    }
    posix_fallocate(global_cursor_gdi_fd, 0, size);

    if(global_cursor_shm_data) {
      munmap(global_cursor_shm_data, global_cursor_width * 4 * global_cursor_height);
    }

    global_cursor_width = width;
    global_cursor_height = height;

    //MAP_SHARED

    global_cursor_shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, global_cursor_gdi_fd, 0);

    if (global_cursor_shm_data == MAP_FAILED) {
      fprintf(stderr, "mmap failed: %m\n");
      close(global_cursor_gdi_fd);
      exit(1);
    }

    TRACE( "creating cursor wl_shm_data \n" );

    if(global_cursor_pool) {
      wl_shm_pool_destroy(global_cursor_pool);
    }
    global_cursor_pool = wl_shm_create_pool(global_shm, global_cursor_gdi_fd, size);


  }

  //compositor may not be ready
  if(!wayland_compositor || !wayland_cursor_surface)
    return;

  dest_pixels = (uint32_t *)global_cursor_shm_data;
  src_pixels = bits;



  buffer = wl_shm_pool_create_buffer(global_cursor_pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
  wl_buffer_add_listener(buffer, &buffer_listener, NULL);

  wl_surface_attach(wayland_cursor_surface, buffer, 0, 0);

  for (y = 0; y < height ; y++) {
    for (x = 0; x < width; x++) {
        dest_pixels[x] = src_pixels[x];
    }
    src_pixels += width;
    dest_pixels += width;
  }

  wl_surface_damage(wayland_cursor_surface, 0, 0, width, height);
  wl_surface_commit(wayland_cursor_surface);
  wl_pointer_set_cursor(wayland_pointer, wayland_serial_id,
    wayland_cursor_surface,
    xhotspot,
    yhotspot);


  //TRACE( "DONE set_custom_cursor \n" );

}


void WAYLANDDRV_SetCursor( HWND hwnd, HCURSOR handle )
{

    if(global_hide_cursor || !wayland_default_cursor) {
      return;
    }

    if(handle) {

      struct wl_cursor_image *image;
      struct wl_buffer *buffer;

      //show mouse
      if( !global_is_cursor_visible || !global_last_cursor_change ||
        ( global_custom_cursors && handle != global_last_cursor_handle)
      ) {

        //TRACE("Showing cursor \n");

        global_last_cursor_change = 1;
        global_is_cursor_visible = 1;

        if(!global_custom_cursors) {
          image = wayland_default_cursor->images[0];
          buffer = wl_cursor_image_get_buffer(image);
          wl_pointer_set_cursor(wayland_pointer, wayland_serial_id,
            wayland_cursor_surface,
            image->hotspot_x,
            image->hotspot_y);
            wl_surface_attach(wayland_cursor_surface, buffer, 0, 0);
            wl_surface_damage(wayland_cursor_surface, 0, 0,
            image->width, image->height);

          wl_surface_commit(wayland_cursor_surface);
        } else {
          global_last_cursor_handle = handle;
          set_custom_cursor( handle );
        }

        //ungrab screen if necessary
        ungrab_wayland_screen();
      }

    } else if(!handle && (global_is_cursor_visible || !global_last_cursor_change) ) { //Remove cursor
      TRACE("Removing cursor \n");
      global_is_cursor_visible = 0;
      global_last_cursor_change = 1;
      wl_pointer_set_cursor(wayland_pointer, wayland_serial_id,
        NULL, 0, 0);
      wl_surface_commit(wayland_cursor_surface);
      if( global_fullscreen_grab_cursor ){
        grab_wayland_screen();
      }

    }

}



// End wayland

//GDI surface

/* only for use on sanitized BITMAPINFO structures */
static inline int get_dib_info_size( const BITMAPINFO *info, UINT coloruse )
{
    if (info->bmiHeader.biCompression == BI_BITFIELDS)
        return sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD);
    if (coloruse == DIB_PAL_COLORS)
        return sizeof(BITMAPINFOHEADER) + info->bmiHeader.biClrUsed * sizeof(WORD);
    return FIELD_OFFSET( BITMAPINFO, bmiColors[info->bmiHeader.biClrUsed] );
}

static inline int get_dib_stride( int width, int bpp )
{
    return ((width * bpp + 31) >> 3) & ~3;
}

static inline int get_dib_image_size( const BITMAPINFO *info )
{
    return get_dib_stride( info->bmiHeader.biWidth, info->bmiHeader.biBitCount )
        * abs( info->bmiHeader.biHeight );
}


/* Window surface support */

static inline int context_idx( HWND hwnd  )
{
    return LOWORD( hwnd ) >> 1;
}

static struct gdi_window_surface *get_gdi_surface( struct window_surface *surface )
{
    return (struct gdi_window_surface *)surface;
}



/* store the palette or color mask data in the bitmap info structure */
static void set_color_info( BITMAPINFO *info, BOOL has_alpha )
{
    //DWORD *colors = (DWORD *)info->bmiColors;

    info->bmiHeader.biSize = sizeof(info->bmiHeader);
    info->bmiHeader.biClrUsed = 0;
    info->bmiHeader.biBitCount = 32;
    if (has_alpha)
    {
        info->bmiHeader.biCompression = BI_RGB;
        return;
    }
    /*

    info->bmiHeader.biCompression = BI_BITFIELDS;
    colors[0] = 0xff0000;
    colors[1] = 0x00ff00;
    colors[2] = 0x0000ff;
    */
}


/***********************************************************************
 *           alloc_gdi_win_data
 */
static struct gdi_win_data *alloc_gdi_win_data( HWND hwnd )
{
    struct gdi_win_data *data;

    if ((data = calloc(1, sizeof(*data))))
    {
        data->hwnd = hwnd;
        data->window = gdi_window;
        win_data_context[context_idx(hwnd)] = data;
    }
    return data;
}


/***********************************************************************
 *           free_win_data
 */
static void free_win_data( struct gdi_win_data *data )
{
    win_data_context[context_idx( data->hwnd )] = NULL;
    free( data );
}

/***********************************************************************
 *           get_win_data
 *
 * Lock and return the data structure associated with a window.
 */
static struct gdi_win_data *get_win_data( HWND hwnd )
{
    struct gdi_win_data *data;

    if (!hwnd) return NULL;
    if ((data = win_data_context[context_idx(hwnd)]) && data->hwnd == hwnd) {
      return data;
    }
    return NULL;
}

/***********************************************************************
 *           gdi_surface_lock
 */
static void gdi_surface_lock( struct window_surface *window_surface )
{

}

/***********************************************************************
 *           gdi_surface_unlock
 */
static void gdi_surface_unlock( struct window_surface *window_surface )
{

}

/***********************************************************************
 *           gdi_surface_get_bitmap_info
 */
static void *gdi_surface_get_bitmap_info( struct window_surface *window_surface, BITMAPINFO *info )
{
    struct gdi_window_surface *surface = get_gdi_surface( window_surface );

    memcpy( info, &surface->info, get_dib_info_size( &surface->info, DIB_RGB_COLORS ));
    return surface->bits;
}

/***********************************************************************
 *           gdi_surface_get_bounds
 */
static RECT *gdi_surface_get_bounds( struct window_surface *window_surface )
{
    struct gdi_window_surface *surface = get_gdi_surface( window_surface );

    return &surface->bounds;
}

/***********************************************************************
 *           gdi_surface_set_region
 */
static void gdi_surface_set_region( struct window_surface *window_surface, HRGN region )
{
    struct gdi_window_surface *surface = get_gdi_surface( window_surface );


    if (!region)
    {
        if (surface->region) NtGdiDeleteObjectApp( surface->region );
        surface->region = 0;
    }
    else
    {
        if (!surface->region) surface->region = NtGdiCreateRectRgn( 0, 0, 0, 0 );
        NtGdiCombineRgn( surface->region, region, 0, RGN_COPY );
    }

    set_surface_region( &surface->header, (HRGN)1 );
}



/***********************************************************************
 *           gdi_surface_flush
 */
//Basic GDI windows support - mostly not working
//https://github.com/wayland-project/weston/blob/3957863667c15bc5f1984ddc6c5967a323f41e7a/clients/simple-shm.c
//https://github.com/ricardomv/cairo-wayland/blob/master/src/shm.c
static void gdi_surface_flush( struct window_surface *window_surface )
{

    //TRACE( "GDI flush %p \n", window_surface);

    int x, y, width;
    uint32_t *src_pixels;
    uint32_t *dest_pixels;

    HWND owner;
    RECT client_rect;

    int HEIGHT = 0;
    int WIDTH = 0;

    RECT rect;
    BOOL needs_flush;

    LONG l,t;

    struct gdi_window_surface *surface = get_gdi_surface( window_surface );

    int stride = 0;
    int size = 0;

    struct gdi_win_data *hwnd_data;

    char sprint_buffer[200];

    if(global_is_vulkan) {
      return;
    }

    if(!wayland_display) {
      return;
    }


    surface = get_gdi_surface( window_surface );


    if(!surface) {
      TRACE("No surface found \n" );
      return;
    }


    if(surface->hwnd == global_vulkan_hwnd) {
      TRACE("Global Vulkan hwnd is %p \n", surface->hwnd);
      window_surface_release( &*window_surface );
      return;
    }


    if(!surface->hwnd) {
      return;
    }

    if (!(hwnd_data = get_win_data( surface->hwnd )))
      return;

    //without global_update_hwnd gray screen on startup
    if(hwnd_data->buffer_busy && global_update_hwnd) {
      return;
    }

    owner = NtUserGetWindowRelative( surface->hwnd, GW_OWNER );

    NtUserGetWindowRect(surface->hwnd, &client_rect);

    WIDTH = client_rect.right - client_rect.left;
    HEIGHT = client_rect.bottom - client_rect.top;
    stride = WIDTH * 4; // 4 bytes per pixel
    size = stride * HEIGHT;

    SetRect( &rect, 0, 0, surface->header.rect.right - surface->header.rect.left,
             surface->header.rect.bottom - surface->header.rect.top );



    //Checks and reduces rect to changed areas
    needs_flush = intersect_rect( &rect, &rect, &surface->bounds );
    reset_bounds( &surface->bounds );

    //(hwnd_data->window_width == WIDTH && hwnd_data->window_height == HEIGHT)
    if (!needs_flush && hwnd_data->surface_changed < 1) {
      return;
    }

    intersect_rect( &rect, &rect, &surface->header.rect );

    if(hwnd_data->window_width && (hwnd_data->window_width != WIDTH || hwnd_data->window_height != HEIGHT)) {
      hwnd_data->size_changed = 1;
      hwnd_data->window_width = WIDTH;
      hwnd_data->window_height = HEIGHT;
      TRACE( "Size changed %p \n", surface->hwnd);
    }



    //TODO proper cleanup
    if(hwnd_data->size_changed > 0) {
      TRACE("wl surface changed \n");
      if(hwnd_data->gdi_fd)
         close(hwnd_data->gdi_fd);
      if(hwnd_data->wl_pool)
         wl_shm_pool_destroy(hwnd_data->wl_pool);

      if(hwnd_data->shm_data && hwnd_data->size)
         munmap(hwnd_data->shm_data, hwnd_data->size);

      if(hwnd_data->buffer)
         wl_buffer_destroy(hwnd_data->buffer);

      hwnd_data->gdi_fd = 0;
      hwnd_data->shm_data = NULL;
      hwnd_data->wl_pool = NULL;
      hwnd_data->buffer = NULL;
    }
    hwnd_data->size = size;


    if(!hwnd_data->gdi_fd) {
      sprintf(sprint_buffer, "wine-shared-%p", surface->hwnd);

      TRACE( "creating gdi fd %s \n", sprint_buffer);

      hwnd_data->gdi_fd = memfd_create(sprint_buffer, MFD_CLOEXEC | MFD_ALLOW_SEALING);
      if (hwnd_data->gdi_fd >= 0) {
        fcntl(hwnd_data->gdi_fd, F_ADD_SEALS, F_SEAL_SHRINK);
      } else {
        exit(1);
      }
      posix_fallocate(hwnd_data->gdi_fd, 0, size);
    }


    //MAP_SHARED

    if(!hwnd_data->shm_data) {

      hwnd_data->shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, hwnd_data->gdi_fd, 0);



      if (hwnd_data->shm_data == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %m\n");
        close(hwnd_data->gdi_fd);
        exit(1);
      }

      TRACE( "creating wl_shm_data \n" );
    }


    //TRACE("Client Rect rect %s %d %d %p \n", wine_dbgstr_rect( &client_rect ), WIDTH, HEIGHT, surface->hwnd);


    if(!hwnd_data->wl_pool) {
      TRACE( "creating wl_pool \n" );
      hwnd_data->wl_pool = wl_shm_create_pool(global_shm, hwnd_data->gdi_fd, size);
    }
    if(!hwnd_data->buffer) {
      TRACE( "creating buffer \n" );
      hwnd_data->buffer = wl_shm_pool_create_buffer(hwnd_data->wl_pool, 0, WIDTH, HEIGHT, stride, WL_SHM_FORMAT_ARGB8888);

      wl_buffer_add_listener(hwnd_data->buffer, &buffer_listener, surface->hwnd);
    }

    hwnd_data->buffer_busy= 1;


    if(!hwnd_data->wayland_subsurface) {

      hwnd_data->window_width = WIDTH;
      hwnd_data->window_height = HEIGHT;

      hwnd_data->wayland_surface = wl_compositor_create_surface(wayland_compositor);
      hwnd_data->wayland_subsurface = wl_subcompositor_get_subsurface(wayland_subcompositor, hwnd_data->wayland_surface, gdi_window->surface);

      TRACE( "creating wl_subsurface %p %p for hwnd %p \n", hwnd_data->wayland_subsurface, hwnd_data->wayland_surface, surface->hwnd );

      wl_subsurface_set_position(hwnd_data->wayland_subsurface, client_rect.left, client_rect.top);
      wl_subsurface_set_desync(hwnd_data->wayland_subsurface);


      //if window is owned
      if(owner) {
        struct gdi_win_data *owner_hwnd_data;
        owner_hwnd_data = get_win_data( owner );

        if (owner_hwnd_data && owner_hwnd_data->wayland_surface)
          wl_subsurface_place_above(hwnd_data->wayland_subsurface, owner_hwnd_data->wayland_surface);
        else
          wl_subsurface_place_above(hwnd_data->wayland_subsurface, gdi_window->surface);
      }

      //alloc_wl_win_data(hwnd_data->wayland_subsurface, surface->hwnd);

      wl_surface_set_user_data(hwnd_data->wayland_surface, surface->hwnd);

      wl_surface_attach(hwnd_data->wayland_surface, hwnd_data->buffer, 0, 0);
    } else {


      wl_surface_attach(hwnd_data->wayland_surface, hwnd_data->buffer, 0, 0);


      if(hwnd_data->surface_changed) {

        wl_subsurface_set_position(hwnd_data->wayland_subsurface, client_rect.left, client_rect.top);

        //Dynamic move is currently broken
        if(!global_gdi_lb_hold) {
          TRACE("wl surface changed %d %d \n", client_rect.left, client_rect.top );
          global_gdi_position_changing = 1;
          TRACE("relative mouse move on \n");
        }
        wl_surface_commit(gdi_window->surface);

        //global_gdi_position_changed = 1;
        //wl_surface_damage(hwnd_data->wayland_surface, 0, 0, WIDTH, HEIGHT);
        //wl_surface_commit(hwnd_data->wayland_surface);
      }

    }

    src_pixels = (unsigned int *)surface->bits + (rect.top - surface->header.rect.top) * surface->info.bmiHeader.biWidth + (rect.left - surface->header.rect.left);

    dest_pixels = (unsigned int *)hwnd_data->shm_data;



    l = rect.left;
    t = rect.top;


    if(l != 0 || t != 0 ) {

      dest_pixels = (unsigned int *)hwnd_data->shm_data + (rect.top) * WIDTH + rect.left;

    }


    width = min( rect.right - rect.left, stride );


    //for (y = rect.top; y < min( HEIGHT, rect.bottom); y++)
    for (y = rect.top; y < min( HEIGHT, rect.bottom); y++)
    {
        for (x = 0; x < width; x++) {
          dest_pixels[x] = src_pixels[x] | 0xFF000000;
        }

        src_pixels += surface->info.bmiHeader.biWidth;
        dest_pixels += WIDTH;
    }
    hwnd_data->surface_changed = 0;
    hwnd_data->size_changed = 0;

    wl_surface_damage(hwnd_data->wayland_surface, 0, 0, WIDTH, HEIGHT);
    wl_surface_commit(hwnd_data->wayland_surface);


}


/***********************************************************************
 *           gdi_surface_destroy
 */
static void gdi_surface_destroy( struct window_surface *window_surface )
{
    struct gdi_window_surface *surface = get_gdi_surface( window_surface );
    struct gdi_win_data *hwnd_data;

    hwnd_data = get_win_data( surface->hwnd );

    if (hwnd_data) {
      //hwnd_data->surface_changed = 1;
    }

    TRACE( "Freeing wine surface - %p bits %p %p \n", surface, surface->bits, surface->hwnd );

    free( surface->region_data );
    if (surface->region) NtGdiDeleteObjectApp( surface->region );

    free( surface->bits );
    free( surface );
}

static const struct window_surface_funcs gdi_surface_funcs =
{
    gdi_surface_lock,
    gdi_surface_unlock,
    gdi_surface_get_bitmap_info,
    gdi_surface_get_bounds,
    gdi_surface_set_region,
    gdi_surface_flush,
    gdi_surface_destroy
};


/***********************************************************************
 *           set_surface_region
 */
static void set_surface_region( struct window_surface *window_surface, HRGN win_region )
{
    struct gdi_window_surface *surface = get_gdi_surface( window_surface );
    struct gdi_win_data *win_data;
    HRGN region = win_region;
    RGNDATA *data = NULL;
    UINT size;
    int offset_x, offset_y;

    if (window_surface->funcs != &gdi_surface_funcs) return;  /* we may get the null surface */

    if (!(win_data = get_win_data( surface->hwnd ))) return;
    offset_x = win_data->window_rect.left - win_data->whole_rect.left;
    offset_y = win_data->window_rect.top - win_data->whole_rect.top;


    if (win_region == (HRGN)1)  /* hack: win_region == 1 means retrieve region from server */
    {
        region = NtGdiCreateRectRgn( 0, 0, win_data->window_rect.right - win_data->window_rect.left,
                                win_data->window_rect.bottom - win_data->window_rect.top );
        if (NtUserGetWindowRgnEx( surface->hwnd, region, 0 ) == ERROR && !surface->region) goto done;
    }

    NtGdiOffsetRgn( region, offset_x, offset_y );
    if (surface->region) NtGdiCombineRgn( region, region, surface->region, RGN_AND );

    if (!(size = NtGdiGetRegionData( region, 0, NULL ))) goto done;
    if (!(data = calloc( 1, size )) ) goto done;

    if (!NtGdiGetRegionData( region, size, data ))
    {
        free( data );
        data = NULL;
    }

done:
    window_surface->funcs->lock( window_surface );
    free( surface->region_data );
    surface->region_data = data;
    *window_surface->funcs->get_bounds( window_surface ) = surface->header.rect;
    window_surface->funcs->unlock( window_surface );
    if (region != win_region) NtGdiDeleteObjectApp( region );
}

/***********************************************************************
 *           create_surface
 */
static struct window_surface *create_surface( HWND hwnd, const RECT *rect,
                                              BYTE alpha, COLORREF color_key, BOOL src_alpha )
{
    struct gdi_window_surface *surface;
    int width = rect->right - rect->left, height = rect->bottom - rect->top;

    surface = calloc( 1, FIELD_OFFSET( struct gdi_window_surface, info.bmiColors[3] ));
    if (!surface) return NULL;
    set_color_info( &surface->info, 1 );
//    set_color_info( &surface->info, src_alpha );
    surface->info.bmiHeader.biWidth       = width;
    surface->info.bmiHeader.biHeight      = -height; /* top-down */
    surface->info.bmiHeader.biPlanes      = 1;
    surface->info.bmiHeader.biSizeImage   = get_dib_image_size( &surface->info );


    surface->header.funcs = &gdi_surface_funcs;
    surface->header.rect  = *rect;
    surface->header.ref   = 1;
    surface->hwnd         = hwnd;
    surface->window       = gdi_window;
    surface->wayland_subsurface       = NULL;
    surface->wayland_surface       = NULL;
    surface->shm_data       = NULL;
    surface->wl_pool       = NULL;
    surface->gdi_fd       = 0;
    surface->alpha        = alpha;

    set_surface_region( &surface->header, (HRGN)1 );
    reset_bounds( &surface->bounds );

    if (!(surface->bits = malloc( surface->info.bmiHeader.biSizeImage )))
        goto failed;

    TRACE( "created %p hwnd %p %s bits %p-%p\n", surface, hwnd, wine_dbgstr_rect(rect),
           surface->bits, (char *)surface->bits + surface->info.bmiHeader.biSizeImage );

    return &surface->header;

failed:
    gdi_surface_destroy( &surface->header );
    return NULL;
}

//Windows functions

/***********************************************************************
 *
 *
 * Create an gdi data structure for an existing window.
 */
static int do_create_gdi_data( HWND hwnd, const RECT *window_rect, const RECT *client_rect )
{

    HWND parent;
    if (!(parent = NtUserGetAncestor( hwnd, GA_PARENT )))
      return 0;  /* desktop */

    // don't create win data for HWND_MESSAGE windows */
    if (parent != NtUserGetDesktopWindow() && !NtUserGetAncestor( parent, GA_PARENT ))
      return 0;

    if (NtUserGetWindowThread( hwnd, NULL ) != GetCurrentThreadId())
      return 0;

    return 1;

}

static struct gdi_win_data *create_gdi_data( HWND hwnd, const RECT *window_rect,
                                                 const RECT *client_rect )
{
    struct gdi_win_data *data;
    HWND parent;

    if (!(parent = NtUserGetAncestor( hwnd, GA_PARENT ))) return NULL;  /* desktop or HWND_MESSAGE */

    if (!(data = alloc_gdi_win_data( hwnd ))) return NULL;

    data->parent = (parent == NtUserGetDesktopWindow()) ? 0 : parent;
    data->whole_rect = data->window_rect = *window_rect;
    data->client_rect = *client_rect;
    data->wayland_subsurface = NULL;
    data->wayland_surface = NULL;
    data->window_width = 0;
    data->window_height = 0;
    data->size = 0;
    data->shm_data = NULL;
    data->wl_pool = NULL;
    data->buffer = NULL;
    data->gdi_fd = 0;
    data->buffer_busy = 0;
    data->surface_changed = 0;
    data->size_changed = 0;

    TRACE( "created gdi_win_data for %p hwnd /n", hwnd);

    return data;
}


static inline BOOL get_surface_rect( const RECT *visible_rect, RECT *surface_rect )
{
    RECT virtual_screen_rect = get_virtual_screen_rect();

    if (!intersect_rect( surface_rect, visible_rect, &virtual_screen_rect )) return FALSE;
    OffsetRect( surface_rect, -visible_rect->left, -visible_rect->top );
    surface_rect->left &= ~31;
    surface_rect->top  &= ~31;
    surface_rect->right  = max( surface_rect->left + 32, (surface_rect->right + 31) & ~31 );
    surface_rect->bottom = max( surface_rect->top + 32, (surface_rect->bottom + 31) & ~31 );
    return TRUE;
}


/***********************************************************************
 *		WindowPosChanging   (WAYLANDDRV.@)
 */
BOOL WAYLANDDRV_WindowPosChanging( HWND hwnd, HWND insert_after, UINT swp_flags,
                                     const RECT *window_rect, const RECT *client_rect, RECT *visible_rect,
                                     struct window_surface **surface )
{

  int count = 0;
  struct gdi_win_data *data;
  COLORREF key;
  HWND parent;
  int do_create_surface = 0;
  int HEIGHT = 0;
  int WIDTH = 0;
  RECT window_client_rect, rect;
  WCHAR title_name[1024] = { L'\0' };
  WCHAR class_buff[64];

  UNICODE_STRING class_name = { .Buffer = class_buff, .MaximumLength = sizeof(class_buff) };

  static const WCHAR desktop_class[] = {'#', '3', '2', '7', '6', '9', 0};
  static const WCHAR tray_class[] = {'S','h','e','l','l','_','T','r','a','y','W','n','d', 0};

  static const WCHAR update_class[] = {'r','u','n','d','l','l','3','2', 0};
  static const WCHAR ole_class[] = {'O','l','e','M','a','i','n','T','h','r','e','a','d','W','n','d','C','l','a','s','s', 0};
  static const WCHAR msg_class[] = {'M','e','s','s','a','g','e', 0};
  static const WCHAR ime_class[] = {'I','M','E', 0};
  static const WCHAR ime_class2[] = {'W','i','n','e', ' ', 'I','M','E', 0};
  static const WCHAR dde_class1[] = {'W','i','n','e','D','d','e','S','e','r','v','e','r','N','a','m','e', 0};
  static const WCHAR dde_class2[] = {'W','i','n','e','D','d','e','E','v','e','n','t','C','l','a','s','s', 0};
  static const WCHAR tooltip_class[] = {'t','o','o','l','t','i','p','s','_',
  'c','l','a','s','s','3','2', 0};


  static const WCHAR sdl_class[] = {'S','D','L','_','a','p','p', 0};
  static const WCHAR unreal_class[] = {'U','n','r','e','a','l','W','i','n','d','o','w', 0};
  //POEWindowClass
  static const WCHAR poe_class[] = {'P','O','E','W','i','n','d','o','w','C','l','a','s','s', 0};
  //OgreD3D11Wnd
  static const WCHAR ogre_class[] = {'O','g','r','e','D','3','D','1','1','W','n','d', 0};
  static const WCHAR unity_class[] = {'U','n','i','t','y','W','n','d','C','l','a','s','s', 0};
  //Unreal splash screens are not destroyed
  static const WCHAR unreal_splash_class[] = {'S','p','l','a','s','h','S','c','r','e','e','n','C','l','a','s','s', 0};
  //Shogun2 crash fix
  static const WCHAR shogun2_frame_class[] = {'S','h','o','g','u','n','2', 0};


  //Flstudio buggy splashscreen
  static const WCHAR flstudio_hwnd_class[] = {
     'T','L','i','g','h','t','w','e',
    'i','g','h','t','L','a','y','e','r','e','d','C','o','n','t','r','o','l', 0};

  const char *is_vulkan_only = getenv( "WINE_VK_VULKAN_ONLY" );

  if(hwnd == global_vulkan_hwnd) {
    TRACE("Removing window decorations for FSR \n");
    //For FSR
    NtUserSetWindowLongPtr(global_vulkan_hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST, 0);
    NtUserSetWindowLongPtr(global_vulkan_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE, 0);
    //For caching global_vulkan_hwnd rect
    global_vulkan_rect_flag = 0;
    NtUserGetWindowRect(global_vulkan_hwnd, &global_vulkan_rect);
    return TRUE;
  }

  if(is_vulkan_only) {
    return TRUE;
  }

  if(hwnd == NtUserGetDesktopWindow()) {
    return TRUE;
  }



  parent = NtUserGetAncestor(hwnd, GA_PARENT);

  if( !parent || parent != NtUserGetDesktopWindow()) {
    return TRUE;
  }

  if( NtUserGetClassName(hwnd, FALSE, &class_name )) {

    if(!wcsicmp(class_name.Buffer, msg_class)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, update_class)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, ole_class)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, ime_class)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, ime_class2)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, desktop_class)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, tray_class)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, tooltip_class)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, dde_class1)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, dde_class2)) {
      return TRUE;
    }

    if(!wcsicmp(class_name.Buffer, unreal_class)) {
      return TRUE;
    }

    if(!wcsicmp(class_name.Buffer, unreal_splash_class)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, shogun2_frame_class)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, flstudio_hwnd_class)) {
      return TRUE;
    }

    TRACE( "Changing %p %s \n",
      hwnd, debugstr_w(class_name.Buffer)
    );

    //Upscale

    //Wayland display may not be ready yet, test for FSR variable here as well
    if(getenv( "WINE_VK_USE_FSR" )) {
      global_fsr = 1;
      global_fsr_set = 1;
      global_is_always_fullscreen = 1; //enable fullscreen for FSR
    }

    //Remove borders for some games that refuse to do elsewhere
    if(global_fsr) {
      TRACE( "Removing window decorations %p %s \n",
        hwnd, debugstr_w(class_name.Buffer)
      );
      NtUserSetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST, 0);
      NtUserSetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE, 0);
    }

    if(!wcsicmp(class_name.Buffer, sdl_class)) {
      return TRUE;
    }

    if(!wcsicmp(class_name.Buffer, poe_class)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, ogre_class)) {
      return TRUE;
    }
    if(!wcsicmp(class_name.Buffer, unity_class)) {
      return TRUE;
    }



  }

  //Get window width/height
  NtUserGetWindowRect(hwnd, &window_client_rect);

  NtUserInternalGetWindowText(hwnd, title_name, 1024);
  TRACE( "Changing %p %s Window title %d / %s %s rect \n",
    hwnd, debugstr_w(class_name.Buffer), lstrlenW( title_name ),
    debugstr_wn(title_name, lstrlenW( title_name )),
    wine_dbgstr_rect( &window_client_rect )
  );


  do_create_surface = do_create_gdi_data( hwnd, window_rect, client_rect );

  if (!do_create_surface) {
    return TRUE;
  }

  if (!(swp_flags & SWP_SHOWWINDOW) && !(NtUserGetWindowLongW( hwnd, GWL_STYLE ) & WS_VISIBLE)) {
    return TRUE;
  }


  if (swp_flags & SWP_HIDEWINDOW) {
    TRACE("Window Should be hidden %s \n", debugstr_wn(title_name, lstrlenW( title_name )));
    return TRUE;
  }

  if(!wayland_display) {
    create_wayland_display();
  }

  WIDTH = window_client_rect.right - window_client_rect.left;
  HEIGHT = window_client_rect.bottom - window_client_rect.top;
  if(WIDTH < 1)
    WIDTH = 1440;
  if(HEIGHT < 1)
    HEIGHT = 900;

   //TRACE("WXH is %d %d for %p \n", WIDTH, HEIGHT, hwnd);

  if(wayland_display && !gdi_window) {
    TRACE("Creating wayland window %s %p \n", debugstr_w(class_name.Buffer), hwnd);
    gdi_window = create_wayland_window (hwnd, WIDTH, HEIGHT);


    while (!count) {
      sleep(0.1);
      wl_display_dispatch_pending (wayland_display);
      draw_gdi_wayland_window (gdi_window);
      sleep(0.1);
      count = 1;
    }
  } else if(wayland_display && gdi_window && global_vulkan_hwnd != NULL) {
    draw_gdi_wayland_window (gdi_window);
  }

  data = get_win_data( hwnd );
  if(!data) {
    data = create_gdi_data( hwnd, window_rect, client_rect );
  } else {

    if (*surface) {
//      TRACE("Surface exists \n", WIDTH, HEIGHT);
//      gdi_surface_destroy( *surface );
      window_surface_release( *surface );
      *surface = NULL;
    }

  }

    if(!data) {
      TRACE("NO DATA \n");
      return TRUE;
    } else {

      if (data->surface)
      {

        /* existing surface is good enough */
        window_surface_add_ref( data->surface );
        if (*surface) window_surface_release( *surface );
        *surface = data->surface;
        return TRUE;

      }

      rect = get_virtual_screen_rect();


      if ( (!parent || parent == NtUserGetDesktopWindow()) ) {

        if (*surface) {
          window_surface_release( *surface );
        }
        *surface = NULL;
        *surface = create_surface( data->hwnd, &rect, 255, key, FALSE );

      }



    }

  return TRUE;
}

/***********************************************************************
 *           ShowWindow   (WAYLANDDRV.@)
 */
UINT WAYLANDDRV_ShowWindow( HWND hwnd, INT cmd, RECT *rect, UINT swp )
{
//  WCHAR title_name[1024] = { L'\0' };
  struct gdi_win_data *hwnd_data;
//  WCHAR class_name[64];
  struct gdi_win_data *data;

  if(global_is_vulkan) {
    return swp;
  }

  TRACE( "Show Window \n");

  data = get_win_data( hwnd );
  if(!data) {
    return swp;
  }

//  if(RealGetWindowClassW(hwnd, class_name, ARRAY_SIZE(class_name))) {
//    NtUserInternalGetWindowText(hwnd, title_name, 1024);
//    TRACE( "Show/hide window %p %s title %s \n", hwnd, debugstr_w(class_name), debugstr_wn(title_name, strlenW( title_name )));
//  }


  if(!cmd || cmd & SW_HIDE) {
    TRACE("Hiding window %d %p %p \n", cmd, hwnd, global_update_hwnd);

    hwnd_data = get_win_data( hwnd );
    if (hwnd_data && hwnd_data->wayland_surface ) {

      TRACE("Hiding window %d %p clearing wayland surfaces \n", cmd, hwnd);

      wl_subsurface_destroy(hwnd_data->wayland_subsurface);
      wl_surface_destroy(hwnd_data->wayland_surface);
      hwnd_data->wayland_subsurface = NULL;
      hwnd_data->wayland_surface = NULL;

      wl_shm_pool_destroy(hwnd_data->wl_pool);

      if(hwnd_data->gdi_fd)
         close(hwnd_data->gdi_fd);

      if(hwnd_data->shm_data && hwnd_data->size) {
        TRACE("Clearing shm_data for %p \n", hwnd);
        munmap(hwnd_data->shm_data, hwnd_data->size);
      }
      if(hwnd_data->buffer) {
        wl_buffer_destroy(hwnd_data->buffer);
      }

      if(hwnd_data->surface) {
        //gdi_surface_destroy( hwnd_data->surface );
        window_surface_release( hwnd_data->surface );
        hwnd_data->surface = NULL;
      }

      hwnd_data->wl_pool = NULL;
      hwnd_data->buffer = NULL;
      hwnd_data->size = 0;
      hwnd_data->gdi_fd = 0;
      free_win_data(hwnd_data);
    }


  }

  return swp;

}

/***********************************************************************
 *           WindowPosChanged
 */
void WAYLANDDRV_WindowPosChanged( HWND hwnd, HWND insert_after, UINT swp_flags,
                                    const RECT *window_rect, const RECT *client_rect,
                                    const RECT *visible_rect, const RECT *valid_rects,
                                    struct window_surface *surface )
{
  struct gdi_win_data *hwnd_data;
  RECT rect;
  int height = 0;
  int width = 0;
  struct wl_region *region;

  hwnd_data = get_win_data( hwnd );
  if(!hwnd_data) {
    return;
  }

  NtUserGetWindowRect(hwnd, &rect);
  width = rect.right - rect.left;
  height = rect.bottom - rect.top;


  region = wl_compositor_create_region(wayland_compositor);
  wl_region_add(region, rect.left, rect.top, width, height);
//  wl_surface_set_input_region(data->wayland_surface, region);

  TRACE("Adding surface for hwnd %p %d %d / rect %s \n", hwnd, rect.left, rect.top,
        wine_dbgstr_rect( &rect )
  );

  if (surface)
    window_surface_add_ref( surface );

  if (hwnd_data->surface) {
      window_surface_release( hwnd_data->surface );
  }
  hwnd_data->surface = surface;

  if (swp_flags & SWP_HIDEWINDOW) {
    TRACE("Window Should be hidden %p \n", hwnd);
    WAYLANDDRV_ShowWindow( hwnd, 0, NULL, 0 );
    return;
  }
  if(hwnd_data->wayland_surface) {
    hwnd_data->surface_changed = 1;
  }

}

/***********************************************************************
 *           SysCommand
 */
LRESULT WAYLANDDRV_SysCommand(HWND hwnd, WPARAM wparam, LPARAM lparam)
{

  struct gdi_win_data *hwnd_data;
  WPARAM command = wparam & 0xfff0;

  hwnd_data = get_win_data( hwnd );

  if(!hwnd_data) {
    return -1;
  }

  if (command == SC_MOVE)
  {
    return 0;
  }
  return -1;
}




/**********************************************************************
 *		CreateWindow   (WAYLANDDRV.@)
 */
BOOL WAYLANDDRV_CreateWindow( HWND hwnd )
{
  WCHAR title_name[1024] = { L'\0' };
  WCHAR class_buff[64];
  UNICODE_STRING class_name = { .Buffer = class_buff,
    .MaximumLength = sizeof(class_buff)
  };

  if( NtUserGetClassName(hwnd, FALSE, &class_name )) {
    NtUserInternalGetWindowText(hwnd, title_name, 1024);
    TRACE( "Changing %p %s Window title %d / %s \n",
      hwnd, debugstr_w(class_name.Buffer), lstrlenW( title_name ),
      debugstr_wn(title_name, lstrlenW( title_name ))
    );
    TRACE("Creating window 1 \n");
  }
  return TRUE;
}


/***********************************************************************
 *		DestroyWindow   (WAYLANDDRV.@)
 */
void WAYLANDDRV_DestroyWindow( HWND hwnd )
{

  WCHAR title_name[1024] = { L'\0' };
  WCHAR class_buff[64];
  UNICODE_STRING class_name = { .Buffer = class_buff,
    .MaximumLength = sizeof(class_buff)
  };
  struct gdi_win_data *hwnd_data;

  if( NtUserGetClassName(hwnd, FALSE, &class_name )) {
    NtUserInternalGetWindowText(hwnd, title_name, 1024);
    TRACE( "Destroying %p %s Window title %d / %s \n",
      hwnd, debugstr_w(class_name.Buffer), lstrlenW( title_name ),
      debugstr_wn(title_name, lstrlenW( title_name ))
    );
    TRACE("Destroying window 1 \n");
  }



  if(global_is_vulkan) {
    if(hwnd == global_vulkan_hwnd) {
      global_vulkan_hwnd = NULL;
    }

    //destroy GDI windows games create
    if(vulkan_window && vulkan_window->pointer_to_hwnd == hwnd) {
      TRACE("Destroy wayland window for hwnd %p \n", hwnd);
      delete_wayland_window(vulkan_window);
      vulkan_window = NULL;
    } else if( vulkan_window && vulkan_window->pointer_to_hwnd != hwnd ){
      //try to find the window
      for(int ii = 0; ii < 32768; ii++ )
        if(wl_surface_data_context[ii]) {
          if(wl_surface_data_context[ii]->hwnd == hwnd) {
            delete_wayland_window(wl_surface_data_context[ii]->wayland_window);
          }
        }
    }

    return;
  }


    //Clean subsurface windows data

    hwnd_data = get_win_data( hwnd );


    if (hwnd_data && hwnd_data->wayland_surface ) {



      TRACE("destroying hwnd_data %p for %p \n", hwnd_data, hwnd);

      wl_subsurface_destroy(hwnd_data->wayland_subsurface);
      TRACE("hwnd_data %p for %p \n", hwnd_data, hwnd);
      wl_surface_destroy(hwnd_data->wayland_surface);
      wl_shm_pool_destroy(hwnd_data->wl_pool);

      if(hwnd_data->gdi_fd)
         close(hwnd_data->gdi_fd);

      if(hwnd_data->shm_data && hwnd_data->size) {
        TRACE("Clearing shm_data for %p \n", hwnd);
        munmap(hwnd_data->shm_data, hwnd_data->size);
      }
      if(hwnd_data->buffer) {
        wl_buffer_destroy(hwnd_data->buffer);
      }


      hwnd_data->wayland_subsurface = NULL;
      hwnd_data->wayland_surface = NULL;
      hwnd_data->wl_pool = NULL;
      hwnd_data->buffer = NULL;
      hwnd_data->size = 0;
      hwnd_data->gdi_fd = 0;

      if(hwnd_data->surface != NULL) {
        TRACE("Attempt clear surface %p for %p \n", hwnd_data->surface, hwnd);

        window_surface_release( hwnd_data->surface );
        hwnd_data->surface = NULL;
      }

      free_win_data(hwnd_data);
    }

    return;

}

//Win32 loop callback
BOOL WAYLANDDRV_ProcessEvents( DWORD mask )
{
//    unsigned int count = 0;
    if (wayland_display && desktop_tid && GetCurrentThreadId() == desktop_tid && !global_wait_for_configure)
    {

        while (wl_display_prepare_read(wayland_display) != 0) {
          wl_display_dispatch_pending(wayland_display);
//          count++;
        }

        wl_display_flush(wayland_display);
        wl_display_read_events(wayland_display);
        wl_display_dispatch_pending(wayland_display);

        return FALSE;
    }


    return FALSE;

}


//Windows functions


/* Helper function for converting between win32 and X11 compatible VkInstanceCreateInfo.
 * Caller is responsible for allocation and cleanup of 'dst'.
 */
static VkResult wine_vk_instance_convert_create_info(const VkInstanceCreateInfo *src,
        VkInstanceCreateInfo *dst)
{
    unsigned int i;
    const char **enabled_extensions = NULL;

    dst->sType = src->sType;
    dst->flags = src->flags;
    dst->pApplicationInfo = src->pApplicationInfo;
    dst->pNext = src->pNext;
    dst->enabledLayerCount = 0;
    dst->ppEnabledLayerNames = NULL;
    dst->enabledExtensionCount = 0;
    dst->ppEnabledExtensionNames = NULL;

    if (src->enabledExtensionCount > 0)
    {
        enabled_extensions = calloc(src->enabledExtensionCount, sizeof(*src->ppEnabledExtensionNames));
        if (!enabled_extensions)
        {
            ERR("Failed to allocate memory for enabled extensions\n");
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        for (i = 0; i < src->enabledExtensionCount; i++)
        {
            if (!strcmp(src->ppEnabledExtensionNames[i], "VK_KHR_win32_surface"))
            {

                enabled_extensions[i] = "VK_KHR_wayland_surface";

            }
            else
            {
                enabled_extensions[i] = src->ppEnabledExtensionNames[i];
            }
        }
        dst->ppEnabledExtensionNames = enabled_extensions;
        dst->enabledExtensionCount = src->enabledExtensionCount;
    }

    return VK_SUCCESS;
}

static VkSurfaceKHR WAYLANDDRV_wine_get_host_surface(VkSurfaceKHR surface)
{
   return surface;
}

static VkResult WAYLANDDRV_vkCreateInstance(const VkInstanceCreateInfo *create_info,
        const VkAllocationCallbacks *allocator, VkInstance *instance)
{
    VkInstanceCreateInfo create_info_host;
    VkResult res;

    /* Perform a second pass on converting VkInstanceCreateInfo. Winevulkan
     * performed a first pass in which it handles everything except for WSI
     * functionality such as VK_KHR_win32_surface. Handle this now.
     */
    res = wine_vk_instance_convert_create_info(create_info, &create_info_host);
    if (res != VK_SUCCESS)
    {
        ERR("Failed to convert instance create info, res=%d\n", res);
        return res;
    }

    res = pvkCreateInstance(&create_info_host, NULL /* allocator */, instance);

    if(res == VK_SUCCESS) {
      global_vk_instance = instance;
      TRACE("Create global_vk_instance create_info %p, allocator %p, instance %p\n", create_info, allocator, instance);
    }

    free((void *)create_info_host.ppEnabledExtensionNames);
    return res;
}

static VkResult WAYLANDDRV_vkCreateSwapchainKHR(VkDevice device,
        const VkSwapchainCreateInfoKHR *create_info,
        const VkAllocationCallbacks *allocator, VkSwapchainKHR *swapchain)
{
    RECT window_rect;

    global_vulkan_rect_flag = 0;

    //FSR
    //TRACE("%p %p %p %p\n", device, create_info, allocator, swapchain);

    TRACE("Vulkan swapchain rect %d %d \n",
        create_info->imageExtent.width,
        create_info->imageExtent.height
      );

    if(global_vulkan_hwnd && global_fsr && !fsr_matches_real_mode(
      create_info->imageExtent.width, create_info->imageExtent.height
      ) ) {
      NtUserGetClientRect(global_vulkan_hwnd, &window_rect);
      TRACE("FSR Vulkan hwnd rect %d %d \n",
        create_info->imageExtent.width,
        create_info->imageExtent.height
      );
      fsr_set_current_mode(create_info->imageExtent.width, create_info->imageExtent.height);
    }


    TRACE("Vulkan vkCreateSwapchainKHR done %d %d \n",
        create_info->imageExtent.width,
        create_info->imageExtent.height
      );


    if (allocator)
        FIXME("Support for allocation callbacks not implemented yet\n");

    return pvkCreateSwapchainKHR(device, create_info, NULL /* allocator */, swapchain);
}



/***************************************************************************
 *	get_basename
 *
 * Return the base name of a file name (i.e. remove the path components).
 */
//TODO
#if 0
static const WCHAR *get_basename( const WCHAR *name )
{
    const WCHAR *ptr;

    if (name[0] && name[1] == ':') name += 2;  /* strip drive specification */
    if ((ptr = strrchrW( name, '\\' ))) name = ptr + 1;
    if ((ptr = strrchrW( name, '/' ))) name = ptr + 1;
    return name;
}
#endif



static VkResult WAYLANDDRV_vkCreateWin32SurfaceKHR(VkInstance instance,
        const VkWin32SurfaceCreateInfoKHR *create_info,
        const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface)
{
    VkResult res;
    VkWaylandSurfaceCreateInfoKHR create_info_host;


    int no_flag = 1;
    int count = 0;
    int screen_width = 1920;
    int screen_height = 1080;
    RECT window_rect;
    char *env_width = NULL;
    char *env_height = NULL;
    WCHAR class_buff[64];
    UNICODE_STRING class_name = { .Buffer = class_buff, .MaximumLength = sizeof(class_buff) };
    //Hack
    //Do not create vulkan windows for Paradox detect
    static const WCHAR pdx_class[] = {'P','d','x','D','e','t','e','c','t','W','i','n','d','o','w', 0};

    TRACE("Vulkan hwnd %d  \n", no_flag);



    if( NtUserGetClassName(create_info->hwnd, FALSE, &class_name )) {

        if(!wcsicmp(class_name.Buffer, pdx_class)) {
          no_flag = 0;
        }
    }

    if(no_flag) {


      TRACE("Creating wayland display early \n");
      if(!wayland_display) {
        create_wayland_display();
      }


     TRACE("Vulkan hwnd 1 \n" );

      env_width = getenv( "WINE_VK_WAYLAND_WIDTH" );
      env_height = getenv( "WINE_VK_WAYLAND_HEIGHT" );

      if(global_output_width > 0 && global_output_height > 0) {
        screen_width = global_output_width;
        screen_height = global_output_height;
      }

      if(env_width) {
        screen_width = atoi(env_width);
      }
      if(env_height) {
        screen_height = atoi(env_height);
      }

      TRACE("hwnd hxw %d %d \n", screen_width, screen_height);

      global_vulkan_hwnd = create_info->hwnd;

      NtUserSetActiveWindow( global_vulkan_hwnd );
      NtUserSetForegroundWindow( global_vulkan_hwnd );

      NtUserSetFocus(global_vulkan_hwnd);

      if(global_fsr) {

        NtUserGetClientRect(global_vulkan_hwnd, &window_rect);
        NtUserSetWindowLongPtr(global_vulkan_hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST, 0);
        NtUserSetWindowLongPtr(global_vulkan_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE, 0);

        TRACE("Vulkan hwnd rect %s \n",  wine_dbgstr_rect( &window_rect ));
        TRACE("Vulkan hwnd set to borderless %s \n",  wine_dbgstr_rect( &window_rect ));

        fsr_set_current_mode(window_rect.right, window_rect.bottom);
      }

      SERVER_START_REQ( set_focus_window )
      {
        req->handle = wine_server_user_handle( global_vulkan_hwnd );
      }
      SERVER_END_REQ;

      TRACE("New global vulkan hwnd is %p \n", create_info->hwnd);

    } else {
      TRACE("Not visible for %p %p %p %p\n", instance, create_info, allocator, surface);
    }



    //TRACE("%p %p %p %p\n", instance, create_info->hwnd, allocator, surface);
    TRACE("Creating vulkan Window %p %s \n", create_info->hwnd, debugstr_w(class_name.Buffer));

    /* TODO: support child window rendering. */
    if (NtUserGetAncestor(create_info->hwnd, GA_PARENT) != NtUserGetDesktopWindow())
    {
        TRACE("Application requires child window rendering, which is not implemented yet!\n");
        //return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

  global_is_vulkan = 1;
	vulkan_window = create_wayland_window (create_info->hwnd, screen_width, screen_height);

  while (!count) {
    sleep(0.5);
		wl_display_dispatch_pending (wayland_display);
    sleep(0.5);
    count = 1;
	}


    NtUserSystemParametersInfo( SPI_SETMOUSESPEED , 0 ,
      (LPVOID)1, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE |
        SPIF_SENDWININICHANGE ) ;

    create_info_host.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    create_info_host.pNext = NULL;
    create_info_host.flags = 0;
    create_info_host.display = wayland_display;
    create_info_host.surface = vulkan_window->surface;
    res = pvkCreateWaylandSurfaceKHR(instance, &create_info_host, NULL /* allocator */, surface);

    if (res != VK_SUCCESS)
    {
        TRACE("Failed to create Vulkan surface, res=%d\n", res);
        exit(0);
        goto err;
    }

    TRACE("Created vulkan Window for %p  %s \n", create_info->hwnd, debugstr_w(class_name.Buffer));

    return VK_SUCCESS;

err:
    return res;
}

static void WAYLANDDRV_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *allocator)
{
    TRACE("%p %p\n", instance, allocator);

    //if(instance != VK_NULL_HANDLE)
    //  pvkDestroyInstance(instance, NULL /* allocator */);

    if(instance != VK_NULL_HANDLE && instance && global_vk_instance != NULL && &instance == global_vk_instance) {
      TRACE("=vkDestroyInstance 2 \n");
      TRACE("%p %p\n", instance, global_vk_instance);
      pvkDestroyInstance(instance, NULL /* allocator */);
    }

    TRACE("vkDestroyInstance 2 \n");

//    if(instance != VK_NULL_HANDLE)
//      pvkDestroyInstance(instance, NULL /* allocator */);

    TRACE("vkDestroyInstance 1 \n");
}

static void WAYLANDDRV_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
        const VkAllocationCallbacks *allocator)
{
    //TRACE("%p 0x%s %p\n", instance, wine_dbgstr_longlong(surface), allocator);

    if (allocator)
        FIXME("Support for allocation callbacks not implemented yet\n");

    /* vkDestroySurfaceKHR must handle VK_NULL_HANDLE (0) for surface. */
    if (surface)
    {
        pvkDestroySurfaceKHR(instance, surface, NULL /* allocator */);
    }
}

static void WAYLANDDRV_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
         const VkAllocationCallbacks *allocator)
{
    TRACE("%p, 0x%s %p\n", device, wine_dbgstr_longlong(swapchain), allocator);

    if(swapchain != VK_NULL_HANDLE)
      pvkDestroySwapchainKHR(device, swapchain, NULL /* allocator */);
}

static VkResult WAYLANDDRV_vkEnumerateInstanceExtensionProperties(const char *layer_name,
        uint32_t *count, VkExtensionProperties* properties)
{
    unsigned int i;
    VkResult res;

    /* This shouldn't get called with layer_name set, the ICD loader prevents it. */
    if (layer_name)
    {
        ERR("Layer enumeration not supported from ICD.\n");
        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    /* We will return the same number of instance extensions reported by the host back to
     * winevulkan. Along the way we may replace xlib extensions with their win32 equivalents.
     * Winevulkan will perform more detailed filtering as it knows whether it has thunks
     * for a particular extension.
     */
    res = pvkEnumerateInstanceExtensionProperties(layer_name, count, properties);
    if (!properties || res < 0)
        return res;

    for (i = 0; i < *count; i++)
    {
        /* For now the only x11 extension we need to fixup. Long-term we may need an array. */

        if (!strcmp(properties[i].extensionName, "VK_KHR_wayland_surface"))

        {
            //TRACE("Substituting VK_KHR_xlib_surface for VK_KHR_win32_surface\n");

            snprintf(properties[i].extensionName, sizeof(properties[i].extensionName),
                    VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
            properties[i].specVersion = VK_KHR_WIN32_SURFACE_SPEC_VERSION;
        }
    }

    TRACE("Returning %u extensions.\n", *count);
    return res;
}





static VkBool32 WAYLANDDRV_vkGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice phys_dev,
        uint32_t index)
{
    return pvkGetPhysicalDeviceWaylandPresentationSupportKHR(phys_dev, index, wayland_display);
}

static VkResult WAYLANDDRV_vkGetSwapchainImagesKHR(VkDevice device,
        VkSwapchainKHR swapchain, uint32_t *count, VkImage *images)
{
    //TRACE("%p, 0x%s %p %p\n", device, wine_dbgstr_longlong(swapchain), count, images);
    return pvkGetSwapchainImagesKHR(device, swapchain, count, images);
}

static VkResult WAYLANDDRV_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *present_info)
{
    //TRACE("%p, %p\n", queue, present_info);
    return pvkQueuePresentKHR(queue, present_info);
}

#ifdef HAS_FSR


static VkBool32 WAYLANDDRV_query_fsr(VkSurfaceKHR surface, VkExtent2D *real_sz,
  VkExtent2D *user_sz, VkRect2D *dst_blit,
  VkFilter *filter, BOOL *fsr, float *sharpness)
{

  RECT window_rect;
  char *env_width, *env_height;
  int screen_width = 0, screen_height = 0;

  TRACE("Test for FSR %d \n", global_fsr);

  global_vulkan_rect_flag = 0;

  env_width = getenv( "WINE_VK_WAYLAND_WIDTH" );
  env_height = getenv( "WINE_VK_WAYLAND_HEIGHT" );

  if(!global_fsr && !global_fsr_set)
    return VK_FALSE;

  if(!global_vulkan_hwnd)
    return VK_FALSE;

  //TODO move to function
  if(global_output_width > 0 && global_output_height > 0) {
    screen_width = global_output_width;
    screen_height = global_output_height;
  }

  if(env_width) {
    screen_width = atoi(env_width);
  }
  if(env_height) {
    screen_height = atoi(env_height);
  }

  fsr_set_real_mode(screen_width, screen_height);

  NtUserGetClientRect(global_vulkan_hwnd, &window_rect);

  if(window_rect.right == 0)
    return VK_FALSE;



  //real res equals user res
  if(window_rect.right == screen_width && window_rect.bottom == screen_height &&
  fsr_matches_current_mode(window_rect.right, window_rect.bottom)  ) {
    TRACE("Disabling FSR \n");
    global_fsr = 0;
    return VK_FALSE;
  } else {
    global_fsr = 1;
  }

  fsr_set_current_mode(window_rect.right, window_rect.bottom);


  if(real_sz){
    real_sz->width = screen_width;
    real_sz->height = screen_height;
  }

  if(user_sz){

      user_sz->width = window_rect.right;
      user_sz->height = window_rect.bottom;

  }

  if(dst_blit){
    dst_blit->offset.x = 0;
    dst_blit->offset.y = 0;
    dst_blit->extent.width = screen_width;
    dst_blit->extent.height = screen_height;
  }

  if(filter)
    *filter = VK_FILTER_NEAREST;

  if(sharpness)
    *sharpness = (float) 2 / 10.0f;


  return VK_TRUE;
}
#endif

static void *WAYLANDDRV_vkGetDeviceProcAddr(VkDevice device, const char *name)
{
    void *proc_addr;

    //TRACE("%p, %s\n", device, debugstr_a(name));

    if ((proc_addr = get_vulkan_driver_device_proc_addr(&vulkan_funcs, name)))
        return proc_addr;

    return pvkGetDeviceProcAddr(device, name);
}


static void *WAYLANDDRV_vkGetInstanceProcAddr(VkInstance instance, const char *name)
{
    void *proc_addr;

    //TRACE("%p, %s\n", instance, debugstr_a(name));

    if ((proc_addr = get_vulkan_driver_instance_proc_addr(&vulkan_funcs, instance, name) ))
        return proc_addr;

    return pvkGetInstanceProcAddr(instance, name);
}


static const struct vulkan_funcs vulkan_funcs =
{

    .p_vkCreateInstance = WAYLANDDRV_vkCreateInstance,
    .p_vkCreateSwapchainKHR = WAYLANDDRV_vkCreateSwapchainKHR,
    .p_vkCreateWin32SurfaceKHR = WAYLANDDRV_vkCreateWin32SurfaceKHR,

    .p_vkDestroyInstance = WAYLANDDRV_vkDestroyInstance,
    .p_vkDestroySurfaceKHR = WAYLANDDRV_vkDestroySurfaceKHR,
    .p_vkDestroySwapchainKHR = WAYLANDDRV_vkDestroySwapchainKHR,
    .p_vkEnumerateInstanceExtensionProperties = WAYLANDDRV_vkEnumerateInstanceExtensionProperties,

    .p_vkGetDeviceProcAddr = WAYLANDDRV_vkGetDeviceProcAddr,
    .p_vkGetInstanceProcAddr = WAYLANDDRV_vkGetInstanceProcAddr,


    .p_vkGetPhysicalDeviceWin32PresentationSupportKHR = WAYLANDDRV_vkGetPhysicalDeviceWin32PresentationSupportKHR,

    .p_vkGetSwapchainImagesKHR = WAYLANDDRV_vkGetSwapchainImagesKHR,
    .p_vkQueuePresentKHR = WAYLANDDRV_vkQueuePresentKHR,
    .p_wine_get_host_surface = WAYLANDDRV_wine_get_host_surface,
    #ifdef HAS_FSR
    .query_fsr = WAYLANDDRV_query_fsr
    #endif
};

const struct vulkan_funcs *get_vulkan_driver(UINT version)
{

    static pthread_once_t init_once = PTHREAD_ONCE_INIT;

    pthread_once(&init_once, wine_vk_init);
    if (vulkan_handle)
        return &vulkan_funcs;

    return NULL;
}
