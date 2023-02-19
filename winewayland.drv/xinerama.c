/*
 * Xinerama support
 *
 * Copyright 2006 Alexandre Julliard
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


#include "config.h"
#include <stdarg.h>
#include <stdlib.h>



#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "winreg.h"

#include "setupapi.h"
#define WIN32_NO_STATUS
#include "winternl.h"
#include "wine/debug.h"
#include "waylanddrv.h"

WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);

static RECT virtual_screen_rect;

static struct screen_size {
    unsigned int width;
    unsigned int height;
} screen_sizes[] = {
    {0, 0},
    {1920, 1080},
    {1366,  768},
    {1600,  900},
    {1706, 960},
    {2048, 1536},
    {2560, 1440},
    {3840, 2160}
};

//fsr
double fsr_user_to_real_w = 1., fsr_user_to_real_h = 1.;
double fsr_real_to_user_w = 1., fsr_real_to_user_h = 1.;
static int offs_x = 0, offs_y = 0;
static int fs_width = 0, fs_height = 0;
static int global_current_mode = 0;
static int global_real_mode = 0;


static LONG fsr_set_sizes(int mode)
{
  //TODO
  int realMode = 0;
  double width, height;

  TRACE("Requesting mode change request mode=%d\n", mode);

  if (mode == 0)
    return 1;


  //fsr
  width = screen_sizes[global_current_mode].width;
  height = screen_sizes[global_current_mode].height;
  if(screen_sizes[realMode].width / (double)screen_sizes[realMode].height < width / height){ /* real mode is narrower than fake mode */
      /* scale to fit width */
      height = screen_sizes[realMode].width * (height / width);
      width = screen_sizes[realMode].width;
      offs_x = 0;
      offs_y = (screen_sizes[realMode].height - height) / 2;
      fs_width = screen_sizes[realMode].width;
      fs_height = (int)height;
  }else{
      /* scale to fit height */
      width = screen_sizes[realMode].height * (width / height);
      height = screen_sizes[realMode].height;
      offs_x = (screen_sizes[realMode].width - width) / 2;
      offs_y = 0;
      fs_width = (int)width;
      fs_height = screen_sizes[realMode].height;
  }
  fsr_user_to_real_w = width / (double)screen_sizes[global_current_mode].width;
  fsr_user_to_real_h = height / (double)screen_sizes[global_current_mode].height;
  fsr_real_to_user_w = screen_sizes[global_current_mode].width / (double)width;
  fsr_real_to_user_h = screen_sizes[global_current_mode].height / (double)height;
  TRACE(" mode %d global_current_mode %d wxh %d %d fs w fs h   %f %f \n",
    mode,
    global_current_mode,
    screen_sizes[global_current_mode].width,
    screen_sizes[global_current_mode].height,
    fsr_real_to_user_w,
    fsr_real_to_user_h
  );
    //fsr

  //TRACE("Ignoring mode change request mode=%d\n", mode);
  return 1;
}



//fsr

void fsr_set_real_mode(int width, int height)
{
  for (int i=0; i < ARRAY_SIZE(screen_sizes); i++)
  {
    if(screen_sizes[i].width == width &&
            screen_sizes[i].height == height)
    {
      global_real_mode = i;
    }
  }
}

void fsr_set_current_mode(int width, int height)
{
  TRACE("fsr_set_current_mode %d %d \n", width, height);
  for (int i=0; i < ARRAY_SIZE(screen_sizes); i++)
  {
      if(screen_sizes[i].width == width &&
              screen_sizes[i].height == height)
      {
        global_current_mode = i;
        fsr_set_sizes(i);
        break;
     }
  }


}

BOOL fsr_matches_real_mode(int w, int h)
{
  if(w == screen_sizes[global_real_mode].width
    && h == screen_sizes[global_real_mode].height) {
    return TRUE;
  }
  return FALSE;
}

BOOL fsr_matches_current_mode(int w, int h)
{
  if(w == screen_sizes[global_current_mode].width
    && h == screen_sizes[global_current_mode].height) {
    return TRUE;
  }
  return FALSE;
}

BOOL fsr_matches_last_mode(int w, int h)
{
    return w == fs_width && h == fs_height;
}

void fsr_scale_user_to_real(POINT *pos)
{
  //TRACE("from %d,%d\n", pos->x, pos->y);
  pos->x = lround(pos->x * fsr_user_to_real_w);
  pos->y = lround(pos->y * fsr_user_to_real_h);
  //TRACE("to %d,%d\n", pos->x, pos->y);
}


