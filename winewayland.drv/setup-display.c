/*
 * WAYLAND display device functions
 *
 * Copyright 2019 Zhiyi Zhang for CodeWeavers
 * Copyright 2020 Alexandros Frantzis for Collabora Ltd
 * Copyright 2021 varmd (github.com/varmd)
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

#define NONAMELESSUNION
#define NONAMELESSSTRUCT

#include "config.h"

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "rpc.h"
#include "winreg.h"
#include "initguid.h"
#include "devguid.h"
#include "devpkey.h"

#include "setupapi.h"
#define WIN32_NO_STATUS
#include "winternl.h"
#include "wine/debug.h"
#include "wine/unicode.h"
#include "waylanddrv.h"

WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);

DEFINE_DEVPROPKEY(DEVPROPKEY_GPU_LUID, 0x60b193cb, 0x5276, 0x4d0f, 0x96, 0xfc, 0xf1, 0x73, 0xab, 0xad, 0x3e, 0xc6, 2);
DEFINE_DEVPROPKEY(DEVPROPKEY_MONITOR_GPU_LUID, 0xca085853, 0x16ce, 0x48aa, 0xb1, 0x14, 0xde, 0x9c, 0x72, 0x33, 0x42, 0x23, 1);
DEFINE_DEVPROPKEY(DEVPROPKEY_MONITOR_OUTPUT_ID, 0xca085853, 0x16ce, 0x48aa, 0xb1, 0x14, 0xde, 0x9c, 0x72, 0x33, 0x42, 0x23, 2);

/* Wine specific properties */
DEFINE_DEVPROPKEY(WINE_DEVPROPKEY_GPU_VULKAN_UUID, 0x233a9ef3, 0xafc4, 0x4abd, 0xb5, 0x64, 0xc3, 0x2f, 0x21, 0xf1, 0x53, 0x5c, 2);
DEFINE_DEVPROPKEY(WINE_DEVPROPKEY_MONITOR_STATEFLAGS, 0x233a9ef3, 0xafc4, 0x4abd, 0xb5, 0x64, 0xc3, 0x2f, 0x21, 0xf1, 0x53, 0x5b, 2);
DEFINE_DEVPROPKEY(WINE_DEVPROPKEY_MONITOR_RCMONITOR, 0x233a9ef3, 0xafc4, 0x4abd, 0xb5, 0x64, 0xc3, 0x2f, 0x21, 0xf1, 0x53, 0x5b, 3);
DEFINE_DEVPROPKEY(WINE_DEVPROPKEY_MONITOR_RCWORK, 0x233a9ef3, 0xafc4, 0x4abd, 0xb5, 0x64, 0xc3, 0x2f, 0x21, 0xf1, 0x53, 0x5b, 4);
DEFINE_DEVPROPKEY(WINE_DEVPROPKEY_MONITOR_ADAPTERNAME, 0x233a9ef3, 0xafc4, 0x4abd, 0xb5, 0x64, 0xc3, 0x2f, 0x21, 0xf1, 0x53, 0x5b, 5);

static const WCHAR driver_date_dataW[] = {'D','r','i','v','e','r','D','a','t','e','D','a','t','a',0};
static const WCHAR driver_descW[] = {'D','r','i','v','e','r','D','e','s','c',0};
static const WCHAR displayW[] = {'D','I','S','P','L','A','Y',0};
static const WCHAR pciW[] = {'P','C','I',0};
static const WCHAR video_idW[] = {'V','i','d','e','o','I','D',0};
static const WCHAR symbolic_link_valueW[]= {'S','y','m','b','o','l','i','c','L','i','n','k','V','a','l','u','e',0};
static const WCHAR gpu_idW[] = {'G','P','U','I','D',0};
static const WCHAR mointor_id_fmtW[] = {'M','o','n','i','t','o','r','I','D','%','d',0};
static const WCHAR adapter_name_fmtW[] = {'\\','\\','.','\\','D','I','S','P','L','A','Y','%','d',0};
static const WCHAR state_flagsW[] = {'S','t','a','t','e','F','l','a','g','s',0};
static const WCHAR guid_fmtW[] = {
    '{','%','0','8','x','-','%','0','4','x','-','%','0','4','x','-','%','0','2','x','%','0','2','x','-',
    '%','0','2','x','%','0','2','x','%','0','2','x','%','0','2','x','%','0','2','x','%','0','2','x','}',0};
static const WCHAR gpu_instance_fmtW[] = {
    'P','C','I','\\',
    'V','E','N','_','%','0','4','X','&',
    'D','E','V','_','%','0','4','X','&',
    'S','U','B','S','Y','S','_','%','0','8','X','&',
    'R','E','V','_','%','0','2','X','\\',
    '%','0','8','X',0};
