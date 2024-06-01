#include <windows.h>

#include "types.h"
#include "win32.c"
#include "performance.c"
#include "format.c"
#include "string.c"

typedef enum Mode
{
    ModeNormal,
    ModeInsert
} Mode;

Mode mode = ModeNormal;

bool ignoreNextCharEvent = 0;

bool isRunning = 1;
bool isFullscreen = 0;
// f32 appScale = 1.0f;

HDC dc;
BITMAPINFO bitmapInfo;
MyBitmap canvas;
HBITMAP bitmap;

HFONT consolasFont;
TEXTMETRICA tm;

i32 pagePaddingX = 20;
i32 pagePaddingY = 15;
i32 zDelta = 0;

i32 cursorPosition = 0;

FileContent file;
WideString wFile;

inline void PaintRect(MyBitmap *destination, i32 offsetX, i32 offsetY, u32 width, u32 height, u32 color)
{
    if (offsetY < 0)
    {
        height += offsetY;
        offsetY = 0;
    }
    // need to check bounds of the screen
    u32 *row = (u32 *)destination->pixels + offsetX + offsetY * destination->width;
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
void InitMyFont()
{
    consolasFont = CreateFontA(-MulDiv(14, GetDeviceCaps(dc, LOGPIXELSY), USER_DEFAULT_SCREEN_DPI), 0, 0, 0,
                               FW_NORMAL, // Weight
                               0,         // Italic
                               0,         // Underline
                               0,         // Strikeout
                               DEFAULT_CHARSET,
                               OUT_TT_ONLY_PRECIS,
                               CLIP_DEFAULT_PRECIS,

                               // I've experimented with the Chrome and it doesn't render LCD quality for fonts above 32px
                               CLEARTYPE_QUALITY,

                               DEFAULT_PITCH,
                               "Consolas");
    GetTextMetricsA(dc, &tm);
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
        // appScale = (float)GetDeviceCaps(dc, LOGPIXELSY) / (float)USER_DEFAULT_SCREEN_DPI;
        canvas.width = LOWORD(lParam);
        canvas.height = HIWORD(lParam);
        canvas.bytesPerPixel = 4;

        if (canvas.pixels)
            DeleteObject(bitmap);

        InitBitmapInfo(&bitmapInfo, canvas.width, canvas.height);

        bitmap = CreateDIBSection(dc, &bitmapInfo, DIB_RGB_COLORS, &canvas.pixels, 0, 0);

        SelectObject(dc, bitmap);
        SelectObject(dc, consolasFont);

        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, 0xffffff);

        break;

    case WM_CHAR:
        if (mode == ModeInsert)
        {
            if (ignoreNextCharEvent)
                ignoreNextCharEvent = 0;
            else
            {
                if (wParam == '\r')
                    InsertCharAt(&wFile, cursorPosition, L'\n');
                else
                    InsertCharAt(&wFile, cursorPosition, wParam);
                cursorPosition++;
            }
        }
        break;

    case WM_KEYDOWN:
        if (wParam == VK_F11)
        {
            isFullscreen = !isFullscreen;
            SetFullscreen(window, isFullscreen);
        }

        if (mode == ModeInsert && wParam == VK_ESCAPE)
            mode = ModeNormal;

        if (mode == ModeNormal)
        {

            if (wParam == 'I')
            {
                mode = ModeInsert;
                ignoreNextCharEvent = 1;
            }
            if (wParam == 'L' && cursorPosition < wFile.size - 1)
                cursorPosition++;
            if (wParam == 'H' && cursorPosition > 0)
                cursorPosition--;
            if (wParam == 'X')
            {
                RemoveWCharAtPosition(&wFile, cursorPosition);
                if (cursorPosition > wFile.size - 1)
                    cursorPosition = wFile.size - 1;
            }
        }
        break;
    }
    return DefWindowProc(window, message, wParam, lParam);
}

void PrintMetric(char *label, PerfMetric metric)
{
    u64 time = GetMicrosecondsFor(metric);

    char val[30] = {0};
    i32 len = FormatNumber(time, (char *)&val);

    TextOutA(dc, canvas.width - 80, canvas.height - tm.tmHeight, val, len);
}

void DrawFile()
{
    i32 middleY = pagePaddingY + zDelta;
    i32 x = pagePaddingX;

    WCHAR *lineStart = wFile.content;
    WCHAR *ch = lineStart;
    i32 lineLength = 0;

    for (int i = 0; i < wFile.size; i++)
    {
        if (i == cursorPosition)
        {
            SIZE size;
            GetTextExtentPoint32A(dc, "W", 1, &size);

            i32 lineOffset = lineStart - wFile.content;
            u32 carretColor = mode == ModeNormal ? 0xaa55aa : 0x55aa55;
            PaintRect(&canvas, x + size.cx * (cursorPosition - lineOffset), middleY, size.cx, tm.tmHeight, carretColor);
        }

        if (*ch == '\r')
        {
        }
        else if (*ch == '\n')
        {

            TextOutW(dc, x, middleY, lineStart, lineLength);
            lineStart = ch + 1;
            lineLength = 0;
            middleY += tm.tmHeight;
        }
        else
        {
            lineLength++;
        }
        ch++;
    }
    if (lineLength > 0)
        TextOutW(dc, x, middleY, lineStart, lineLength);
}

void WinMainCRTStartup()
{
    PreventWindowsDPIScaling();

    dc = CreateCompatibleDC(0);

    InitMyFont(dc);

    HWND window = OpenWindow(OnEvent, 0x222222);
    HDC windowDC = GetDC(window);

    MSG msg;

    InitPerf();

    // I need to figure what to do with /r symbols on windows if I want to have proper file handling
    file = ReadMyFileImp("..\\sample.txt");
    RemoveCarriageReturns(&file);

    wFile.size = MultiByteToWideChar(CP_UTF8, 0, file.content, file.size, 0, 0);

    wFile.capacity = wFile.size * 2;
    wFile.content = VirtualAllocateMemory(wFile.capacity);

    MultiByteToWideChar(CP_UTF8, 0, file.content, file.size, wFile.content, wFile.size);

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

        StartMetric(Draw);
        DrawFile();
        EndMetric(Draw);

        PrintMetric("Memory", Memory);

        StartMetric(DiBits);
        BitBlt(windowDC, 0, 0, canvas.width, canvas.height, dc, 0, 0, SRCCOPY);
        EndMetric(DiBits);

        // PrintMetric("Draw", Draw);
        // PrintMetric("DiBits", DiBits);

        Sleep(10); // TODO: proper FPS handling needed, this is just now not to burn CPU
    }

    ExitProcess(0);
}