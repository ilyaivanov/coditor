#include <windows.h>

#include "types.h"
#include "win32.c"
#include "performance.c"
#include "format.c"

bool isRunning = 1;
bool isFullscreen = 0;
f32 appScale = 1.0f;

BITMAPINFO bitmapInfo;
MyBitmap canvas;

inline void PaintRect(MyBitmap *destination, i32 offsetX, i32 offsetY, u32 width, u32 height, u32 color)
{
    // need to check bounds of the screen
    u32 *row = (u32 *)destination->pixels + destination->width * (destination->height - 1) + offsetX - offsetY * destination->width;
    for (i32 y = 0; y < height; y += 1)
    {
        u32 *pixel = row;
        for (i32 x = 0; x < width; x += 1)
        {
            if ((y + offsetY) > 0 && (x + offsetX) > 0 && y + offsetY < destination->height && x + offsetX < destination->width)
                *pixel = color;
            pixel += 1;
        }
        row -= destination->width;
    }
}

LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_QUIT:
    case WM_DESTROY:
        PostQuitMessage(0);
        isRunning = 0;
        break;

    case WM_PAINT:
        PAINTSTRUCT paint;
        BeginPaint(window, &paint);
        EndPaint(window, &paint);
        break;

    case WM_SIZE:
        HDC dc = GetDC(window);
        appScale = (float)GetDeviceCaps(dc, LOGPIXELSY) / (float)USER_DEFAULT_SCREEN_DPI;
        canvas.width = LOWORD(lParam);
        canvas.height = HIWORD(lParam);
        canvas.bytesPerPixel = 4;
        InitBitmapInfo(&bitmapInfo, canvas.width, canvas.height);
        // TODO: Initialize Arena of screen size and assign proper width and height on resize

        if (canvas.pixels)
            VirtualFreeMemory(canvas.pixels);
        canvas.pixels = VirtualAllocateMemory(sizeof(u32) * canvas.width * canvas.height);
        break;

    case WM_KEYDOWN:
        if (wParam == VK_F11)
        {
            isFullscreen = !isFullscreen;
            SetFullscreen(window, isFullscreen);
        }
        break;
    }
    return DefWindowProc(window, message, wParam, lParam);
}

void PrintMetric(char *label, PerfMetric metric)
{
    u64 time = GetMicrosecondsFor(metric);

    OutputDebugStringA(label);
    OutputDebugStringA(": ");
    char val[30] = {0};
    FormatNumber(time, (char *)&val);
    OutputDebugStringA(val);
    OutputDebugStringA("us\n");
}

void WinMainCRTStartup()
{
    PreventWindowsDPIScaling();

    HWND window = OpenWindow(OnEvent, 0x222222);
    HDC dc = GetDC(window);

    MSG msg;

    InitPerf();
    while (isRunning)
    {
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        StartMetric(Memory);
        memset(canvas.pixels, 0, canvas.bytesPerPixel * canvas.width * canvas.height);
        EndMetric(Memory);

        PaintRect(&canvas, 200, 200, 200, 200, 0xffffff);

        StartMetric(DiBits);
        StretchDIBits(dc, 0, 0, canvas.width, canvas.height, 0, 0, canvas.width, canvas.height, canvas.pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        EndMetric(DiBits);

        PrintMetric("Memory", Memory);
        PrintMetric("DiBits", DiBits);

        Sleep(100); // TODO: proper FPS handling needed, this is just now not to burn CPU
    }

    ExitProcess(0);
}