static const WCHAR gpu_hardware_id_fmtW[] = {
    'P','C','I','\\',
    'V','E','N','_','%','0','4','X','&',
    'D','E','V','_','%','0','4','X','&',
    'S','U','B','S','Y','S','_','0','0','0','0','0','0','0','0','&',
    'R','E','V','_','0','0',0};
static const WCHAR video_keyW[] = {
    'H','A','R','D','W','A','R','E','\\',
    'D','E','V','I','C','E','M','A','P','\\',
    'V','I','D','E','O',0};
static const WCHAR adapter_key_fmtW[] = {
    'S','y','s','t','e','m','\\',
    'C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\',
    'C','o','n','t','r','o','l','\\',
    'V','i','d','e','o','\\',
    '%','s','\\',
    '%','0','4','x',0};
static const WCHAR device_video_fmtW[] = {
    '\\','D','e','v','i','c','e','\\',
    'V','i','d','e','o','%','d',0};
static const WCHAR machine_prefixW[] = {
    '\\','R','e','g','i','s','t','r','y','\\',
    'M','a','c','h','i','n','e','\\',0};
static const WCHAR nt_classW[] = {
    '\\','R','e','g','i','s','t','r','y','\\',
    'M','a','c','h','i','n','e','\\',
    'S','y','s','t','e','m','\\',
    'C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\',
    'C','o','n','t','r','o','l','\\',
    'C','l','a','s','s','\\',0};
static const WCHAR monitor_instance_fmtW[] = {
    'D','I','S','P','L','A','Y','\\',
    'D','e','f','a','u','l','t','_','M','o','n','i','t','o','r','\\',
    '%','0','4','X','&','%','0','4','X',0};
static const WCHAR monitor_hardware_idW[] = {
    'M','O','N','I','T','O','R','\\',
    'D','e','f','a','u','l','t','_','M','o','n','i','t','o','r',0,0};

/* Represent a physical GPU in the PCI slots */
struct waylanddrv_gpu
{
    /* ID to uniquely identify a GPU in handler */
    ULONG_PTR id;
    /* Name */
    WCHAR name[128];
    /* PCI ID */
    UINT vendor_id;
    UINT device_id;
    UINT subsys_id;
    UINT revision_id;
    /* Vulkan device UUID */
    GUID vulkan_uuid;
};

HANDLE acquire_display_devices_init_mutex(void)
{
    static const WCHAR init_mutexW[] = {'d','i','s','p','l','a','y','_','d','e','v','i','c','e','_','i','n','i','t',0};
    HANDLE mutex = CreateMutexW(NULL, FALSE, init_mutexW);

    WaitForSingleObject(mutex, INFINITE);
    return mutex;
}

void release_display_devices_init_mutex(HANDLE mutex)
{
    ReleaseMutex(mutex);
    CloseHandle(mutex);
}

/* Initialize a GPU instance.
 * Return its GUID string in guid_string, driver value in driver parameter and LUID in gpu_luid */
