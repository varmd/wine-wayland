# GDI driver

#@ cdecl wine_get_gdi_driver(long) WAYLANDDRV_get_gdi_driver

# USER driver

# keyboard funcs

#@ cdecl ActivateKeyboardLayout(long long) WAYLANDDRV_ActivateKeyboardLayout


#@ cdecl GetKeyNameText(long ptr long) WAYLANDDRV_GetKeyNameText
#@ cdecl GetKeyboardLayout(long) WAYLANDDRV_GetKeyboardLayout
#@ cdecl MapVirtualKeyEx(long long long) WAYLANDDRV_MapVirtualKeyEx

#@ cdecl GetKeyboardLayoutName(ptr) WAYLANDDRV_GetKeyboardLayoutName
#@ cdecl LoadKeyboardLayout(wstr long) WAYLANDDRV_LoadKeyboardLayout

#@ cdecl ToUnicodeEx(long long ptr ptr long long long) WAYLANDDRV_ToUnicodeEx

#@ cdecl VkKeyScanEx(long long) WAYLANDDRV_VkKeyScanEx
#@ cdecl GetCursorPos(ptr) WAYLANDDRV_GetCursorPos

#@ cdecl ShowWindow(long long ptr long) WAYLANDDRV_ShowWindow
#@ cdecl SetCursor(long) WAYLANDDRV_SetCursor
@ cdecl ShowCursor(long) WAYLANDDRV_ShowCursor
#@ cdecl ClipCursor(ptr) WAYLANDDRV_ClipCursor

#@ cdecl ChangeDisplaySettingsEx(ptr ptr long long long) WAYLANDDRV_ChangeDisplaySettingsEx

#@ cdecl EnumDisplayMonitors(long ptr ptr long) WAYLANDDRV_EnumDisplayMonitors
#@ cdecl EnumDisplaySettingsEx(ptr long ptr long) WAYLANDDRV_EnumDisplaySettingsEx
#@ cdecl GetMonitorInfo(long ptr) WAYLANDDRV_GetMonitorInfo
#@ cdecl CreateWindow(long) WAYLANDDRV_CreateWindow
#@ cdecl DestroyWindow(long) WAYLANDDRV_DestroyWindow
#@ cdecl MsgWaitForMultipleObjectsEx(long ptr long long long) WAYLANDDRV_MsgWaitForMultipleObjectsEx
#@ cdecl SysCommand(long long long) WAYLANDDRV_SysCommand
#@ cdecl WindowPosChanging(long long long ptr ptr ptr ptr) WAYLANDDRV_WindowPosChanging
#@ cdecl WindowPosChanged(long long long ptr ptr ptr ptr ptr) WAYLANDDRV_WindowPosChanged

# Cursor


# @ cdecl DestroyCursorIcon(long) WAYLANDDRV_DestroyCursorIcon


# @ cdecl SetCursorPos(long long) WAYLANDDRV_SetCursorPos

# @ cdecl CreateDesktopWindow(long) WAYLANDDRV_CreateDesktopWindow

# @ cdecl FlashWindowEx(ptr) WAYLANDDRV_FlashWindowEx


# @ cdecl GetDC(long long long ptr ptr long) WAYLANDDRV_GetDC




# @ cdecl SetActiveWindow(long) WAYLANDDRV_SetActiveWindow
#@ cdecl Beep() WAYLANDDRV_Beep
# @ cdecl SetCapture(long long) WAYLANDDRV_SetCapture
# @ cdecl ReleaseCapture( ) WAYLANDDRV_ReleaseCapture

# @ cdecl SetFocus(long) WAYLANDDRV_SetFocus
# @ cdecl SetLayeredWindowAttributes(long long long long) WAYLANDDRV_SetLayeredWindowAttributes
#@ cdecl SetParent(long long long) WAYLANDDRV_SetParent

#@ cdecl UnloadKeyboardLayout(long) WAYLANDDRV_UnloadKeyboardLayout

# @ cdecl SetWindowIcon(long long long) WAYLANDDRV_SetWindowIcon
# @ cdecl SetWindowRgn(long long long) WAYLANDDRV_SetWindowRgn
# @ cdecl SetWindowStyle(ptr long ptr) WAYLANDDRV_SetWindowStyle
# @ cdecl SetWindowText(long wstr) WAYLANDDRV_SetWindowText





#@ cdecl UpdateClipboard() WAYLANDDRV_UpdateClipboard

#@ cdecl UpdateLayeredWindow(long ptr ptr) WAYLANDDRV_UpdateLayeredWindow

#@ cdecl WindowMessage(long long long long) WAYLANDDRV_WindowMessage



# @ cdecl SystemParametersInfo(long long ptr long) WAYLANDDRV_SystemParametersInfo
# @ cdecl UpdateCandidatePos(long ptr) WAYLANDDRV_UpdateCandidatePos
# @ cdecl ThreadDetach() WAYLANDDRV_ThreadDetach

# Desktop
#@ cdecl wine_create_desktop(long long) WAYLANDDRV_create_desktop
