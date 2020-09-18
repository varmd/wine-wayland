/*
 * WAYLANDDRV initialization code
 *
 * Copyright 1998 Patrik Stridvall
 * Copyright 2000 Alexandre Julliard
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
#include "wine/port.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc.h>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#define DESKTOP_CLASS_ATOM ((LPCWSTR)MAKEINTATOM(32769))
#define DESKTOP_ALL_ACCESS 0x01ff

#include "windef.h"
#include "winbase.h"
#include "winreg.h"

#include "waylanddrv.h"

#include "wine/server.h"
#include "wine/unicode.h"
#include "wine/debug.h"


WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);
WINE_DECLARE_DEBUG_CHANNEL(synchronous);
WINE_DECLARE_DEBUG_CHANNEL(winediag);



//Colormap default_colormap = None;
//XPixmapFormatValues **pixmap_formats;
unsigned int screen_bpp;
Window root_window;

BOOL use_xkb = FALSE;
BOOL use_take_focus = FALSE;
BOOL use_primary_selection = FALSE;
BOOL use_system_cursors = TRUE;
BOOL show_systray = FALSE;
BOOL grab_pointer = TRUE;
BOOL grab_fullscreen = FALSE;
BOOL managed_mode = FALSE;
BOOL decorated_mode = FALSE;
BOOL private_color_map = FALSE;
int primary_monitor = 0;
BOOL client_side_graphics = TRUE;
BOOL client_side_with_render = TRUE;
BOOL shape_layered_windows = FALSE;
int copy_default_colors = 128;
int alloc_system_colors = 256;
int default_display_frequency = 0;
DWORD thread_data_tls_index = TLS_OUT_OF_INDEXES;
int xrender_error_base = 0;
HMODULE waylanddrv_module = 0;
char *process_name = NULL;

//static waylanddrv_error_callback err_callback;   /* current callback for error */
//static Display *err_callback_display;        /* display callback is set for */
static void *err_callback_arg;               /* error callback argument */
static int err_callback_result;              /* error callback result */

//static int (*old_error_handler)( Display *, XErrorEvent * );
//static BOOL use_xim = FALSE;
static char input_style[20];

#define IS_OPTION_TRUE(ch) \
    ((ch) == 'y' || (ch) == 'Y' || (ch) == 't' || (ch) == 'T' || (ch) == '1')
#define IS_OPTION_FALSE(ch) \
    ((ch) == 'n' || (ch) == 'N' || (ch) == 'f' || (ch) == 'F' || (ch) == '0')



/***********************************************************************
 *		init_pixmap_formats
 */
static void init_pixmap_formats( Display *display )
{
    
    /*
    int i, count, max = 32;
    XPixmapFormatValues *formats = XListPixmapFormats( display, &count );

    for (i = 0; i < count; i++)
    {
        TRACE( "depth %u, bpp %u, pad %u\n",
               formats[i].depth, formats[i].bits_per_pixel, formats[i].scanline_pad );
        if (formats[i].depth > max) max = formats[i].depth;
    }
    pixmap_formats = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*pixmap_formats) * (max + 1) );
    for (i = 0; i < count; i++) pixmap_formats[formats[i].depth] = &formats[i];
    */
}


/***********************************************************************
 *		get_config_key
 *
 * Get a config key from either the app-specific or the default config
 */
static inline DWORD get_config_key( HKEY defkey, HKEY appkey, const char *name,
                                    char *buffer, DWORD size )
{
    if (appkey && !RegQueryValueExA( appkey, name, 0, NULL, (LPBYTE)buffer, &size )) return 0;
    if (defkey && !RegQueryValueExA( defkey, name, 0, NULL, (LPBYTE)buffer, &size )) return 0;
    return ERROR_FILE_NOT_FOUND;
}



