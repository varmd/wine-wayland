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

static int palette_size;

static INIT_ONCE init_once = INIT_ONCE_STATIC_INIT;

static const struct gdi_dc_funcs waylanddrv_funcs;

ColorShifts global_palette_default_shifts = { {0,0,0,}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} };


/**********************************************************************
 *	     device_init
 *
 * Perform initializations needed upon creation of the first device.
 */
static BOOL WINAPI device_init( INIT_ONCE *once, void *param, void **context )
{
    
    palette_size = 16777216 ;

    
    return TRUE;
}


static inline void push_dc_driver2( PHYSDEV *dev, PHYSDEV physdev, const struct gdi_dc_funcs *funcs )
{
  
  while ((*dev)->funcs->priority > funcs->priority) dev = &(*dev)->next;
    physdev->funcs = funcs;
    physdev->next = *dev;
    physdev->hdc = (*dev)->hdc;
    *dev = physdev;
}

static WAYLANDDRV_PDEVICE *create_x11_physdev( /* Drawable drawable*/ )
{
    WAYLANDDRV_PDEVICE *physDev;
  
    
    if (!(physDev = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*physDev) ))) return NULL;
    return physDev;
    
  /*

    InitOnceExecuteOnce( &init_once, device_init, NULL, NULL );

    if (!(physDev = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*physDev) ))) return NULL;

    //physDev->drawable = drawable;
  
    return physDev;
  */
}

/**********************************************************************
 *	     WAYLANDDRV_CreateDC
 */
