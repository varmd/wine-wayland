/*
 * Wine X11drv display settings functions
 *
 * Copyright 2003 Alexander James Pasadyn
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
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define NONAMELESSUNION
#define NONAMELESSSTRUCT

#include "waylanddrv.h"

#include "windef.h"
#include "winreg.h"
#include "wingdi.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);

//fshack
double fs_hack_user_to_real_w = 1., fs_hack_user_to_real_h = 1.;
double fs_hack_real_to_user_w = 1., fs_hack_real_to_user_h = 1.;
static int offs_x = 0, offs_y = 0;
static int fs_width = 0, fs_height = 0;
static int global_current_mode = 0;

static struct screen_size {
    unsigned int width;
    unsigned int height;
} screen_sizes[] = {
    /* 4:3 */
    {2048, 1536},
    /* 5:4 */
    //{1280, 1024},
    {2560, 2048},
    /* 16:9 */
    //{1280,  720},
    {1366,  768},
    {1600,  900},
    {1706, 960},
    {1920, 1080},
    {2560, 1440},
    {3840, 2160},
    /* 16:10 */
    {1920, 1200},
    {2560, 1600}
};


/* create the mode structures */
static void make_modes(void)
{
  
  
    
  
    RECT primary_rect = get_primary_monitor_rect();
    unsigned int i;
    unsigned int screen_width = primary_rect.right - primary_rect.left;
    unsigned int screen_height = primary_rect.bottom - primary_rect.top;

    /* original specified desktop size */
    WAYLANDDRV_Settings_AddOneMode(screen_width, screen_height, 32, 60);
    //WAYLANDDRV_Settings_AddOneMode(1920, 1080, 32, 60);
    
    for (i=0; i<ARRAY_SIZE(screen_sizes); i++)
    {
      if ( (screen_sizes[i].width != screen_width) || (screen_sizes[i].height != screen_height)  )
        WAYLANDDRV_Settings_AddOneMode(screen_sizes[i].width, screen_sizes[i].height, 32, 60);
    }
   
}

static struct waylanddrv_mode_info *dd_modes = NULL;
static unsigned int dd_mode_count = 0;
static unsigned int dd_max_modes = 0;


/* pointers to functions that actually do the hard stuff */
static int (*pGetCurrentMode)(void);
static LONG (*pSetCurrentMode)(int mode);
static const char *handler_name;

/*
 * Set the handlers for resolution changing functions
 * and initialize the master list of modes
 */
struct waylanddrv_mode_info *WAYLANDDRV_Settings_SetHandlers(const char *name,
                                                     int (*pNewGCM)(void),
                                                     LONG (*pNewSCM)(int),
                                                     unsigned int nmodes,
                                                     int reserve_depths)
{
    handler_name = name;
    pGetCurrentMode = pNewGCM;
    pSetCurrentMode = pNewSCM;
    TRACE("Resolution settings now handled by: %s\n", name);

    dd_max_modes = nmodes * 2;

    if (dd_modes) 
    {
        TRACE("Destroying old display modes array\n");
        HeapFree(GetProcessHeap(), 0, dd_modes);
    }
    dd_modes = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*dd_modes) * dd_max_modes);
    dd_mode_count = 0;
    TRACE("Initialized new display modes array\n");
    return dd_modes;
}

/* Add one mode to the master list */
void WAYLANDDRV_Settings_AddOneMode(unsigned int width, unsigned int height, unsigned int bpp, unsigned int freq)
{
    struct waylanddrv_mode_info *info = &dd_modes[dd_mode_count];
    DWORD dwBpp = screen_bpp;
    if (dd_mode_count >= dd_max_modes)
    {
        ERR("Maximum modes (%d) exceeded\n", dd_max_modes);
        return;
    }
    if (bpp == 0)       bpp = dwBpp;
    info->width         = width;
    info->height        = height;
    info->refresh_rate  = freq;
    info->bpp           = bpp;
    
    //TRACE("Resolution initialized mode %d: %dx%dx%d @%d Hz (%s)\n",          dd_mode_count, width, height, bpp, freq, handler_name);
    dd_mode_count++;
}



