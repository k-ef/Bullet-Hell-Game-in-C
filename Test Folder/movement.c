//Directives
#define UNICODE
#define _UNICODE
#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

//HMENU table for window message processing
#define HMENU_MAINWINDOW 1

//Global variables
uint32_t CURSOR_SIZE_X = 3;
uint32_t CURSOR_SIZE_Y = 3;
bool KEYBOARD[256] = {0};

//Structs
struct position {
    uint32_t x;
    uint32_t y;
} character = {0};

//Function prototypes
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void DrawBackground(HWND hWnd, HDC device_context, PAINTSTRUCT paint_info); 
void InitCharacter(HWND hWnd);
void DrawCharacter(HWND hWnd, HDC device_context, PAINTSTRUCT paint_info);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR nCmdLine, int nCmdShow) 
{    
    WNDCLASS MW = {0};
    const wchar_t MAIN_WINDOW_CLASSNAME[] = L"Main Window";

    MW.lpszClassName = MAIN_WINDOW_CLASSNAME;
    MW.lpfnWndProc = WindowProcedure;
    MW.hInstance = hInstance;
    MW.hCursor = LoadCursor(NULL, IDC_ARROW);
    MW.style = CS_HREDRAW | CS_VREDRAW;

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

        if (KEYBOARD[VK_RIGHT] || KEYBOARD['D']) character.x++;
        if (KEYBOARD[VK_LEFT] || KEYBOARD['A']) character.x--;
        if (KEYBOARD[VK_UP] || KEYBOARD['W']) character.y--;
        if (KEYBOARD[VK_DOWN] || KEYBOARD['S']) character.y++;

        printf("(%d, %d)", character.x, character.y);
        InvalidateRect(hwnd_MainWindow, NULL, FALSE);
        UpdateWindow(hwnd_MainWindow);
    }

    return 0;
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool has_focus = true;

    switch (uMsg) {
        case WM_CREATE: {
            InitCharacter(hWnd);
        } break;

        case WM_PAINT: {
            PAINTSTRUCT paint_info;
            HDC device_context = BeginPaint(hWnd, &paint_info);
            // DrawBackground(hWnd, device_context, paint_info);
            DrawCharacter(hWnd, device_context, paint_info);
        } break;

        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_KEYDOWN: {
            if (has_focus) {
                static bool curr_keydown, prev_keydown;
                curr_keydown = ((lParam & (1 << 31)) == 0);
                prev_keydown = ((lParam & (1 << 30)) != 0);
                if (curr_keydown != prev_keydown) {
                    KEYBOARD[(uint8_t)wParam] = curr_keydown;
                    if (curr_keydown) {
                        switch (wParam) {
                            case VK_ESCAPE: PostQuitMessage(0); break;
                        }
                    }
                }
            }
        } break;

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        } break;
        
        default: {
            return DefWindowProcW(hWnd, uMsg, wParam, lParam);
        } break;
    }
    return wParam;
}

void DrawBackground(HWND hWnd, HDC device_context, PAINTSTRUCT paint_info) 
{
    RECT bg;

    GetClientRect(hWnd, &bg);

    if (bg.bottom == 0) {
        return;
    }
    
    //Draw BG
    for (int i = 0; i < bg.right; i++) {
        for (int j = 0; j < bg.bottom; j++) {
            SetPixel(device_context, i, j, RGB(0, 0, 0));
        }
    }
}

void InitCharacter(HWND hWnd)
{
    RECT play_area;
    GetClientRect(hWnd, &play_area);

    character.x = play_area.right / 2;
    character.y = play_area.bottom / 2;

    // printf("cursorx: %d\ncursory: %d\n", cursor.x, cursor.y);
    // printf("left: %d\nright: %d\n", play_area.left, play_area.right);
    // printf("top: %d\nbottom: %d\n", play_area.top, play_area.bottom);
}

void DrawCharacter(HWND hWnd, HDC device_context, PAINTSTRUCT paint_info)
{
    //Draw character
    int size_offset_x = CURSOR_SIZE_X / 2;
    int size_offset_y = CURSOR_SIZE_Y / 2;
    for (int x = character.x - size_offset_x; x <= character.x + size_offset_x; x++) {
        for (int y = character.y - size_offset_y; y <= character.y + size_offset_y; y++) {
            SetPixel(device_context, x, y, RGB(255, 0, 0));
            // printf("(%d, %d)", x, y);
        }
    }
}