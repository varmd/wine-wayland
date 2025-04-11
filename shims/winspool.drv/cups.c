/*
 * CUPS functions
 *
 * Copyright 2021 Huw Davies
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>



#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "windef.h"
#include "winbase.h"
#include "winternl.h"
#include "winuser.h"
#include "winerror.h"
#include "winreg.h"
#include "wingdi.h"
#include "winspool.h"
#include "ddk/winsplp.h"
#include "wine/debug.h"
#include "wine/unixlib.h"

#include "wspool.h"

WINE_DEFAULT_DEBUG_CHANNEL(winspool);

static NTSTATUS process_attach( void *args )
{
    return STATUS_NOT_SUPPORTED;
}

static NTSTATUS enum_printers( void *args )
{

    return STATUS_NOT_SUPPORTED;

}

static NTSTATUS get_ppd( void *args )
{
    NTSTATUS status = STATUS_SUCCESS;
    return status;
}

static NTSTATUS get_default_page_size( void *args )
{

    return STATUS_NOT_IMPLEMENTED;

}

const unixlib_entry_t __wine_unix_call_funcs[] =
{
    process_attach,
    enum_printers,
    get_default_page_size,
    get_ppd,
};

C_ASSERT( ARRAYSIZE(__wine_unix_call_funcs) == unix_funcs_count );

#ifdef _WIN64

typedef ULONG PTR32;

struct printer_info32
{
    PTR32 name;
    PTR32 comment;
    PTR32 location;
    BOOL  is_default;
};

static NTSTATUS wow64_enum_printers( void *args )
{
    return STATUS_NOT_IMPLEMENTED;
}

static NTSTATUS wow64_get_default_page_size( void *args )
{
    return STATUS_NOT_IMPLEMENTED;
}

static NTSTATUS wow64_get_ppd( void *args )
{
    return STATUS_NOT_IMPLEMENTED;
}

const unixlib_entry_t __wine_unix_call_wow64_funcs[] =
{
    process_attach,
    wow64_enum_printers,
    wow64_get_default_page_size,
    wow64_get_ppd,
};

C_ASSERT( ARRAYSIZE(__wine_unix_call_wow64_funcs) == unix_funcs_count );

#endif  /* _WIN64 */
