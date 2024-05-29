#pragma once

#include <windows.h>
#include <dwmapi.h>
#include "types.h"

HWND OpenWindow(WNDPROC OnEvent, u32 color)
{
    HINSTANCE instance = GetModuleHandle(0);
    WNDCLASSW windowClass = {0};
    windowClass.hInstance = instance;
    windowClass.lpfnWndProc = OnEvent;
    windowClass.lpszClassName = L"MyWindow";
    windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.hbrBackground = CreateSolidBrush(color); // fixes a flash of a white background for a couple of frames during start

    RegisterClassW(&windowClass);

    HWND window = CreateWindowW(windowClass.lpszClassName, (wchar_t *)"Editor",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                0, 0, instance, 0);

    BOOL USE_DARK_MODE = TRUE;
    BOOL SET_IMMERSIVE_DARK_MODE_SUCCESS = SUCCEEDED(DwmSetWindowAttribute(
        window, DWMWA_USE_IMMERSIVE_DARK_MODE, &USE_DARK_MODE, sizeof(USE_DARK_MODE)));

    ShowWindow(window, SW_SHOW);
    return window;
}

// DPI Scaling
// user32.dll is linked statically, so dynamic linking won't load that dll again
// taken from https://github.com/cmuratori/refterm/blob/main/refterm.c#L80
// this is done because GDI font drawing is ugly and unclear when DPI scaling is enabled

typedef BOOL WINAPI set_process_dpi_aware(void);
typedef BOOL WINAPI set_process_dpi_awareness_context(DPI_AWARENESS_CONTEXT);
static void PreventWindowsDPIScaling()
{
    HMODULE WinUser = LoadLibraryW(L"user32.dll");
    set_process_dpi_awareness_context *SetProcessDPIAwarenessContext = (set_process_dpi_awareness_context *)GetProcAddress(WinUser, "SetProcessDPIAwarenessContext");
    if (SetProcessDPIAwarenessContext)
    {
        SetProcessDPIAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    }
    else
    {
        set_process_dpi_aware *SetProcessDPIAware = (set_process_dpi_aware *)GetProcAddress(WinUser, "SetProcessDPIAware");
        if (SetProcessDPIAware)
        {
            SetProcessDPIAware();
        }
    }
}