static void init_visuals( int screen )
{
    int count;


    /*
    argb_visual.screen     = screen;
    argb_visual.class      = TrueColor;
    argb_visual.depth      = 32;
    argb_visual.red_mask   = 0xff0000;
    argb_visual.green_mask = 0x00ff00;
    argb_visual.blue_mask  = 0x0000ff;
    */
  
    /*
    if ((info = XGetVisualInfo( display, VisualScreenMask | VisualDepthMask | VisualClassMask |
                                VisualRedMaskMask | VisualGreenMaskMask | VisualBlueMaskMask,
                                &argb_visual, &count )))
    {
        argb_visual = *info;
        XFree( info );
    }*/

    //default_visual.screen = screen;
    
  /*  
    if (default_visual.depth) 
    {
        if (default_visual.depth == 32 && argb_visual.visual)
        {
            
        }
        else if ((info = XGetVisualInfo( display, VisualScreenMask | VisualDepthMask, &default_visual, &count )))
        {
            default_visual = *info;
            XFree( info );
        }
        else WARN( "no visual found for depth %d\n", default_visual.depth );
    }
*/
  /*

    if (!default_visual.visual)
    {
        //default_visual.depth         = DefaultDepth( display, screen );
        default_visual.depth         = 32;
        //default_visual.visual        = DefaultVisual( display, screen );
        //default_visual.visualid      = default_visual.visual->visualid;
        
        default_visual.class         = default_visual.visual->class;
        default_visual.red_mask      = default_visual.visual->red_mask;
        default_visual.green_mask    = default_visual.visual->green_mask;
        default_visual.blue_mask     = default_visual.visual->blue_mask;
        default_visual.colormap_size = default_visual.visual->map_entries;
        default_visual.bits_per_rgb  = default_visual.visual->bits_per_rgb;
        
        default_visual.class         = TrueColor;
        default_visual.red_mask      = 0xff0000;
        default_visual.green_mask    = 0x00ff00;
        default_visual.blue_mask     = 0x0000ff;
        //default_visual.colormap_size = default_visual.visual->map_entries;
        //default_visual.bits_per_rgb  = default_visual.visual->bits_per_rgb
      
         
    }
    */



}


void manage_desktop(  )
{
    static const WCHAR messageW[] = {'M','e','s','s','a','g','e',0};
    HDESK desktop = 0;
    GUID guid;
    MSG msg;
    HWND hwnd;
    HMODULE graphics_driver;
    unsigned int width, height;
    WCHAR *cmdline = NULL, *driver = NULL;
    const WCHAR *name = NULL;
    static const WCHAR desktopW[] = {'D','e','s','k','t','o','p',0};
    BOOL enable_shell = FALSE;
    void (WINAPI *pShellDDEInit)( BOOL ) = NULL;





        if (!(desktop = CreateDesktopW( desktopW, NULL, NULL, 0, DESKTOP_ALL_ACCESS, NULL )))
        {
            WINE_ERR( "failed to create desktop %s error %d\n", wine_dbgstr_w(name), GetLastError() );
            ExitProcess( 1 );
        }
        SetThreadDesktop( desktop );




    /* create the desktop window */
    hwnd = CreateWindowExW( 0, DESKTOP_CLASS_ATOM, NULL,
                            WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 0, 0, 0, 0, 0, NULL );

    if (hwnd)
    {
        /* create the HWND_MESSAGE parent */
        CreateWindowExW( 0, messageW, NULL, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 100, 100, 0, 0, 0, NULL );

    
        SetWindowPos( hwnd, 0, GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
                      GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
                      SWP_SHOWWINDOW );
        ClipCursor( NULL );
        

       
    }

}

/***********************************************************************
 *           WAYLANDDRV process initialisation routine
 */