static BOOL wayland_init_gpu(HDEVINFO devinfo, const struct waylanddrv_gpu *gpu, INT gpu_index, WCHAR *guid_string,
                             WCHAR *driver, LUID *gpu_luid)
{
    static const BOOL present = TRUE;
    SP_DEVINFO_DATA device_data = {sizeof(device_data)};
    WCHAR instanceW[MAX_PATH];
    DEVPROPTYPE property_type;
    WCHAR bufferW[2048];
    HKEY hkey = NULL;
    GUID guid;
    LUID luid;
    INT written;
    DWORD size;
    BOOL ret = FALSE;
    FILETIME filetime;

    TRACE("GPU id:0x%s name:%s.\n", wine_dbgstr_longlong(gpu->id), wine_dbgstr_w(gpu->name));

    sprintfW(instanceW, gpu_instance_fmtW, gpu->vendor_id, gpu->device_id, gpu->subsys_id, gpu->revision_id, gpu_index);
    if (!SetupDiOpenDeviceInfoW(devinfo, instanceW, NULL, 0, &device_data))
    {
        SetupDiCreateDeviceInfoW(devinfo, instanceW, &GUID_DEVCLASS_DISPLAY, gpu->name, NULL, 0, &device_data);
        if (!SetupDiRegisterDeviceInfo(devinfo, &device_data, 0, NULL, NULL, NULL))
            goto done;
    }

    /* Write HardwareID registry property, REG_MULTI_SZ */
    written = sprintfW(bufferW, gpu_hardware_id_fmtW, gpu->vendor_id, gpu->device_id);
    bufferW[written + 1] = 0;
    if (!SetupDiSetDeviceRegistryPropertyW(devinfo, &device_data, SPDRP_HARDWAREID, (const BYTE *)bufferW,
                                           (written + 2) * sizeof(WCHAR)))
        goto done;

    /* Write DEVPKEY_Device_IsPresent property */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &DEVPKEY_Device_IsPresent, DEVPROP_TYPE_BOOLEAN,
                                   (const BYTE *)&present, sizeof(present), 0))
        goto done;

    /* Write DEVPROPKEY_GPU_LUID property */
    if (!SetupDiGetDevicePropertyW(devinfo, &device_data, &DEVPROPKEY_GPU_LUID, &property_type,
                                   (BYTE *)&luid, sizeof(luid), NULL, 0))
    {
        if (!AllocateLocallyUniqueId(&luid))
            goto done;

        if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &DEVPROPKEY_GPU_LUID,
                                       DEVPROP_TYPE_UINT64, (const BYTE *)&luid, sizeof(luid), 0))
            goto done;
    }
    *gpu_luid = luid;
    TRACE("LUID:%08x:%08x.\n", luid.HighPart, luid.LowPart);

    /* Write WINE_DEVPROPKEY_GPU_VULKAN_UUID property */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &WINE_DEVPROPKEY_GPU_VULKAN_UUID,
                                   DEVPROP_TYPE_GUID, (const BYTE *)&gpu->vulkan_uuid,
                                   sizeof(gpu->vulkan_uuid), 0))
        goto done;
    TRACE("Vulkan UUID:%s.\n", wine_dbgstr_guid(&gpu->vulkan_uuid));

    /* Open driver key.
     * This is where HKLM\System\CurrentControlSet\Control\Video\{GPU GUID}\{Adapter Index} links to */
    hkey = SetupDiCreateDevRegKeyW(devinfo, &device_data, DICS_FLAG_GLOBAL, 0, DIREG_DRV, NULL, NULL);

    /* Write DriverDesc value */
    if (RegSetValueExW(hkey, driver_descW, 0, REG_SZ, (const BYTE *)gpu->name,
                       (strlenW(gpu->name) + 1) * sizeof(WCHAR)))
        goto done;
    /* Write DriverDateData value, using current time as driver date, needed by Evoland */
    GetSystemTimeAsFileTime(&filetime);
    if (RegSetValueExW(hkey, driver_date_dataW, 0, REG_BINARY, (BYTE *)&filetime, sizeof(filetime)))
        goto done;

    RegCloseKey(hkey);

    /* Retrieve driver value for adapters */
    if (!SetupDiGetDeviceRegistryPropertyW(devinfo, &device_data, SPDRP_DRIVER, NULL, (BYTE *)bufferW, sizeof(bufferW),
                                           NULL))
        goto done;
    lstrcpyW(driver, nt_classW);
    lstrcatW(driver, bufferW);

    /* Write GUID in VideoID in .../instance/Device Parameters, reuse the GUID if it's existent */
    hkey = SetupDiCreateDevRegKeyW(devinfo, &device_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, NULL, NULL);

    size = sizeof(bufferW);
    if (RegQueryValueExW(hkey, video_idW, 0, NULL, (BYTE *)bufferW, &size))
    {
        UuidCreate(&guid);
        sprintfW(bufferW, guid_fmtW, guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2],
                 guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
        if (RegSetValueExW(hkey, video_idW, 0, REG_SZ, (const BYTE *)bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR)))
            goto done;
    }
    lstrcpyW(guid_string, bufferW);
    
    TRACE("GPU 1");

    ret = TRUE;
    
    
done:
    RegCloseKey(hkey);
    
    if (!ret)
        ERR("Failed to initialize GPU\n");
    return ret;
}