/* return the number of modes that are initialized */
unsigned int WAYLANDDRV_Settings_GetModeCount(void)
{
    return dd_mode_count;
}

/***********************************************************************
 * Default handlers if resolution switching is not enabled
 *
 */
static int WAYLANDDRV_nores_GetCurrentMode(void)
{
    return 0;
}

static LONG WAYLANDDRV_nores_SetCurrentMode(int mode)
{
  //TODO
  int realMode = 0;
  
  TRACE("Requesting mode change request mode=%d\n", mode);
  
    if (mode == 0) 
      return DISP_CHANGE_SUCCESSFUL;
  
  
  
  
    //fshack
  double width = dd_modes[global_current_mode].width;
  double height = dd_modes[global_current_mode].height;
  if(dd_modes[realMode].width / (double)dd_modes[realMode].height < width / height){ /* real mode is narrower than fake mode */
      /* scale to fit width */
      height = dd_modes[realMode].width * (height / width);
      width = dd_modes[realMode].width;
      offs_x = 0;
      offs_y = (dd_modes[realMode].height - height) / 2;
      fs_width = dd_modes[realMode].width;
      fs_height = (int)height;
  }else{
      /* scale to fit height */
      width = dd_modes[realMode].height * (width / height);
      height = dd_modes[realMode].height;
      offs_x = (dd_modes[realMode].width - width) / 2;
      offs_y = 0;
      fs_width = (int)width;
      fs_height = dd_modes[realMode].height;
  }
  fs_hack_user_to_real_w = width / (double)dd_modes[global_current_mode].width;
  fs_hack_user_to_real_h = height / (double)dd_modes[global_current_mode].height;
  fs_hack_real_to_user_w = dd_modes[global_current_mode].width / (double)width;
  fs_hack_real_to_user_h = dd_modes[global_current_mode].height / (double)height;
  TRACE(" mode %d global_current_mode %d wxh %d %d fs w fs h   %f %f \n", 
    mode, 
    global_current_mode, 
    dd_modes[global_current_mode].width,
    dd_modes[global_current_mode].height,
    fs_hack_real_to_user_w, 
    fs_hack_real_to_user_h
  );
    //fshack
  
  TRACE("Ignoring mode change request mode=%d\n", mode);
  return DISP_CHANGE_SUCCESSFUL;
}

/* default handler only gets the current X desktop resolution */
void WAYLANDDRV_Settings_Init(void)
{
    RECT primary = get_primary_monitor_rect();
    WAYLANDDRV_Settings_SetHandlers("NoRes", 
                                WAYLANDDRV_nores_GetCurrentMode, 
                                WAYLANDDRV_nores_SetCurrentMode, 
                                ARRAY_SIZE(screen_sizes)+2, 0);
    make_modes();
}

#if 0
static BOOL get_display_device_reg_key(char *key, unsigned len)
{
    static const char display_device_guid_prop[] = "__wine_display_device_guid";
    static const char video_path[] = "System\\CurrentControlSet\\Control\\Video\\{";
    static const char display0[] = "}\\0000";
    ATOM guid_atom;

    assert(len >= sizeof(video_path) + sizeof(display0) + 40);

    guid_atom = HandleToULong(GetPropA(GetDesktopWindow(), display_device_guid_prop));
    if (!guid_atom) return FALSE;

    memcpy(key, video_path, sizeof(video_path));

    if (!GlobalGetAtomNameA(guid_atom, key + strlen(key), 40))
        return FALSE;

    strcat(key, display0);

    TRACE("display device key %s\n", wine_dbgstr_a(key));
    return TRUE;
}