static BOOL process_attach(void)
{
  
    
  
  
    char *env_width = getenv( "WINE_VK_WAYLAND_WIDTH" );
    char *env_height = getenv( "WINE_VK_WAYLAND_HEIGHT" );
    
    int screen_width = 1440;
    int screen_height = 900;
    
    if(env_width) {
      screen_width = atoi(env_width);
    }
    if(env_height) {
      screen_height = atoi(env_height);
    }
    
    init_visuals( 0);
    //screen_bpp = pixmap_formats[default_visual.depth]->bits_per_pixel;
    screen_bpp = 24;
    
     TRACE( "Creating desktop %d %d \n\n", screen_width , screen_height ); 
    
    xinerama_init( screen_width , screen_height); 
    
    //esync
    //?? is this really needed
    //set_queue_display_fd( 0 );
  
    
    /*
    
  static const WCHAR messageW[] = {'M','e','s','s','a','g','e',0};
    HDESK desktop = 0;
    GUID guid;
    MSG msg;
    HWND hwnd;
    
    unsigned int width, height;
    WCHAR *cmdline = NULL, *driver = NULL;
    
    const WCHAR *name = NULL;
    BOOL enable_shell = FALSE;
    void (WINAPI *pShellDDEInit)( BOOL ) = NULL;

    


    //if (name)
    //    enable_shell = get_default_enable_shell( name );

    //if (name && width && height)
    //{
    desktop = CreateDesktopW( "test", NULL, NULL, 0, DESKTOP_ALL_ACCESS, NULL );
  
    if (!desktop)
    {
        ERR( "failed to create desktop %s error %d\n", wine_dbgstr_w(name), GetLastError() );
        ExitProcess( 1 );
    }
      SetThreadDesktop( desktop );
    

    //UuidCreate( &guid );
    
    

    // create the desktop window
    hwnd = CreateWindowExW( 0, DESKTOP_CLASS_ATOM, NULL,
                            WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 1440, 900, 0, 0, 0, NULL ); //&guid
    
    
    TRACE( "Creating desktop %p \n\n", hwnd );    
    

    if (hwnd)
    {
        //CreateWindowExW( 0, messageW, NULL, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        //                 0, 0, 100, 100, 0, 0, 0, NULL );

        //desktop_orig_wndproc = (WNDPROC)SetWindowLongPtrW( hwnd, GWLP_WNDPROC,
        //    (LONG_PTR)desktop_wnd_proc );
        
        //SendMessageW( hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIconW( 0, MAKEINTRESOURCEW(OIC_WINLOGO)));
        //if (name) set_desktop_window_title( hwnd, name );
        SetWindowPos( hwnd, 0, GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
                      GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
                      SWP_SHOWWINDOW );
        SystemParametersInfoW( SPI_SETDESKWALLPAPER, 0, NULL, FALSE );
        ClipCursor( NULL );  
    }
  
*/
  
    //struct user_thread_info *thread_info = get_user_thread_info();
  
    char error[1024];
    //Display *display;
    //void *libx11 = wine_dlopen( "libX11.so", RTLD_NOW|RTLD_GLOBAL, error, sizeof(error) );

//    if (!libx11)
//    {
        //ERR( "failed to load libx11");
        //return FALSE;
//    }
    //pXGetEventData = wine_dlsym( libx11, "XGetEventData", NULL, 0 );
    //pXFreeEventData = wine_dlsym( libx11, "XFreeEventData", NULL, 0 );
//#ifdef SONAME_LIBXEXT
    //wine_dlopen( SONAME_LIBXEXT, RTLD_NOW|RTLD_GLOBAL, NULL, 0 );
//#endif


    

    /* Open display */

      //return FALSE;
      


    //fcntl( ConnectionNumber(display), F_SETFD, 1 ); /* set close on exec flag */
    //gdi_display = display;

    //init_pixmap_formats( display );
    //init_visuals( display, DefaultScreen( display ));
    

    //XInternAtoms( display, (char **)atom_names, NB_XATOMS - FIRST_XATOM, False, WAYLANDDRV_Atoms );

    //winContext = XUniqueContext();
    //win_data_context = XUniqueContext();
    //cursor_context = XUniqueContext();

    //if (TRACE_ON(synchronous)) XSynchronize( display, True );

    /*
    xinerama_init( DisplayWidth( display, default_visual.screen ),
                   DisplayHeight( display, default_visual.screen ));
                
    
    */
    WAYLANDDRV_Settings_Init();
    
    
    
    //WAYLANDDRV_init_desktop( 1440 , 900);



    //WAYLANDDRV_InitKeyboard( gdi_display );

    
    
    TRACE( "Creating desktop done %d %d \n\n", screen_width , screen_height ); 
    
    
    if ((thread_data_tls_index = TlsAlloc()) == TLS_OUT_OF_INDEXES) {
      return FALSE;
    }
    
    manage_desktop();
    
    return TRUE;
}






/***********************************************************************
 *           WAYLANDDRV thread initialisation routine
 */
struct waylanddrv_thread_data *waylanddrv_init_thread_data(void)
{
    
  
  
    
    struct waylanddrv_thread_data *data = waylanddrv_thread_data();

    if (data) return data;
  
  /*

    if (!(data = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*data) )))
    {
        ERR( "could not create data\n" );
        ExitProcess(1);
    }
    if (!(data->display = XOpenDisplay(NULL)))
    {
        ERR_(winediag)( "waylanddrv: Can't open display: %s. Please ensure that your X server is running and that $DISPLAY is set correctly.\n", XDisplayName(NULL));
        //ExitProcess(1);
    }

    fcntl( ConnectionNumber(data->display), F_SETFD, 1 ); // set close on exec flag


    
    TlsSetValue( thread_data_tls_index, data );



    return data;
    */
}


/***********************************************************************
 *           WAYLANDDRV initialisation routine
 */
BOOL WINAPI DllMain( HINSTANCE hinst, DWORD reason, LPVOID reserved )
{
    BOOL ret = TRUE;

    switch(reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls( hinst );
        waylanddrv_module = hinst;
        ret = process_attach();
        break;
    }
    return ret;
}