static BOOL wayland_init_adapter(HKEY video_hkey, INT video_index, INT gpu_index,
                                 INT adapter_index, INT monitor_count,
                                   const struct waylanddrv_gpu *gpu, const WCHAR *guid_string,
                                   const WCHAR *gpu_driver)
{
    WCHAR adapter_keyW[MAX_PATH];
    WCHAR key_nameW[MAX_PATH];
    WCHAR bufferW[1024];
    HKEY hkey = NULL;
    BOOL ret = FALSE;
    LSTATUS ls;
    INT i;
    DWORD state_flags = adapter_index == 0 ? DISPLAY_DEVICE_PRIMARY_DEVICE | DISPLAY_DEVICE_ATTACHED_TO_DESKTOP: 0;

    sprintfW(key_nameW, device_video_fmtW, video_index);
    lstrcpyW(bufferW, machine_prefixW);
    sprintfW(adapter_keyW, adapter_key_fmtW, guid_string, adapter_index);
    lstrcatW(bufferW, adapter_keyW);

    /* Write value of \Device\Video? (adapter key) in HKLM\HARDWARE\DEVICEMAP\VIDEO\ */
    if (RegSetValueExW(video_hkey, key_nameW, 0, REG_SZ, (const BYTE *)bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR)))
        goto done;

    /* Create HKLM\System\CurrentControlSet\Control\Video\{GPU GUID}\{Adapter Index} link to GPU driver */
    ls = RegCreateKeyExW(HKEY_LOCAL_MACHINE, adapter_keyW, 0, NULL, REG_OPTION_VOLATILE | REG_OPTION_CREATE_LINK,
                         KEY_ALL_ACCESS, NULL, &hkey, NULL);
    if (ls == ERROR_ALREADY_EXISTS)
        RegCreateKeyExW(HKEY_LOCAL_MACHINE, adapter_keyW, 0, NULL, REG_OPTION_VOLATILE | REG_OPTION_OPEN_LINK,
                        KEY_ALL_ACCESS, NULL, &hkey, NULL);
    if (RegSetValueExW(hkey, symbolic_link_valueW, 0, REG_LINK, (const BYTE *)gpu_driver,
                       strlenW(gpu_driver) * sizeof(WCHAR)))
        goto done;
    RegCloseKey(hkey);
    hkey = NULL;

    /* FIXME:
     * Following information is Wine specific, it doesn't really exist on Windows. It is used so that we can
     * implement EnumDisplayDevices etc by querying registry only. This information is most likely reported by the
     * device driver on Windows */
    RegCreateKeyExW(HKEY_CURRENT_CONFIG, adapter_keyW, 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &hkey, NULL);

    /* Write GPU instance path so that we can find the GPU instance via adapters quickly. Another way is trying to match
     * them via the GUID in Device Parameters/VideoID, but it would require enumerating all GPU instances */
    sprintfW(bufferW, gpu_instance_fmtW, gpu->vendor_id, gpu->device_id, gpu->subsys_id, gpu->revision_id, gpu_index);
    if (RegSetValueExW(hkey, gpu_idW, 0, REG_SZ, (const BYTE *)bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR)))
        goto done;

    /* Write all monitor instances paths under this adapter */
    for (i = 0; i < monitor_count; i++)
    {
        sprintfW(key_nameW, mointor_id_fmtW, i);
        sprintfW(bufferW, monitor_instance_fmtW, video_index, i);
        if (RegSetValueExW(hkey, key_nameW, 0, REG_SZ, (const BYTE *)bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR)))
            goto done;
    }

    /* Write StateFlags */
    if (RegSetValueExW(hkey, state_flagsW, 0, REG_DWORD, (const BYTE *)&state_flags,
                        sizeof(state_flags)))
        goto done;

    TRACE("Adapter 1");
    
    ret = TRUE;
done:
    RegCloseKey(hkey);
    if (!ret)
        ERR("Failed to initialize adapter\n");
    return ret;
}

