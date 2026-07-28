//Directives
#define UNICODE
#define _UNICODE
#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

//HMENU table for window message processing
#define HMENU_MAINWINDOW 1

//Global variables

//Function prototypes
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR nCmdLine, int nCmdShow) 
{    
    WNDCLASS MW = {0};
    const wchar_t MAIN_WINDOW_CLASSNAME[] = L"Main Window";

    MW.lpszClassName = MAIN_WINDOW_CLASSNAME;
    MW.lpfnWndProc = WindowProcedure;
    MW.hInstance = hInstance;
    MW.hCursor = LoadCursor(hInstance, IDC_ARROW);

    if (!RegisterClass(&MW)) {
        return -1;
    }

    HWND hwnd_MainWindow = CreateWindowEx(
        0,  
        MAIN_WINDOW_CLASSNAME,
        L"Bullet Hell",
        WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        100, 100,
        500, 500,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd_MainWindow == NULL) {
        return -1;
    }

    //Main message loop
    while (true) {
        MSG msg = {0};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                exit(0);
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_CREATE: 
            break;

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        } break;
        
        default: {
            return DefWindowProcW(hWnd, uMsg, wParam, lParam);
        } break;
    }
}