static BOOL write_registry_settings(const DEVMODEW *dm)
{
    char wine_x11_reg_key[128];
    HKEY hkey;
    BOOL ret = TRUE;

    if (!get_display_device_reg_key(wine_x11_reg_key, sizeof(wine_x11_reg_key)))
        return FALSE;

    if (RegCreateKeyExA(HKEY_CURRENT_CONFIG, wine_x11_reg_key, 0, NULL,
                        REG_OPTION_VOLATILE, KEY_WRITE, NULL, &hkey, NULL))
        return FALSE;

#define set_value(name, data) \
    if (RegSetValueExA(hkey, name, 0, REG_DWORD, (const BYTE*)(data), sizeof(DWORD))) \
        ret = FALSE

    set_value("DefaultSettings.BitsPerPel", &dm->dmBitsPerPel);
    set_value("DefaultSettings.XResolution", &dm->dmPelsWidth);
    set_value("DefaultSettings.YResolution", &dm->dmPelsHeight);
    set_value("DefaultSettings.VRefresh", &dm->dmDisplayFrequency);
    set_value("DefaultSettings.Flags", &dm->u2.dmDisplayFlags);
    set_value("DefaultSettings.XPanning", &dm->u1.s2.dmPosition.x);
    set_value("DefaultSettings.YPanning", &dm->u1.s2.dmPosition.y);
    set_value("DefaultSettings.Orientation", &dm->u1.s2.dmDisplayOrientation);
    set_value("DefaultSettings.FixedOutput", &dm->u1.s2.dmDisplayFixedOutput);

#undef set_value

    RegCloseKey(hkey);
    return ret;
}
#endif


/***********************************************************************
 *		EnumDisplaySettingsEx  (WAYLANDDRV.@)
 *
 */
BOOL CDECL WAYLANDDRV_EnumDisplaySettingsEx( LPCWSTR name, DWORD n, LPDEVMODEW devmode, DWORD flags)
{
    //static const WCHAR dev_name[CCHDEVICENAME] = { 'W','i','n','e',0 };
    //static const WCHAR dev_name[CCHDEVICENAME] = {'\\','\\','.','\\','D','I','S','P','L','A','Y', '\\', 'W','i','n','e',0};
  
  
    //TRACE("Asking for mode %d %p \n", n, handler_name);    
    if (n == ENUM_CURRENT_SETTINGS)
    {
        //TRACE("mode %d (current) -- getting current mode (%s)\n", n, handler_name);
        n = 0;
    }
    if (n == ENUM_REGISTRY_SETTINGS)
    {
        //TRACE("mode %d (registry) -- getting default mode (%s)\n", n, handler_name);
        n = 0;
        //return read_registry_settings(devmode);
    }
      
        
    devmode->dmSize = FIELD_OFFSET(DEVMODEW, dmICMMethod);
    devmode->dmSpecVersion = DM_SPECVERSION;
    devmode->dmDriverVersion = DM_SPECVERSION;
    //memcpy(devmode->dmDeviceName, dev_name, sizeof(dev_name));
    devmode->dmDriverExtra = 0;
    devmode->u2.dmDisplayFlags = 0;
    devmode->dmDisplayFrequency = 0;
    devmode->dmBitsPerPel = 32;
    devmode->u1.s2.dmPosition.x = 0;
    devmode->u1.s2.dmPosition.y = 0;
    devmode->u1.s2.dmDisplayOrientation = 0;
    devmode->u1.s2.dmDisplayFixedOutput = 0;
        
    if (n < dd_mode_count)
    {
        devmode->dmPelsWidth = dd_modes[n].width;
        devmode->dmPelsHeight = dd_modes[n].height;
        devmode->dmBitsPerPel = dd_modes[n].bpp;
        devmode->dmDisplayFrequency = dd_modes[n].refresh_rate;
        devmode->dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL |
                            DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY;
        return TRUE;
    }
    TRACE("mode %d -- not present (%s)\n", n, handler_name);
    SetLastError(ERROR_NO_MORE_FILES);
    return FALSE;
}


/***********************************************************************
 *		ChangeDisplaySettingsEx  (WAYLANDDRV.@)
 *
 */
