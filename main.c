#include <windows.h>

#include "types.h"
#include "win32.c"
#include "performance.c"
#include "format.c"
#include "font.c"

bool isRunning = 1;
bool isFullscreen = 0;
f32 appScale = 1.0f;

HDC newDc;
BITMAPINFO bitmapInfo;
MyBitmap canvas;
HBITMAP myNewBitmap;

i32 zDelta = 0;
FontData consolasFont14;
FontData consolasFontSelected14;

inline void CopyBitmapRectTo(MyBitmap *sourceT, MyBitmap *destination, i32 offsetX, i32 offsetY)
{
    // TODO: this should be fixed
    if (offsetY < 0 || offsetX < 0)
        return;

    u32 *row = (u32 *)destination->pixels + destination->width * (destination->height - 1) + offsetX - offsetY * destination->width;
    u32 *source = (u32 *)sourceT->pixels + sourceT->width * (sourceT->height - 1);
    for (i32 y = 0; y < sourceT->height; y += 1)
    {
        u32 *pixel = row;
        u32 *sourcePixel = source;
        for (i32 x = 0; x < sourceT->width; x += 1)
        {
            // stupid fucking logic needs to replaced
            if (*sourcePixel != 0xff000000 && (y + offsetY) > 0 && (x + offsetX) > 0 && y + offsetY < destination->height && x + offsetX < destination->width)
                *pixel = *sourcePixel;
            sourcePixel += 1;
            pixel += 1;
        }
        source -= sourceT->width;
        row -= destination->width;
    }
}

inline void PaintRect(MyBitmap *destination, i32 offsetX, i32 offsetY, u32 width, u32 height, u32 color)
{
    if (offsetY < 0)
    {
        height += offsetY;
        offsetY = 0;
    }
    // need to check bounds of the screen
    u32 *row = (u32 *)destination->pixels + offsetX - offsetY * destination->width;
    for (i32 y = 0; y < height; y += 1)
    {
        u32 *pixel = row;
        for (i32 x = 0; x < width; x += 1)
        {
            if ((y + offsetY) >= 0 && (x + offsetX) >= 0 && y + offsetY < destination->height && x + offsetX < destination->width)
                *pixel = color;
            pixel += 1;
        }
        row += destination->width;
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

    case WM_MOUSEWHEEL:
        zDelta += GET_WHEEL_DELTA_WPARAM(wParam);
        break;

    case WM_SIZE:
        HDC dc = GetDC(window);
        appScale = (float)GetDeviceCaps(dc, LOGPIXELSY) / (float)USER_DEFAULT_SCREEN_DPI;
        canvas.width = LOWORD(lParam);
        canvas.height = HIWORD(lParam);
        canvas.bytesPerPixel = 4;

        // if (canvas.pixels)
        //     DeleteObject(bitmap);

        InitBitmapInfo(&bitmapInfo, canvas.width, canvas.height);

        myNewBitmap = CreateDIBSection(newDc, &bitmapInfo, DIB_RGB_COLORS, &canvas.pixels, 0, 0);

        // fontCanvas = (MyBitmap){.bytesPerPixel = 4, .height = textureSize, .width = textureSize, .pixels = bits};

        // TODO: Initialize Arena of screen size and assign proper width and height on resize

        // if (canvas.pixels)
        //     VirtualFreeMemory(canvas.pixels);
        // canvas.pixels = VirtualAllocateMemory(sizeof(u32) * canvas.width * canvas.height);
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

FileContent file;
i32 selectedChar = 2;

void DrawFile()
{
    i32 middleY = 20 + zDelta;
    i32 x = 20;
    char *lineStart = file.content;
    char *ch = lineStart;
    i32 lineLength = 0;

    for (int i = 0; i < file.size; i++)
    {
        if (*ch == '\r')
        {
        }
        else if (*ch == '\n')
        {

            DrawLine(myNewBitmap, newDc, x, middleY, lineStart, lineLength);
            lineStart = ch + 1;
            lineLength = 0;
            // x = 20;
            middleY += 22;
        }
        else
        {

            lineLength++;
            // MyBitmap *texture = &currentFont->textures[*ch];
            // CopyBitmapRectTo(texture, &canvas, x, middleY);
            // x += texture->width + GetKerningValue(*ch, *(ch + 1));
        }
        ch++;
    }
    if (lineLength > 0)
        DrawLine(myNewBitmap, newDc, x, middleY, lineStart, lineLength);
}

void WinMainCRTStartup()
{
    PreventWindowsDPIScaling();

    HWND window = OpenWindow(OnEvent, 0x222222);
    HDC windowDC = GetDC(window);

    newDc = CreateCompatibleDC(0);

    MSG msg;

    InitPerf();
    // InitFontSystem();
    InitMyFont();

    // Arena arena = CreateArena(Megabytes(44));
    // InitFont(&consolasFont14, FontInfoClearType("Consolas", 14, 0xFFFFFF, 0x00111111), &arena);
    // InitFont(&consolasFontSelected14, FontInfoClearType("Consolas", 14, 0x000000, 0x00FFFFFF), &arena);

    file = ReadMyFileImp("..\\main.c");

    while (isRunning)
    {
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        StartMetric(Memory);
        memset(canvas.pixels, 0x11, canvas.bytesPerPixel * canvas.width * canvas.height);
        EndMetric(Memory);

        // PaintRect(&canvas, 0, 0, 10, 10, 0xffffff);
        // DrawChar(myNewBitmap, newDc);

        StartMetric(Draw);
        DrawFile();
        EndMetric(Draw);

        StartMetric(DiBits);
        StretchDIBits(windowDC, 0, 0, canvas.width, canvas.height, 0, 0, canvas.width, canvas.height, canvas.pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        EndMetric(DiBits);

        PrintMetric("Memory", Memory);
        PrintMetric("Draw", Draw);
        PrintMetric("DiBits", DiBits);

        Sleep(10); // TODO: proper FPS handling needed, this is just now not to burn CPU
    }

    ExitProcess(0);
}