static BOOL CDECL WAYLANDDRV_CreateDC( PHYSDEV *pdev, LPCWSTR driver, LPCWSTR device,
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
 *           WAYLANDDRV_wine_get_wgl_driver
*/ 

static struct opengl_funcs * CDECL WAYLANDDRV_wine_get_wgl_driver( PHYSDEV dev, UINT version )
{
    struct opengl_funcs *ret = NULL;
    
    //re-enable for opengl
    
    if (!(ret = get_wgl_driver( version )))
    {
      dev = GET_NEXT_PHYSDEV( dev, wine_get_wgl_driver );
      ret = dev->funcs->wine_get_wgl_driver( dev, version );
    }
    
    
    return ret;
    
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
    NULL,                         /* pArc */
    NULL,                               /* pArcTo */
    NULL,                               /* pBeginPath */
    NULL,                               /* pBlendImage */
    NULL,                       /* pChord */
    NULL,                               /* pCloseFigure */
    WAYLANDDRV_CreateCompatibleDC,          /* pCreateCompatibleDC */
    WAYLANDDRV_CreateDC,                    /* pCreateDC */
    WAYLANDDRV_DeleteDC,                    /* pDeleteDC */
    NULL,                               /* pDeleteObject */
    NULL,                               /* pDeviceCapabilities */
    NULL,                     /* pEllipse */
    NULL,                               /* pEndDoc */
    NULL,                               /* pEndPage */
    NULL,                               /* pEndPath */
    NULL,                               /* pEnumFonts */
    NULL,             /* pEnumICMProfiles */
    NULL,                               /* pExcludeClipRect */
    NULL,                               /* pExtDeviceMode */
    //WAYLANDDRV_ExtEscape,                   /* pExtEscape */
    NULL,                   /* pExtEscape */
    //WAYLANDDRV_ExtFloodFill,                /* pExtFloodFill */
    NULL,                /* pExtFloodFill */
    NULL,                               /* pExtSelectClipRgn */
    NULL,                               /* pExtTextOut */
    //WAYLANDDRV_FillPath,                    /* pFillPath */
    NULL,                    /* pFillPath */
    NULL,                               /* pFillRgn */
    NULL,                               /* pFlattenPath */
    NULL,                               /* pFontIsLinked */
    NULL,                               /* pFrameRgn */
    NULL,                               /* pGdiComment */
    NULL,                               /* pGetBoundsRect */
    NULL,                               /* pGetCharABCWidths */
    NULL,                               /* pGetCharABCWidthsI */
    NULL,                               /* pGetCharWidth */
    NULL,                               /* pGetCharWidthInfo */
    WAYLANDDRV_GetDeviceCaps,               /* pGetDeviceCaps */
    //WAYLANDDRV_GetDeviceGammaRamp,          /* pGetDeviceGammaRamp */
    NULL,          /* pGetDeviceGammaRamp */
    NULL,                               /* pGetFontData */
    NULL,                               /* pGetFontRealizationInfo */
    NULL,                               /* pGetFontUnicodeRanges */
    NULL,                               /* pGetGlyphIndices */
    NULL,                               /* pGetGlyphOutline */
    NULL,               /* pGetICMProfile */
    NULL,                    /* pGetImage */
    NULL,                               /* pGetKerningPairs */
    NULL,             /* pGetNearestColor */
    NULL,                               /* pGetOutlineTextMetrics */
    NULL,                               /* pGetPixel */
    NULL,     /* pGetSystemPaletteEntries */
    NULL,                               /* pGetTextCharsetInfo */
    NULL,                               /* pGetTextExtentExPoint */
    NULL,                               /* pGetTextExtentExPointI */
    NULL,                               /* pGetTextFace */
    NULL,                               /* pGetTextMetrics */
    //WAYLANDDRV_GradientFill,                /* pGradientFill */
    NULL,                /* pGradientFill */
    NULL,                               /* pIntersectClipRect */
    NULL,                               /* pInvertRgn */
    //WAYLANDDRV_LineTo,                      /* pLineTo */
    NULL,                      /* pLineTo */
    NULL,                               /* pModifyWorldTransform */
    NULL,                               /* pMoveTo */
    NULL,                               /* pOffsetClipRgn */
    NULL,                               /* pOffsetViewportOrg */
    NULL,                               /* pOffsetWindowOrg */
    NULL,                    /* pPaintRgn */
    NULL,                      /* pPatBlt */
    NULL,                         /* pPie */
    NULL,                               /* pPolyBezier */
    NULL,                               /* pPolyBezierTo */
    NULL,                               /* pPolyDraw */
    NULL,                 /* pPolyPolygon */
    NULL,                /* pPolyPolyline */
    //WAYLANDDRV_Polygon,                     /* pPolygon */
    NULL,                     /* pPolygon */
    NULL,                               /* pPolyline */
    NULL,                               /* pPolylineTo */
    NULL,                    /* pPutImage */
    NULL,       /* pRealizeDefaultPalette */
    NULL,              /* pRealizePalette */
    NULL,                   /* pRectangle */
    NULL,                               /* pResetDC */
    NULL,                               /* pRestoreDC */
    NULL,                   /* pRoundRect */
    NULL,                               /* pSaveDC */
    NULL,                               /* pScaleViewportExt */
    NULL,                               /* pScaleWindowExt */
    NULL,                               /* pSelectBitmap */
    NULL,                 /* pSelectBrush */
    NULL,                               /* pSelectClipPath */
    NULL,                  /* pSelectFont */  //NULLED
    NULL,                               /* pSelectPalette */
    NULL,                   /* pSelectPen */
    NULL,                               /* pSetArcDirection */
    NULL,                               /* pSetBkColor */
    NULL,                               /* pSetBkMode */
    //WAYLANDDRV_SetBoundsRect,               /* pSetBoundsRect */
    NULL,               /* pSetBoundsRect */
    //WAYLANDDRV_SetDCBrushColor,             /* pSetDCBrushColor */
    NULL,             /* pSetDCBrushColor */
    //WAYLANDDRV_SetDCPenColor,               /* pSetDCPenColor */
    NULL,               /* pSetDCPenColor */
    //NULL,               /* pSetDCPenColor */
    NULL,                               /* pSetDIBitsToDevice */
    //WAYLANDDRV_SetDeviceClipping,           /* pSetDeviceClipping */
    NULL,           /* pSetDeviceClipping */
    //WAYLANDDRV_SetDeviceGammaRamp,          /* pSetDeviceGammaRamp */
    NULL,          /* pSetDeviceGammaRamp */
    NULL,                               /* pSetLayout */
    NULL,                               /* pSetMapMode */
    NULL,                               /* pSetMapperFlags */
    NULL,                    /* pSetPixel */
    NULL,                               /* pSetPolyFillMode */
    NULL,                               /* pSetROP2 */
    NULL,                               /* pSetRelAbs */
    NULL,                               /* pSetStretchBltMode */
    NULL,                               /* pSetTextAlign */
    NULL,                               /* pSetTextCharacterExtra */
    NULL,                               /* pSetTextColor */
    NULL,                               /* pSetTextJustification */
    NULL,                               /* pSetViewportExt */
    NULL,                               /* pSetViewportOrg */
    NULL,                               /* pSetWindowExt */
    NULL,                               /* pSetWindowOrg */
    NULL,                               /* pSetWorldTransform */
    NULL,                               /* pStartDoc */
    NULL,                               /* pStartPage */
    NULL,                  /* pStretchBlt */ //WAYLANDDRV_StretchBlt, 
    NULL,                               /* pStretchDIBits */
    NULL,           /* pStrokeAndFillPath */ //WAYLANDDRV_StrokeAndFillPath
    NULL,                  /* pStrokePath */   
    NULL,            /* pUnrealizePalette */
    NULL,                               /* pWidenPath */
    NULL,                                // D3D ??
    NULL,                                // D3D ??
    NULL,         /* WGL Not Supported */
    //WAYLANDDRV_wine_get_wgl_driver,         /* wine_get_wgl_driver */
    WAYLANDDRV_wine_get_vulkan_driver,      /* wine_get_vulkan_driver */
    GDI_PRIORITY_GRAPHICS_DRV           /* priority */
};




/******************************************************************************
 *      WAYLANDDRV_get_gdi_driver
 */
const struct gdi_dc_funcs * CDECL WAYLANDDRV_get_gdi_driver( unsigned int version )
{
    /*
    if (version != WINE_GDI_DRIVER_VERSION)
    {
        ERR( "version mismatch, gdi32 wants %u but winex11 has %u\n", version, WINE_GDI_DRIVER_VERSION );
        return NULL;
    }*/
    return &waylanddrv_funcs;
}
