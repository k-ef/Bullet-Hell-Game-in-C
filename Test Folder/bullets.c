//Directives
#define UNICODE
#define _UNICODE
#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//HMENU table for window message processing
#define HMENU_MAINWINDOW 1
#define MAX_BULLETS 200

//Structs
struct position {
    uint32_t x;
    uint32_t y;
} character = {0};

typedef struct bullet_tag {
    double x1, x2;
    double y1, y2;
    uint32_t thickness;
    int length;
    double speed;
    double dx, dy;
} bullet_struct;

typedef struct point_tag {
    double x;
    double y;
} point_struct;

typedef struct line_segment_tag {
    point_struct p1;
    point_struct p2;
} line_segment_struct;


//Global variables
uint32_t CHARACTER_SIZE_X = 7;
uint32_t CHARACTER_SIZE_Y = 7;
bool KEYBOARD[256] = {0};
RECT CHARACTER_RECT;
RECT PLAY_AREA;
bool MOVED = false;
bullet_struct *bullets[MAX_BULLETS] = {NULL}; //holder of bullets
const int CHARACTER_SPEED = 3;
uint32_t BULLET_COUNT = 0;
bool RAN = 0;

//Function prototypes
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void DrawBackground(HWND hWnd, HDC device_context, PAINTSTRUCT paint_info); 
void InitCharacter(HWND hWnd);
void DrawCharacter(HWND hWnd, HDC device_context, PAINTSTRUCT paint_info);
bullet_struct * CreateBullet(HWND hWnd);
void DrawBullet(HWND hWnd, HDC device_context, PAINTSTRUCT paint_info);
void DestroyBullet(HWND hWnd, int index);
bool DoIntersect(line_segment_struct line1, line_segment_struct line2);
bool ValidProjection(double a, double b, double c, double d);
double Orientation(point_struct p1, point_struct p2, point_struct p3);
void Swap(double *a, double *b);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR nCmdLine, int nCmdShow) 
{    
    srand(time(NULL));

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

    clock_t start, end;
    double elapsed, prev_elapsed = 0.0;
    start = clock();
    printf("USE ARROW KEYS TO MOVE\n");
    printf("TRY NOT TO GET HIT BY THE BULLETS\n");

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

        int size_offset_x = CHARACTER_SIZE_X / 2;
        int size_offset_y = CHARACTER_SIZE_Y / 2;
        if (KEYBOARD[VK_RIGHT] && ((int)character.x + 1 + size_offset_x) <= PLAY_AREA.right) {
           character.x += CHARACTER_SPEED; 
           MOVED = true;
        }  
        if (KEYBOARD[VK_LEFT]  && ((int)character.x - 1 - size_offset_x) >= PLAY_AREA.left) {
            character.x -= CHARACTER_SPEED;
            MOVED = true;
        } 
        if (KEYBOARD[VK_DOWN]  && ((int)character.y + 1 + size_offset_y) <= PLAY_AREA.bottom) {
            character.y += CHARACTER_SPEED;
            MOVED = true;
        } 
        if (KEYBOARD[VK_UP]    && ((int)character.y - 1 - size_offset_y) >= PLAY_AREA.top) {
            character.y -= CHARACTER_SPEED;
            MOVED = true;
        }   

        //Add Bullets
        end = clock();
        elapsed = (double)(end - start)/CLOCKS_PER_SEC;
        if (elapsed >= prev_elapsed + 0.5) {
            if (BULLET_COUNT < MAX_BULLETS) {
                bullet_struct *temp_bullet = CreateBullet(hwnd_MainWindow);
                bullets[BULLET_COUNT] = temp_bullet;
                // printf("bullet count: %d\n", BULLET_COUNT);
                BULLET_COUNT++;
                // printf("Addition of bullet to array successful!\n");
                printf("\n");
            }

            prev_elapsed = elapsed;
        }

        //Check if hit
        for (int i = 0; i < BULLET_COUNT; i++) {
            if (bullets[i] == NULL) {
                continue;
            }

            line_segment_struct bullet_line = {{bullets[i]->x1, bullets[i]->y1}, {bullets[i]->x1 + bullets[i]->dx, bullets[i]->y1 + bullets[i]->dy}};
            line_segment_struct character_top = {{CHARACTER_RECT.left, CHARACTER_RECT.top}, {CHARACTER_RECT.right, CHARACTER_RECT.top}};
            line_segment_struct character_right = {{CHARACTER_RECT.right, CHARACTER_RECT.top}, {CHARACTER_RECT.right, CHARACTER_RECT.bottom}};
            line_segment_struct character_bottom = {{CHARACTER_RECT.left, CHARACTER_RECT.bottom}, {CHARACTER_RECT.right, CHARACTER_RECT.bottom}};
            line_segment_struct character_left = {{CHARACTER_RECT.left, CHARACTER_RECT.top}, {CHARACTER_RECT.left, CHARACTER_RECT.bottom}};

            if (DoIntersect(bullet_line, character_top) || DoIntersect(bullet_line, character_right) || DoIntersect(bullet_line, character_bottom) || DoIntersect(bullet_line, character_left)) {
                printf("Hit!");
            }
            
        }
        
        InvalidateRect(hwnd_MainWindow, &PLAY_AREA, FALSE);
        UpdateWindow(hwnd_MainWindow);
        Sleep(1);
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
            DrawBackground(hWnd, device_context, paint_info);
            DrawBullet(hWnd, device_context, paint_info);
            DrawCharacter(hWnd, device_context, paint_info);
            EndPaint(hWnd, &paint_info);
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
    HGDIOBJ original = NULL;
    original = SelectObject(device_context, GetStockObject(DC_BRUSH));
    SelectObject(device_context, GetStockObject(WHITE_BRUSH));
    SelectObject(device_context, GetStockObject(WHITE_PEN));
    // Rectangle(device_context, CHARACTER_RECT.left, CHARACTER_RECT.top, CHARACTER_RECT.right, CHARACTER_RECT.bottom);
    Rectangle(device_context, PLAY_AREA.left, PLAY_AREA.top, PLAY_AREA.right, PLAY_AREA.bottom);
    SelectObject(device_context, original);
}