static BOOL wayland_init_monitor(HDEVINFO devinfo, int monitor_index, int video_index, const LUID *gpu_luid, UINT output_id)
{
    SP_DEVINFO_DATA device_data = {sizeof(SP_DEVINFO_DATA)};
    WCHAR bufferW[MAX_PATH];
    HKEY hkey;
    BOOL ret = FALSE;
    DWORD state_flags = DISPLAY_DEVICE_ATTACHED | DISPLAY_DEVICE_ACTIVE;
    RECT rc_mode;

    
    SetRect(&rc_mode, 0, 0, 1920, 1080);
    
    static const WCHAR wayland_monW[] = {'W','a','y','l','a','n','d','M','o','n',
    'i','t', 'o', 'r',
    0};

    /* Create GUID_DEVCLASS_MONITOR instance */
    sprintfW(bufferW, monitor_instance_fmtW, video_index, monitor_index);
    SetupDiCreateDeviceInfoW(devinfo, bufferW, &GUID_DEVCLASS_MONITOR, wayland_monW, NULL, 0, &device_data);
    if (!SetupDiRegisterDeviceInfo(devinfo, &device_data, 0, NULL, NULL, NULL))
        goto done;
    
    
    
    
    

    /* Write HardwareID registry property */
    if (!SetupDiSetDeviceRegistryPropertyW(devinfo, &device_data, SPDRP_HARDWAREID,
                                           (const BYTE *)monitor_hardware_idW, sizeof(monitor_hardware_idW)))
        goto done;
    
    
    TRACE("Monitor 2 \n");
    
    

    /* Write DEVPROPKEY_MONITOR_GPU_LUID */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &DEVPROPKEY_MONITOR_GPU_LUID,
                                   DEVPROP_TYPE_INT64, (const BYTE *)gpu_luid, sizeof(*gpu_luid), 0))
        goto done;
    
    
    TRACE("Monitor 2- \n");

    /* Write DEVPROPKEY_MONITOR_OUTPUT_ID */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &DEVPROPKEY_MONITOR_OUTPUT_ID,
                                   DEVPROP_TYPE_UINT32, (const BYTE *)&output_id, sizeof(output_id), 0))
        goto done;
    
    TRACE("Monitor 2-- \n");

    /* Create driver key */
    hkey = SetupDiCreateDevRegKeyW(devinfo, &device_data, DICS_FLAG_GLOBAL, 0, DIREG_DRV, NULL, NULL);
    RegCloseKey(hkey);

    /* FIXME:
     * Following properties are Wine specific, see comments in wayland_init_adapter for details */
    /* StateFlags */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &WINE_DEVPROPKEY_MONITOR_STATEFLAGS, DEVPROP_TYPE_UINT32,
                                   (const BYTE *)&state_flags, sizeof(state_flags), 0))
       goto done;
    /* RcMonitor */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &WINE_DEVPROPKEY_MONITOR_RCMONITOR, DEVPROP_TYPE_BINARY,
                                   (const BYTE *)&rc_mode, sizeof(rc_mode), 0))
        goto done;
    /* RcWork */
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &WINE_DEVPROPKEY_MONITOR_RCWORK, DEVPROP_TYPE_BINARY,
                                   (const BYTE *)&rc_mode, sizeof(rc_mode), 0))
        goto done;
    
    /* Adapter name */
    sprintfW(bufferW, adapter_name_fmtW, video_index + 1);
    if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &WINE_DEVPROPKEY_MONITOR_ADAPTERNAME, DEVPROP_TYPE_STRING,
                                   (const BYTE *)bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR), 0)) {
        TRACE("adapter name failed \n");
        goto done;
    }
    
    TRACE("Monitor 1");

    ret = TRUE;
done:
    
    if (!ret)
        ERR("Failed to initialize monitor\n");
    return ret;
}

static void prepare_devices(HKEY video_hkey)
{
    static const BOOL not_present = FALSE;
    SP_DEVINFO_DATA device_data = {sizeof(device_data)};
    HDEVINFO devinfo;
    DWORD i = 0;

    /* Remove all monitors */
    devinfo = SetupDiGetClassDevsW(&GUID_DEVCLASS_MONITOR, displayW, NULL, 0);
    while (SetupDiEnumDeviceInfo(devinfo, i++, &device_data))
    {
        if (!SetupDiRemoveDevice(devinfo, &device_data))
            ERR("Failed to remove monitor\n");
    }
    SetupDiDestroyDeviceInfoList(devinfo);

    /* Clean up old adapter keys for reinitialization */
    RegDeleteTreeW(video_hkey, NULL);

    /* FIXME:
     * Currently SetupDiGetClassDevsW with DIGCF_PRESENT is unsupported, So we need to clean up not present devices in
     * case application uses SetupDiGetClassDevsW to enumerate devices. Wrong devices could exist in registry as a result
     * of prefix copying or having devices unplugged. But then we couldn't simply delete GPUs because we need to retain
     * the same GUID for the same GPU. */
    i = 0;
    devinfo = SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY, pciW, NULL, 0);
    while (SetupDiEnumDeviceInfo(devinfo, i++, &device_data))
    {
        if (!SetupDiSetDevicePropertyW(devinfo, &device_data, &DEVPKEY_Device_IsPresent, DEVPROP_TYPE_BOOLEAN,
                                       (const BYTE *)&not_present, sizeof(not_present), 0))
            ERR("Failed to set GPU present property\n");
    }
    SetupDiDestroyDeviceInfoList(devinfo);
}

static void cleanup_devices(void)
{
    SP_DEVINFO_DATA device_data = {sizeof(device_data)};
    HDEVINFO devinfo;
    DWORD type;
    DWORD i = 0;
    BOOL present;

    devinfo = SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY, pciW, NULL, 0);
    while (SetupDiEnumDeviceInfo(devinfo, i++, &device_data))
    {
        present = FALSE;
        SetupDiGetDevicePropertyW(devinfo, &device_data, &DEVPKEY_Device_IsPresent, &type, (BYTE *)&present,
                                  sizeof(present), NULL, 0);
        if (!present && !SetupDiRemoveDevice(devinfo, &device_data))
            ERR("Failed to remove GPU\n");
    }
    SetupDiDestroyDeviceInfoList(devinfo);
}

