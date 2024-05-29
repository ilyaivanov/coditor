#include <windows.h>

#include "types.h"
#include "win32.c"

bool isRunning = 1;

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

    case WM_LBUTTONDOWN:
        break;

    case WM_LBUTTONUP:
        break;

    case WM_SIZE:
        HDC dc = GetDC(window);
        break;

    case WM_KEYDOWN:
        break;
    }
    return DefWindowProc(window, message, wParam, lParam);
}

void WinMainCRTStartup()
{
    PreventWindowsDPIScaling();

    HWND window = OpenWindow(OnEvent, 0x222222);
    HDC dc = GetDC(window);

    MSG msg;

    while (isRunning)
    {
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        Sleep(10); // TODO: proper FPS handling needed, this is just now not to burn CPU
    }

    ExitProcess(0);
}