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



static const struct gdi_dc_funcs waylanddrv_funcs;

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


    if (!(physDev = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*physDev) ))) return NULL;
    return physDev;


}

/**********************************************************************
 *	     WAYLANDDRV_CreateDC
 */
static BOOL CDECL WAYLANDDRV_CreateDC( PHYSDEV *pdev, LPCWSTR device,
                             LPCWSTR output, const DEVMODEW* initData )
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

    push_dc_driver2( pdev, &physDev->dev, &waylanddrv_funcs );

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
    push_dc_driver( pdev, &physDev->dev, &waylanddrv_funcs );
    if (orig) return TRUE;  /* we already went through Xrender if we have an orig device */
    return TRUE;
}


/**********************************************************************
 *	     WAYLANDDRV_DeleteDC
 */
static BOOL CDECL WAYLANDDRV_DeleteDC( PHYSDEV dev )
{
    WAYLANDDRV_PDEVICE *physDev = get_waylanddrv_dev( dev );

    HeapFree( GetProcessHeap(), 0, physDev );
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
static const struct vulkan_funcs * CDECL WAYLANDDRV_wine_get_vulkan_driver( PHYSDEV dev, UINT version )
{
    const struct vulkan_funcs *ret;

    if (!(ret = get_vulkan_driver( version )))
    {
        dev = GET_NEXT_PHYSDEV( dev, wine_get_vulkan_driver );
        ret = dev->funcs->wine_get_vulkan_driver( dev, version );
    }
    return ret;
}

static const struct gdi_dc_funcs waylanddrv_funcs =
{
    NULL,                               /* pAbortDoc */
    NULL,                               /* pAbortPath */
    NULL,                               /* pAlphaBlend */
    NULL,                               /* pAngleArc */
    NULL,                               /* pArc */
    NULL,                               /* pArcTo */
    NULL,                               /* pBeginPath */
    NULL,                               /* pBlendImage */
    NULL,                               /* pChord */
    NULL,                               /* pCloseFigure */
  
    WAYLANDDRV_CreateCompatibleDC,          /* pCreateCompatibleDC */
    WAYLANDDRV_CreateDC,                    /* pCreateDC */
    WAYLANDDRV_DeleteDC,                    /* pDeleteDC */
  
    NULL,                               /* pDeleteObject */

    NULL,                               /* pEllipse */
    NULL,                               /* pEndDoc */
    NULL,                               /* pEndPage */
    NULL,                               /* pEndPath */
    NULL,                               /* pEnumFonts */

    NULL,                               /* pExtEscape */
    NULL,                               /* pExtFloodFill */
    NULL,                               /* pExtTextOut */
    NULL,                               /* pFillPath */
    NULL,                               /* pFillRgn */
    NULL,                               /* pFontIsLinked */
    NULL,                               /* pFrameRgn */
    NULL,                               /* pGetBoundsRect */
    NULL,                               /* pGetCharABCWidths */
    NULL,                               /* pGetCharABCWidthsI */
    NULL,                               /* pGetCharWidth */
    NULL,                               /* pGetCharWidthInfo */
    WAYLANDDRV_GetDeviceCaps,           /* pGetDeviceCaps */
    NULL,                               /* pGetDeviceGammaRamp */
    NULL,                               /* pGetFontData */
    NULL,                               /* pGetFontRealizationInfo */
    NULL,                               /* pGetFontUnicodeRanges */
    NULL,                               /* pGetGlyphIndices */
    NULL,                               /* pGetGlyphOutline */
    NULL,                               /* pGetICMProfile */
    NULL,                               /* pGetImage */
    NULL,                               /* pGetKerningPairs */
    NULL,                               /* pGetNearestColor */
    NULL,                               /* pGetOutlineTextMetrics */
    NULL,                               /* pGetPixel */
    NULL,                               /* pGetSystemPaletteEntries */
    NULL,                               /* pGetTextCharsetInfo */
    NULL,                               /* pGetTextExtentExPoint */
    NULL,                               /* pGetTextExtentExPointI */
    NULL,                               /* pGetTextFace */
    NULL,                               /* pGetTextMetrics */
    NULL,                               /* pGradientFill */
    NULL,                               /* pInvertRgn */
    NULL,                               /* pLineTo */
    NULL,                               /* pMoveTo */
    NULL,                               /* pPaintRgn */
    NULL,                               /* pPatBlt */
    NULL,                               /* pPie */
    NULL,                               /* pPolyBezier */
    NULL,                               /* pPolyBezierTo */
    NULL,                               /* pPolyDraw */
    NULL,                               /* pPolyPolygon */
    NULL,                               /* pPolyPolyline */
    NULL,                               /* pPolylineTo */
    NULL,                               /* pPutImage */
    NULL,                               /* pRealizeDefaultPalette */
    NULL,                               /* pRealizePalette */
    NULL,                               /* pRectangle */
    NULL,                               /* pResetDC */
    NULL,                               /* pRoundRect */
    NULL,                               /* pSelectBitmap */
    NULL,                               /* pSelectBrush */
    NULL,                               /* pSelectFont */
    NULL,                               /* pSelectPen */
    NULL,                               /* pSetBkColor */
    NULL,                               /* pSetBoundsRect */
    NULL,                               /* pSetDCBrushColor */
    NULL,                               /* pSetDCPenColor */
    NULL,                               /* pSetDIBitsToDevice */
    NULL,                               /* pSetDeviceClipping */
    NULL,                               /* pSetDeviceGammaRamp */
    NULL,                               /* pSetPixel */
    NULL,                               /* pSetTextColor */
    NULL,                               /* pStartDoc */
    NULL,                               /* pStartPage */
    NULL,                               /* pStretchBlt */
    NULL,                               /* pStretchDIBits */
    NULL,                               /* pStrokeAndFillPath */
    NULL,                               /* pStrokePath */
    NULL,                               /* pUnrealizePalette */
    
    NULL,                               /* pD3DKMTCheckVidPnExclusiveOwnership */
    NULL,                               /* pD3DKMTSetVidPnSourceOwner */
    
    NULL,                               /* WGL not supported */

    WAYLANDDRV_wine_get_vulkan_driver,   /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV            /* priority */
};


/******************************************************************************
 *      WAYLANDDRV_get_gdi_driver
 */
const struct gdi_dc_funcs * CDECL WAYLANDDRV_get_gdi_driver( unsigned int version )
{
    return &waylanddrv_funcs;
}