void wayland_init_display_devices(BOOL force)
{
    HANDLE mutex;
    HDEVINFO gpu_devinfo = NULL, monitor_devinfo = NULL;
    HKEY video_hkey = NULL;
    INT gpu_index = 0;
    INT output_index = 0;
    DWORD disposition = 0;
    WCHAR gpu_guidW[40];
    WCHAR driverW[1024];
    LUID gpu_luid;
    UINT output_id = 0;
    
    struct waylanddrv_gpu gpu = { 0 };
    static const WCHAR wayland_gpuW[] = {'W','a','y','l','a','n','d','G','P','U',0};
    lstrcpyW(gpu.name, wayland_gpuW);

    mutex = acquire_display_devices_init_mutex();

    if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, video_keyW, 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &video_hkey,
                        &disposition))
    {
        ERR("Failed to create video device key\n");
        goto done;
    }

    /* Avoid unnecessary reinit */
    if (disposition != REG_CREATED_NEW_KEY) {
      TRACE("Display registry already created \n");
      RegCloseKey(video_keyW);
      release_display_devices_init_mutex(mutex);
      return;
    }   

    prepare_devices(video_hkey);

    gpu_devinfo = SetupDiCreateDeviceInfoList(&GUID_DEVCLASS_DISPLAY, NULL);
    monitor_devinfo = SetupDiCreateDeviceInfoList(&GUID_DEVCLASS_MONITOR, NULL);

    /* TODO: Support multiple GPUs. Note that wayland doesn't currently expose GPU info. */
    if (!wayland_init_gpu(gpu_devinfo, &gpu, gpu_index, gpu_guidW, driverW, &gpu_luid)) {
        goto done;
    }  

    if (!wayland_init_adapter(video_hkey, output_index, gpu_index, output_index, 1, &gpu, gpu_guidW, driverW)) {
        
        goto done;
    }  

    if (!wayland_init_monitor(monitor_devinfo, output_index, output_index,
      &gpu_luid, output_id++)) {
        TRACE("Monitor init failed \n");
        //goto done;
    }
        

done:
    cleanup_devices();
    SetupDiDestroyDeviceInfoList(monitor_devinfo);
    SetupDiDestroyDeviceInfoList(gpu_devinfo);
    RegCloseKey(video_hkey);
    release_display_devices_init_mutex(mutex);
}

static BOOL get_display_device_reg_key(const WCHAR *device_name, WCHAR *key, unsigned len)
{
    static const WCHAR display[] = {'\\','\\','.','\\','D','I','S','P','L','A','Y'};
    static const WCHAR video_value_fmt[] = {'\\','D','e','v','i','c','e','\\',
                                            'V','i','d','e','o','%','d',0};
    static const WCHAR video_key[] = {'H','A','R','D','W','A','R','E','\\',
                                      'D','E','V','I','C','E','M','A','P','\\',
                                      'V','I','D','E','O','\\',0};
    WCHAR value_name[MAX_PATH], buffer[MAX_PATH], *end_ptr;
    DWORD adapter_index, size;

    /* Device name has to be \\.\DISPLAY%d */
    if (strncmpiW(device_name, display, ARRAY_SIZE(display)))
        return FALSE;

    /* Parse \\.\DISPLAY* */
    adapter_index = strtolW(device_name + ARRAY_SIZE(display), &end_ptr, 10) - 1;
    if (*end_ptr)
        return FALSE;

    /* Open \Device\Video* in HKLM\HARDWARE\DEVICEMAP\VIDEO\ */
    sprintfW(value_name, video_value_fmt, adapter_index);
    size = sizeof(buffer);
    if (RegGetValueW(HKEY_LOCAL_MACHINE, video_key, value_name, RRF_RT_REG_SZ, NULL, buffer, &size))
        return FALSE;

    if (len < lstrlenW(buffer + 18) + 1)
        return FALSE;

    /* Skip \Registry\Machine\ prefix */
    lstrcpyW(key, buffer + 18);
    TRACE("display device %s registry settings key %s.\n", wine_dbgstr_w(device_name), wine_dbgstr_w(key));
    return TRUE;
}

#if 0