LONG CDECL WAYLANDDRV_ChangeDisplaySettingsEx( LPCWSTR devname, LPDEVMODEW devmode,
                                           HWND hwnd, DWORD flags, LPVOID lpvoid )
{
    
    
  
    DWORD i, mode, dwBpp = 0;
    DEVMODEW dm;
    BOOL def_mode = TRUE;

    TRACE(" - (%s,%p,%p,0x%08x,%p)\n",debugstr_w(devname),devmode,hwnd,flags,lpvoid);
    //TRACE("flags=%s\n",_CDS_flags(flags));
    if (devmode)
    {
        /* this is the minimal dmSize that XP accepts */
        if (devmode->dmSize < FIELD_OFFSET(DEVMODEW, dmFields))
            return DISP_CHANGE_FAILED;

        /*
        TRACE("DM_fields=%s\n",_DM_fields(devmode->dmFields));
        TRACE("width=%d height=%d bpp=%d freq=%d (%s)\n",
              devmode->dmPelsWidth,devmode->dmPelsHeight,
              devmode->dmBitsPerPel,devmode->dmDisplayFrequency, handler_name);
        */  
       
        dwBpp = devmode->dmBitsPerPel;
        if (devmode->dmFields & DM_BITSPERPEL) def_mode &= !dwBpp;
        if (devmode->dmFields & DM_PELSWIDTH)  def_mode &= !devmode->dmPelsWidth;
        if (devmode->dmFields & DM_PELSHEIGHT) def_mode &= !devmode->dmPelsHeight;
        if (devmode->dmFields & DM_DISPLAYFREQUENCY) def_mode &= !devmode->dmDisplayFrequency;
    }

    if (def_mode || !dwBpp)
    {
        if (!WAYLANDDRV_EnumDisplaySettingsEx(devname, ENUM_REGISTRY_SETTINGS, &dm, 0))
        {
            ERR("Default mode not found!\n");
            return DISP_CHANGE_BADMODE;
        }
        if (def_mode)
        {
            TRACE("Return to original display mode (%s)\n", handler_name);
            devmode = &dm;
        }
        dwBpp = dm.dmBitsPerPel;
    }

    if ((devmode->dmFields & (DM_PELSWIDTH | DM_PELSHEIGHT)) != (DM_PELSWIDTH | DM_PELSHEIGHT))
    {
        WARN("devmode doesn't specify the resolution: %04x\n", devmode->dmFields);
        return DISP_CHANGE_BADMODE;
    }

    mode = ENUM_CURRENT_SETTINGS;
    for (i = 0; i < dd_mode_count; i++)
    {
        if (devmode->dmFields & DM_BITSPERPEL)
        {
            if (dwBpp != dd_modes[i].bpp)
                continue;
        }
        if (devmode->dmFields & DM_PELSWIDTH)
        {
            if (devmode->dmPelsWidth != dd_modes[i].width)
                continue;
        }
        if (devmode->dmFields & DM_PELSHEIGHT)
        {
            if (devmode->dmPelsHeight != dd_modes[i].height)
                continue;
        }
        if ((devmode->dmFields & DM_DISPLAYFREQUENCY) &&
            devmode->dmDisplayFrequency != 0)
        {
            if (dd_modes[i].refresh_rate != 0 &&
                devmode->dmDisplayFrequency != dd_modes[i].refresh_rate)
                continue;
        }
        else if (default_display_frequency != 0)
        {
            if (dd_modes[i].refresh_rate != 0 &&
                default_display_frequency == dd_modes[i].refresh_rate)
            {
                TRACE("Found display mode %d with default frequency (%s)\n", i, handler_name);
                mode = i;
                break;
            }
        }

        if (mode == ENUM_CURRENT_SETTINGS)
            mode = i;
    }

    if (mode == ENUM_CURRENT_SETTINGS)
    {
        /* no valid modes found */
        ERR("No matching mode found %ux%ux%u @%u! (%s)\n",
            devmode->dmPelsWidth, devmode->dmPelsHeight,
            devmode->dmBitsPerPel, devmode->dmDisplayFrequency, handler_name);
        return DISP_CHANGE_BADMODE;
    }

    /* we have a valid mode */
    TRACE("Requested display settings match mode %d (%s)\n", mode, handler_name);
    global_current_mode = mode;
    //if (flags & CDS_UPDATEREGISTRY)
    //    write_registry_settings(devmode);

    if (!(flags & (CDS_TEST | CDS_NORESET)))
        return pSetCurrentMode(mode);

    return DISP_CHANGE_SUCCESSFUL;
}