void InitCharacter(HWND hWnd)
{
    GetClientRect(hWnd, &PLAY_AREA);
    character.x = PLAY_AREA.right / 2;
    character.y = PLAY_AREA.bottom / 2;

    // printf("cursorx: %d\ncursory: %d\n", character.x, character.y);
    // printf("left: %d\nright: %d\n", PLAY_AREA.left, PLAY_AREA.right);
    // printf("top: %d\nbottom: %d\n", PLAY_AREA.top, PLAY_AREA.bottom);
}

void DrawCharacter(HWND hWnd, HDC device_context, PAINTSTRUCT paint_info)
{
    //Draw character
    int size_offset_x = CHARACTER_SIZE_X / 2; //These could also be global but ICBA
    int size_offset_y = CHARACTER_SIZE_Y / 2;

    HGDIOBJ original = NULL;
    original = SelectObject(device_context, GetStockObject(DC_BRUSH));
    SelectObject(device_context, GetStockObject(BLACK_BRUSH));

    Rectangle(device_context, character.x - size_offset_x, character.y - size_offset_y, character.x + size_offset_x, character.y + size_offset_y);

    SelectObject(device_context, original);

    CHARACTER_RECT.left = character.x - size_offset_x;
    CHARACTER_RECT.right = character.x + size_offset_x;
    CHARACTER_RECT.top = character.y - size_offset_y;
    CHARACTER_RECT.bottom = character.y + size_offset_y;

    // printf("left: %d\nright: %d\ntop: %d\nbottom: %d\n", CHARACTER_RECT.left, CHARACTER_RECT.right, CHARACTER_RECT.top, CHARACTER_RECT.bottom);

    // printf("(%d, %d)", x, y);
}

bullet_struct * CreateBullet(HWND hWnd) 
{
    bullet_struct *bullet = malloc(sizeof(bullet_struct));

    if (bullet == NULL) {
        printf("Cannot allocate space for bullets\n");
        return (bullet_struct *)-1;
    }

    //x1, y1
    bullet->x1 = rand() % PLAY_AREA.right;
    
    int random_number = rand();
    if (bullet->x1 > 0 && bullet->x1 < PLAY_AREA.right) {
        bullet->y1 = (random_number & 1) ? 0 : PLAY_AREA.bottom;
    } else bullet->y1 = random_number % PLAY_AREA.bottom;
    
    //x2, y2
    bullet->x2 = character.x;
    bullet->y2 = character.y;

    //dx, dy
    double dx = character.x - bullet->x1;
    double dy = character.y - bullet->y1;
    double len = sqrt(pow(dx, 2) + pow(dy, 2));
    bullet->dx = 15 * (dx / len);
    bullet->dy = 15 * (dy / len);

    //length;
    bullet->length = 4;

    //speed
    bullet->speed = 1;

    //thickness
    bullet->thickness = 1;

    // printf("Creation of bullet successful!\n");
    return bullet;
}

void DrawBullet(HWND hWnd, HDC device_context, PAINTSTRUCT paint_info)
{
    HGDIOBJ original = NULL;
    original = SelectObject(device_context, GetStockObject(DC_PEN));
    SelectObject(device_context, GetStockObject(BLACK_PEN));
    for (int i = 0; i < BULLET_COUNT; i++) {
        if (bullets[i] == NULL) {
            continue;
        }

        MoveToEx(device_context, 
            bullets[i]->x1, bullets[i]->y1, 
            NULL);

        LineTo(device_context, 
            bullets[i]->x1 + bullets[i]->dx, 
            bullets[i]->y1 + bullets[i]->dy);

        bullets[i]->x1 += bullets[i]->dx;
        bullets[i]->y1 += bullets[i]->dy;
        
        if (bullets[i]->x1 < 0 || bullets[i]->x1 > PLAY_AREA.right || bullets[i]->y1 < 0 || bullets[i]->y1 > PLAY_AREA.bottom) {
            DestroyBullet(hWnd, i);
        }
    }
    SelectObject(device_context, original);
}

void DestroyBullet(HWND hWnd, int index)
{
    bullets[index] = NULL;
}

bool DoIntersect(line_segment_struct line1, line_segment_struct line2)
{
    double o1 = Orientation(line1.p1, line1.p2, line2.p1);
    double o2 = Orientation(line1.p1, line1.p2, line2.p2);
    double o3 = Orientation(line2.p1, line2.p2, line1.p1);
    double o4 = Orientation(line2.p1, line2.p2, line1.p2);

    if (o1 != o2 && o3 != o4) {
        return true;
    }
    if (o1 == 0 && o4 == 0) {
        if (ValidProjection(line1.p1.x, line1.p2.x, line2.p1.x, line2.p2.x) && ValidProjection(line1.p1.y, line1.p2.y, line2.p1.y, line2.p2.y)) {
            return true;
        }
    }

    return false;

}

bool ValidProjection(double a, double b, double c, double d)
{
    if (a > b) {
        Swap(&a, &b);
    }
    if (c > d) {
        Swap(&c, &d);
    }
    return max(a, c) <= min(b, d);
}

double Orientation(point_struct p1, point_struct p2, point_struct p3)
{
    double val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);
    
    if (val == 0) {
        return 0;
    }
    
    return (val > 0) ? 1 : 2;
}

void Swap(double *a, double *b)
{
    double temp = *a;
    *a = *b;
    *b = temp;
}