static BOOL read_registry_settings(const WCHAR *device_name, DEVMODEW *dm)
{
    WCHAR display_device_reg_key[MAX_PATH];
    HANDLE mutex;
    HKEY hkey;
    DWORD type, size;
    BOOL ret = TRUE;

    dm->dmFields = 0;

    mutex = acquire_display_devices_init_mutex();
    if (!get_display_device_reg_key(device_name, display_device_reg_key,
                                    ARRAY_SIZE(display_device_reg_key)))
    {
        ret = FALSE;
        goto done;
    }

    if (RegOpenKeyExW(HKEY_CURRENT_CONFIG, display_device_reg_key, 0, KEY_READ, &hkey))
    {
        ret = FALSE;
        goto done;
    }

#define query_value(name, data) \
    size = sizeof(DWORD); \
    if (RegQueryValueExA(hkey, name, 0, &type, (LPBYTE)(data), &size) || \
        type != REG_DWORD || size != sizeof(DWORD)) \
        ret = FALSE

    query_value("DefaultSettings.BitsPerPel", &dm->dmBitsPerPel);
    dm->dmFields |= DM_BITSPERPEL;
    query_value("DefaultSettings.XResolution", &dm->dmPelsWidth);
    dm->dmFields |= DM_PELSWIDTH;
    query_value("DefaultSettings.YResolution", &dm->dmPelsHeight);
    dm->dmFields |= DM_PELSHEIGHT;
    query_value("DefaultSettings.VRefresh", &dm->dmDisplayFrequency);
    dm->dmFields |= DM_DISPLAYFREQUENCY;
    query_value("DefaultSettings.Flags", &dm->u2.dmDisplayFlags);
    dm->dmFields |= DM_DISPLAYFLAGS;
    query_value("DefaultSettings.XPanning", &dm->u1.s2.dmPosition.x);
    query_value("DefaultSettings.YPanning", &dm->u1.s2.dmPosition.y);
    dm->dmFields |= DM_POSITION;
    query_value("DefaultSettings.Orientation", &dm->u1.s2.dmDisplayOrientation);
    dm->dmFields |= DM_DISPLAYORIENTATION;
    query_value("DefaultSettings.FixedOutput", &dm->u1.s2.dmDisplayFixedOutput);

#undef query_value

    RegCloseKey(hkey);

done:
    release_display_devices_init_mutex(mutex);
    return ret;
}

static BOOL write_registry_settings(const WCHAR *device_name, const DEVMODEW *dm)
{
    WCHAR display_device_reg_key[MAX_PATH];
    HANDLE mutex;
    HKEY hkey;
    BOOL ret = TRUE;

    mutex = acquire_display_devices_init_mutex();
    if (!get_display_device_reg_key(device_name, display_device_reg_key,
                                    ARRAY_SIZE(display_device_reg_key)))
    {
        ret = FALSE;
        goto done;
    }

    if (RegCreateKeyExW(HKEY_CURRENT_CONFIG, display_device_reg_key, 0, NULL,
                        REG_OPTION_VOLATILE, KEY_WRITE, NULL, &hkey, NULL))
    {
        ret = FALSE;
        goto done;
    }

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

done:
    release_display_devices_init_mutex(mutex);
    return ret;
}

static struct wayland_output *wayland_get_output(struct wayland *wayland, LPCWSTR name)
{
    struct wayland_output *output;

    wl_list_for_each(output, &wayland->output_list, link)
    {
        if (!lstrcmpiW(name, output->name))
            return output;
    }

    return NULL;
}

static void populate_devmode(struct wayland_output_mode *output_mode, DEVMODEW *mode)
{
    mode->dmFields = DM_DISPLAYORIENTATION | DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT |
                     DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY | DM_POSITION;
    mode->u1.s2.dmDisplayOrientation = DMDO_DEFAULT;
    mode->u2.dmDisplayFlags = 0;
    mode->u1.s2.dmPosition.x = 0;
    mode->u1.s2.dmPosition.y = 0;
    mode->dmBitsPerPel = 32;
    mode->dmPelsWidth = output_mode->width;
    mode->dmPelsHeight = output_mode->height;
    mode->dmDisplayFrequency = output_mode->refresh / 100;
}

static BOOL wayland_get_current_devmode(struct wayland *wayland, LPCWSTR name, DEVMODEW *mode)
{
    struct wayland_output *output;

    output = wayland_get_output(wayland, name);
    if (!output)
        return FALSE;

    if (!output->current_wine_mode)
        return FALSE;

    populate_devmode(output->current_wine_mode, mode);

    return TRUE;
}

static BOOL wayland_get_devmode(struct wayland *wayland, LPCWSTR name, DWORD n, DEVMODEW *mode)
{
    struct wayland_output *output;
    struct wayland_output_mode *output_mode;
    DWORD i = 0;

    output = wayland_get_output(wayland, name);
    if (!output)
        return FALSE;

    wl_list_for_each(output_mode, &output->mode_list, link)
    {
        if (i == n)
        {
            populate_devmode(output_mode, mode);
            return TRUE;
        }
        i++;
    }

    return FALSE;
}