void fsr_user_to_real(POINT *pos)
{


  TRACE("from %d,%d\n", pos->x, pos->y);
  fsr_scale_user_to_real(pos);
  pos->x += offs_x;
  pos->y += offs_y;
  TRACE("to %d,%d\n", pos->x, pos->y);


}

void fsr_scale_real_to_user(POINT *pos)
{
  //TRACE("from %d,%d\n", pos->x, pos->y);
  pos->x = lround(pos->x * fsr_real_to_user_w);
  pos->y = lround(pos->y * fsr_real_to_user_h);
  //TRACE("to %d,%d\n", pos->x, pos->y);
}

void fsr_real_to_user_relative(double *x, double *y)
{
  TRACE("REL pos from %f,%f\n", *x, *y);
  *x = *x * fsr_real_to_user_w;
  *y = *y * fsr_real_to_user_h;
  TRACE("REL pos to %f,%f\n", *x, *y);
}

void fsr_real_to_user(POINT *pos)
{
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

    fsr_scale_real_to_user(pos);
}

/*
static void fsr_rect_user_to_real(RECT *rect)
{
    fsr_user_to_real((POINT *)&rect->left);
    fsr_user_to_real((POINT *)&rect->right);
}
*/

//end fsr

static RECT monitor_default_rect(int width, int height, int force) {
  static int done = 0;
  static RECT rect;
  if(!done || force ) {
    SetRectEmpty( &rect );
    SetRect( &rect, 0, 0, width, height );
    done = 1;
  }
  return rect;
}

RECT get_virtual_screen_rect(void)
{
    return virtual_screen_rect;
}

RECT get_primary_monitor_rect(void)
{
    return monitor_default_rect(0, 0, 0);
}

void xinerama_init( unsigned int width, unsigned int height )
{
    RECT rect;

    monitor_default_rect(width, height, 1);

    SetRectEmpty( &virtual_screen_rect );
    rect = monitor_default_rect(0, 0, 0);
    SetRect( &virtual_screen_rect, 0, 0, rect.right, rect.bottom );
    screen_sizes[0].width = width;
    screen_sizes[0].height = height;

    TRACE("Virtual rect %d %d %s \n",  width, height, wine_dbgstr_rect( &virtual_screen_rect ));

    #if 0
    static const WCHAR generic_device[] = {
        'G','e','n','e','r','i','c',' ',
        'D','e','v','i','c','e',0};

        UNICODE_STRING device_name;
    RtlInitUnicodeString(&device_name, generic_device);

    NtUserChangeDisplaySettings(&device_name, &mode, NULL,
      CDS_GLOBAL | CDS_NORESET | CDS_UPDATEREGISTRY, NULL);
   #endif
}

static BOOL desktop_get_gpus( struct gdi_gpu **new_gpus, int *count )
{
    struct gdi_gpu *gpu;
    static const WCHAR wine_adapterW[] = {'W','a','y','l','a','n','d','G','P','U',0};

    gpu = calloc( 1, sizeof(*gpu) );
    if (!gpu) return FALSE;

    lstrcpyW( gpu->name, wine_adapterW );

    *new_gpus = gpu;
    *count = 1;
    return TRUE;
}

static BOOL desktop_get_adapters( ULONG_PTR gpu_id,
  struct gdi_adapter **new_adapters, int *count )
{
    struct gdi_adapter *adapter;

    adapter = calloc( 1, sizeof(*adapter) );
    if (!adapter) return FALSE;

    adapter->state_flags = DISPLAY_DEVICE_PRIMARY_DEVICE;
    adapter->state_flags |= DISPLAY_DEVICE_ATTACHED_TO_DESKTOP;

    *new_adapters = adapter;
    *count = 1;
    return TRUE;
}

static BOOL desktop_add_monitors( const struct gdi_device_manager *device_manager, void *param )
{
    struct gdi_monitor monitor = {0};
    UINT len = 0;

    SetRect(&monitor.rc_monitor, 0, 0,
            screen_sizes[0].width,
            screen_sizes[0].height);

    monitor.rc_work = monitor.rc_monitor;
    monitor.state_flags = DISPLAY_DEVICE_ATTACHED | DISPLAY_DEVICE_ACTIVE;

    device_manager->add_monitor( &monitor, param );
    return TRUE;
}

static void populate_devmode(int width, int height, int bpp, DEVMODEW *mode)
{
    mode->dmFields = DM_DISPLAYORIENTATION | DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT |
                     DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY;
    mode->dmDisplayOrientation = DMDO_DEFAULT;
    mode->dmDisplayFlags = 0;
    mode->dmBitsPerPel = bpp;
    mode->dmPelsWidth = width;
    mode->dmPelsHeight = height;
    mode->dmDisplayFrequency = 60000 / 1000;
}