//fshack



BOOL fs_hack_enabled(void)
{
    return TRUE;
}



void fs_hack_set_current_mode(int width, int height)
{
  DEVMODEW dm;
  TRACE("fs_hack_set_current_mode %d %d \n", width, height);
  for(int i = 0; i < dd_mode_count; ++i)
  {
      if(dd_modes[i].width == width &&
              dd_modes[i].height == height)
      {
        global_current_mode = i;
        xinerama_init(width, height);
        WAYLANDDRV_nores_SetCurrentMode(i);
        WAYLANDDRV_EnumDisplaySettingsEx(NULL, ENUM_REGISTRY_SETTINGS, &dm, 0);
        dm.dmPelsWidth = dd_modes[i].width;
        dm.dmPelsHeight = dd_modes[i].height;
        ChangeDisplaySettingsExW(NULL, &dm, NULL, CDS_RESET, NULL);
        break;
     }
  }
    
  
}

BOOL fs_hack_matches_real_mode(int w, int h)
{
  if(w == dd_modes[global_current_mode].width && h == dd_modes[global_current_mode].height) {
    return TRUE;  
  }
  return FALSE;
}

BOOL fs_hack_matches_last_mode(int w, int h)
{
    return w == fs_width && h == fs_height;
}

void fs_hack_scale_user_to_real(POINT *pos)
{
  TRACE("from %d,%d\n", pos->x, pos->y);
  pos->x = lround(pos->x * fs_hack_user_to_real_w);
  pos->y = lround(pos->y * fs_hack_user_to_real_h);
  TRACE("to %d,%d\n", pos->x, pos->y);
}


void fs_hack_user_to_real(POINT *pos)
{
    
    
  TRACE("from %d,%d\n", pos->x, pos->y);
  fs_hack_scale_user_to_real(pos);
  pos->x += offs_x;
  pos->y += offs_y;
  TRACE("to %d,%d\n", pos->x, pos->y);
    
  
}

void fs_hack_scale_real_to_user(POINT *pos)
{
  TRACE("from %d,%d\n", pos->x, pos->y);
  pos->x = lround(pos->x * fs_hack_real_to_user_w);
  pos->y = lround(pos->y * fs_hack_real_to_user_h);
  TRACE("to %d,%d\n", pos->x, pos->y);
}

void fs_hack_real_to_user_relative(double *x, double *y)
{
  TRACE("REL pos from %f,%f\n", *x, *y);
  *x = *x * fs_hack_real_to_user_w;
  *y = *y * fs_hack_real_to_user_h;
  TRACE("REL pos to %f,%f\n", *x, *y);
  
}

void fs_hack_real_to_user(POINT *pos)
{
    
        TRACE("from %d,%d\n", pos->x, pos->y);
  
        

        if(pos->x <= offs_x)
            pos->x = 0;
        else
            pos->x -= offs_x;

        if(pos->y <= offs_y)
            pos->y = 0;
        else
            pos->y -= offs_y;

        if(pos->x >= fs_width)
            pos->x = fs_width - 1;
        if(pos->y >= fs_height)
            pos->y = fs_height - 1;
        
        

        fs_hack_scale_real_to_user(pos);

        TRACE("to %d,%d\n", pos->x, pos->y);
    
}

void fs_hack_rect_user_to_real(RECT *rect)
{
    fs_hack_user_to_real((POINT *)&rect->left);
    fs_hack_user_to_real((POINT *)&rect->right);
}

//fshack