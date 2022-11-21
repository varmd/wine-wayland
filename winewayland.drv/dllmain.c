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
#include "windef.h"
#include "winbase.h"

#include "unixlib.h"

static unixlib_handle_t unix_handle;

/***********************************************************************
 *       dll initialisation routine
 */
BOOL WINAPI DllMain( HINSTANCE inst, DWORD reason, LPVOID reserved )
{
    struct init_params params;
//    void **callback_table;

    //static WCHAR *current_exe = NULL;
    //static WCHAR current_exepath[MAX_PATH] = {0};
    //GetModuleFileNameW(NULL, current_exepath, ARRAY_SIZE(current_exepath));
//    current_exe = (WCHAR *)get_basename(current_exepath);




    if (reason != DLL_PROCESS_ATTACH)
      return TRUE;

//    TRACE("current exe path %s \n", debugstr_wn(current_exepath, lstrlenW( current_exepath )));

    DisableThreadLibraryCalls( inst );
    if (NtQueryVirtualMemory( GetCurrentProcess(), inst, MemoryWineUnixFuncs,
                              &unix_handle, sizeof(unix_handle), NULL ))
        return FALSE;

    //TRACE("1 current exe path %s \n", debugstr_wn(current_exepath, lstrlenW( current_exepath )));

    if (__wine_unix_call( unix_handle, unix_init, NULL ))
      return FALSE;

    return TRUE;
}