BOOL WAYLANDDRV_GetCurrentDisplaySettings(LPCWSTR name, BOOL is_primary, LPDEVMODEW devmode)
{

    devmode->dmFields = DM_DISPLAYORIENTATION | DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT |
                     DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY;
    devmode->dmDisplayOrientation = DMDO_DEFAULT;
    devmode->dmDisplayFlags = 0;
    devmode->dmBitsPerPel = 32;
    devmode->dmPelsWidth = screen_sizes[0].width;
    devmode->dmPelsHeight = screen_sizes[0].height;
    devmode->dmDisplayFrequency = 60000 / 1000;
    devmode->dmFields |= DM_POSITION;
    devmode->dmPosition.x = 0;
    devmode->dmPosition.y = 0;

    return TRUE;
}


BOOL WAYLANDDRV_UpdateDisplayDevices( const struct gdi_device_manager *device_manager,
                                  BOOL force, void *param )
{

  static int done = 0;
  struct gdi_adapter *adapters = NULL;
  struct gdi_gpu *gpus  = NULL;
  INT gpu_count, adapter_count;
  RECT rect = monitor_default_rect(0, 0, 0);

  if (done)
    return TRUE;


  DEVMODEW mode =
  {
      .dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY,
      .dmBitsPerPel = 32, .dmPelsWidth = rect.right, .dmPelsHeight = rect.bottom, .dmDisplayFrequency = 60,
  };

  if (!force && !force_refresh) return TRUE;

  TRACE("via desktop %d %d\n", rect.right, rect.bottom);

  /* Initialize GPUs */
  if (!desktop_get_gpus(&gpus, &gpu_count))
      return TRUE;

  device_manager->add_gpu( &gpus[0], param );

  /* Initialize adapters */
  if (desktop_get_adapters(gpus[0].id, &adapters, &adapter_count)) {

    device_manager->add_adapter( &adapters[0], param );
    desktop_add_monitors(device_manager, param);
    //TODO
    populate_devmode(rect.right, rect.bottom, 32, &mode);
    device_manager->add_mode(&mode, param);
    for (int i=0; i < ARRAY_SIZE(screen_sizes); i++)
    {
      if ( (screen_sizes[i].width != rect.right) || (screen_sizes[i].height != rect.bottom)  ) {
        DEVMODEW mode1 = {0};
        DEVMODEW mode2 = {0};
        populate_devmode(screen_sizes[i].width, screen_sizes[i].height, 32, &mode1);
        device_manager->add_mode(&mode1, param);
        populate_devmode(screen_sizes[i].width, screen_sizes[i].height, 16, &mode2);
        device_manager->add_mode(&mode2, param);
       }
    }
  }

  free(adapters);

  free(gpus);
  done = 1;
  force_refresh = 0;
  return TRUE;
}

static int get_matching_output_mode(LPDEVMODEW devmode)
{
    for (int i=0; i < ARRAY_SIZE(screen_sizes); i++)
    {
        if (devmode->dmPelsWidth == screen_sizes[i].width &&
            devmode->dmPelsHeight == screen_sizes[i].height &&
            32 == devmode->dmBitsPerPel &&
            60000 / 1000 == devmode->dmDisplayFrequency)
        {
            return 1;
        }
    }

    return 0;
}

#define NEXT_DEVMODEW(mode) ((DEVMODEW *)((char *)((mode) + 1) + (mode)->dmDriverExtra))

/***********************************************************************
 *		ChangeDisplaySettings  (WAYLAND.@)
 *
 */
LONG WAYLANDDRV_ChangeDisplaySettings(LPDEVMODEW displays, HWND hwnd, DWORD flags,
                                   LPVOID lpvoid)
{
    DEVMODEW *devmode;

    for (devmode = displays; devmode->dmSize; devmode = NEXT_DEVMODEW(devmode))
    {
        /*
        TRACE("device=%s devmode=%dx%d@%d %dbpp\n",
              wine_dbgstr_w(devmode->dmDeviceName), devmode->dmPelsWidth,
              devmode->dmPelsHeight, devmode->dmDisplayFrequency,
              devmode->dmBitsPerPel);
        */

        if(!get_matching_output_mode(devmode)) {
          return DISP_CHANGE_BADMODE;
        }

    }


    return DISP_CHANGE_SUCCESSFUL;

}