/***********************************************************************
 *		EnumDisplaySettingsEx  (WAYLAND.@)
 *
 */
BOOL CDECL WAYLAND_EnumDisplaySettingsEx(LPCWSTR name, DWORD n, LPDEVMODEW devmode, DWORD flags)
{
    static const WCHAR dev_name[CCHDEVICENAME] =
        {'W','i','n','e',' ','W','a','y','l','a','n','d',' ','d','r','i','v','e','r',0};
    struct wayland *wayland = thread_init_wayland();

    TRACE("(%s,%d,%p,0x%08x) wayland=%p\n", debugstr_w(name), n, devmode, flags, wayland);

    if (n == ENUM_REGISTRY_SETTINGS)
    {
        if (!read_registry_settings(name, devmode))
        {
            ERR("Failed to get %s registry display settings.\n", wine_dbgstr_w(name));
            return FALSE;
        }
        goto done;
    }

    if (n == ENUM_CURRENT_SETTINGS)
    {
        if (!wayland_get_current_devmode(wayland, name, devmode))
        {
            ERR("Failed to get %s current display settings.\n", wine_dbgstr_w(name));
            return FALSE;
        }
        goto done;
    }

    if (!wayland_get_devmode(wayland, name, n, devmode))
    {
        ERR("Modes index out of range\n");
        SetLastError(ERROR_NO_MORE_FILES);
        return FALSE;
    }

done:
    TRACE("=> %dx%d\n", devmode->dmPelsWidth, devmode->dmPelsHeight);
    /* Set generic fields */
    devmode->dmSize = FIELD_OFFSET(DEVMODEW, dmICMMethod);
    devmode->dmDriverExtra = 0;
    devmode->dmSpecVersion = DM_SPECVERSION;
    devmode->dmDriverVersion = DM_SPECVERSION;
    lstrcpyW(devmode->dmDeviceName, dev_name);
    return TRUE;
}

static struct wayland_output_mode *get_matching_output_mode(struct wayland_output *output,
                                                            LPDEVMODEW devmode)
{
    struct wayland_output_mode *output_mode;

    wl_list_for_each(output_mode, &output->mode_list, link)
    {
        if (devmode->dmPelsWidth == output_mode->width &&
            devmode->dmPelsHeight == output_mode->height)
            return output_mode;
    }

    return NULL;
}

/***********************************************************************
 *		ChangeDisplaySettingsEx  (WAYLAND.@)
 *
 */
LONG CDECL WAYLAND_ChangeDisplaySettingsEx(LPCWSTR devname, LPDEVMODEW devmode,
                                           HWND hwnd, DWORD flags, LPVOID lpvoid)
{
    struct wayland *wayland = thread_wayland();
    struct wayland_output *output;
    struct wayland_output_mode *output_mode;

    TRACE("(%s,%p,%p,0x%08x,%p) %dx%d@%d wayland=%p\n",
          debugstr_w(devname), devmode, hwnd, flags, lpvoid,
          devmode->dmPelsWidth, devmode->dmPelsHeight,
          devmode->dmDisplayFrequency, wayland);

    output = wayland_get_output(wayland, devname);
    if (!output)
        return DISP_CHANGE_BADPARAM;

    output_mode = get_matching_output_mode(output, devmode);
    if (!output_mode)
        return DISP_CHANGE_BADMODE;

    if (flags & CDS_UPDATEREGISTRY)
    {
        if (!write_registry_settings(devname, devmode))
        {
            ERR("Failed to write %s display settings to registry.\n", wine_dbgstr_w(devname));
            return DISP_CHANGE_NOTUPDATED;
        }
    }

    if (flags & (CDS_TEST | CDS_NORESET))
        return DISP_CHANGE_SUCCESSFUL;

    /* The notification needs to happen before we reinit the display devices,
     * in order for the reinit to read the new current mode. */
    wayland_notify_wine_mode_change(output->id, output_mode->width, output_mode->height);

    wayland_init_display_devices(wayland, TRUE);

    TRACE("set current wine mode %dx%d wine_scale %f\n",
          output_mode->width, output_mode->height, output->wine_scale);

    SendMessageTimeoutW(HWND_BROADCAST, WM_DISPLAYCHANGE, 32,
                        MAKELPARAM(output_mode->width, output_mode->height),
                        SMTO_ABORTIFHUNG, 2000, NULL);

    return DISP_CHANGE_SUCCESSFUL;
}

#endif