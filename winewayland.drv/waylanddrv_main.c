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

#include "ntstatus.h"


WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);
WINE_DECLARE_DEBUG_CHANNEL(synchronous);
WINE_DECLARE_DEBUG_CHANNEL(winediag);


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


static char input_style[20];

#define IS_OPTION_TRUE(ch) \
    ((ch) == 'y' || (ch) == 'Y' || (ch) == 't' || (ch) == 'T' || (ch) == '1')
#define IS_OPTION_FALSE(ch) \
    ((ch) == 'n' || (ch) == 'N' || (ch) == 'f' || (ch) == 'F' || (ch) == '0')



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




}

/* Registry key and value names */
static const WCHAR ComputerW[] = {'\\','R','e','g','i','s','t','r','y','\\',
                                  'M','a','c','h','i','n','e','\\',
                                  'S','y','s','t','e','m','\\',
                                  'C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\',
                                  'C','o','n','t','r','o','l','\\',
                                  'C','o','m','p','u','t','e','r','N','a','m','e',0};
static const WCHAR ActiveComputerNameW[] =   {'A','c','t','i','v','e','C','o','m','p','u','t','e','r','N','a','m','e',0};
static const WCHAR ComputerNameW[] = {'C','o','m','p','u','t','e','r','N','a','m','e',0};

static const WCHAR default_ComputerName[] = {'W','I','N','E',0};


/*********************************************************************** 
 *                      COMPUTERNAME_Init    (INTERNAL)
 */
void fix_computername_init (void)
{
    HANDLE hkey = INVALID_HANDLE_VALUE, hsubkey = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES attr;
    UNICODE_STRING nameW;
    char buf[offsetof( KEY_VALUE_PARTIAL_INFORMATION, Data ) + (MAX_COMPUTERNAME_LENGTH + 1) * sizeof( WCHAR )];
    DWORD len = sizeof( buf );
    const WCHAR *computer_name = (WCHAR *)(buf + offsetof( KEY_VALUE_PARTIAL_INFORMATION, Data ));
    NTSTATUS st = STATUS_INTERNAL_ERROR;
    char hbuf[256];
    WCHAR *dot, bufW[256];

 

    TRACE("(void)\n");
    InitializeObjectAttributes( &attr, &nameW, 0, 0, NULL );
    RtlInitUnicodeString( &nameW, ComputerW );
    if ( ( st = NtCreateKey( &hkey, KEY_ALL_ACCESS, &attr, 0, NULL, 0, NULL ) ) != STATUS_SUCCESS )
        goto out;

    attr.RootDirectory = hkey;
    RtlInitUnicodeString( &nameW, ComputerNameW );
    if ( (st = NtCreateKey( &hsubkey, KEY_ALL_ACCESS, &attr, 0, NULL, 0, NULL ) ) != STATUS_SUCCESS )
        goto out;

    st = NtQueryValueKey( hsubkey, &nameW, KeyValuePartialInformation, buf, len, &len );

    if ( st != STATUS_SUCCESS)
    {
        computer_name = default_ComputerName;
        len = sizeof(default_ComputerName);
    }
    else
    {
        len = (len - offsetof( KEY_VALUE_PARTIAL_INFORMATION, Data ));
    }

    NtClose( hsubkey );
    TRACE(" ComputerName: %s (%u)\n", debugstr_w (computer_name), len);

    RtlInitUnicodeString( &nameW, ActiveComputerNameW );
    if ( ( st = NtCreateKey( &hsubkey, KEY_ALL_ACCESS, &attr, 0, NULL, REG_OPTION_VOLATILE, NULL ) )
         != STATUS_SUCCESS )
        goto out;

    RtlInitUnicodeString( &nameW, ComputerNameW );
    st = NtSetValueKey( hsubkey, &nameW, 0, REG_SZ, computer_name, len );

out:
    NtClose( hsubkey );
    NtClose( hkey );

    if ( st == STATUS_SUCCESS )
        TRACE( "success\n" );
    else
    {
        WARN( "status trying to set ComputerName: %x\n", st );
        SetLastError ( RtlNtStatusToDosError ( st ) );
    }
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
    void (WINAPI *pShellDDEInit)( BOOL ) = NULL;
    
    static const WCHAR cdot[] = {'.',0};
    static const WCHAR cname[] = {'W', 'I', 'V', 'K',0};
    


    SetComputerNameExW( ComputerNamePhysicalDnsDomain, (LPCWSTR)cdot );
    SetComputerNameExW( ComputerNamePhysicalDnsHostname, (LPCWSTR)cname );
    fix_computername_init();

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
    screen_bpp = 32;
    
     TRACE( "Creating desktop %d %d \n\n", screen_width , screen_height ); 
    
    xinerama_init( screen_width , screen_height); 
    
    
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
