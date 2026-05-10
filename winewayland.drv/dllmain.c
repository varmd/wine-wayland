/*
 * winewayland.drv entry points
 *
 * Copyright 2022 Jacek Caban for CodeWeavers
 * Copyright 2022 varmd
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

#include <stdarg.h>
#include <stdio.h>
#include "windef.h"
#include "winbase.h"

#include "unixlib.h"

static DWORD WINAPI wayland_read_events_thread(void *arg)
{
    WINE_UNIX_CALL(unix_read_events, NULL);
    /* This thread terminates only if an unrecoverable error occurred
     * during event reading (e.g., the connection to the Wayland
     * compositor is broken). */

    TerminateProcess(GetCurrentProcess(), 1);
    return 0;
}


/***********************************************************************
 *       dll initialisation routine
 */
BOOL WINAPI DllMain( HINSTANCE inst, DWORD reason, LPVOID reserved )
{
    if (reason != DLL_PROCESS_ATTACH)
      return TRUE;

    DWORD tid;
    LPSTR strptr;                 /*ptr to the filename portion of the path */
    CHAR tmpstr[MAX_PATH];


    CHAR exe_path[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, exe_path, ARRAY_SIZE(exe_path));
    GetFullPathNameA(exe_path, MAX_PATH, tmpstr, &strptr);


    if(strcmp(strptr, "rundll32.exe") == 0) {
      printf("Skipping wayland event thread for %s \n", strptr );
      return TRUE;
    }
    if(strcmp(strptr, "wineboot.exe") == 0) {
      printf("Skipping wayland event thread for %s \n", strptr );
      return TRUE;
    }


    printf("Starting wayland attach \n");

    DisableThreadLibraryCalls( inst );
    if (__wine_init_unix_call()) return FALSE;

    if (WINE_UNIX_CALL( unix_init, strptr ))
      return FALSE;


    CloseHandle(CreateThread(NULL, 0, wayland_read_events_thread, NULL, 0, &tid));

    return TRUE;
}
