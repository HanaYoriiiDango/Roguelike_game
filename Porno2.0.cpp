#include <windows.h>
#include <algorithm>
#include <wingdi.h> 
#include <vector>
#include <iostream>
#include <cmath>
#pragma comment(lib, "Msimg32.lib")

using namespace std;

struct { 
    HWND hwnd;
    HBITMAP hBack;
    HDC hdc, mem_dc;
    int width, height;

} window;

struct sprite {
    float x, y, speed, rad, dx, dy;
    int widht, height;
    bool active = true;
    HBITMAP hBitmap;

};

struct Room {
    int x, y; // координаты левого верхнего угла в ТАЙЛАХ
    int w, h; // ширина и высота в ТАЙЛАХ
};

struct {
    float x, y;              
    float targetX, targetY;  
    float smooth = 0.30f;    
    const int ZOOM = 4;

} camera;

struct {
    bool action = false;

} game;

sprite Hero;
 
const int TILE = 64;           // размер тайла в мировых пикселях
const int MAP_W = 64;
const int MAP_H = 64;

int map[MAP_H][MAP_W];
bool keys[256] = {};

bool showGrid = false;
HBITMAP hTexWall = NULL;
HBITMAP hTexGrass = NULL;

vector<Room> rooms;

void InitWindow() {

    RECT r;
    GetClientRect(window.hwnd, &r);

    window.width = r.right - r.left;
    window.height = r.bottom - r.top;

} 

void InitGame() {

    srand(time(nullptr));

    hTexWall = (HBITMAP)LoadImageW(NULL, L"wall_tile64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexGrass = (HBITMAP)LoadImageW(NULL, L"grass_tile64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    Hero.hBitmap = (HBITMAP)LoadImageW(NULL, L"fiol_girl128px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    if (!hTexWall || !hTexGrass) {
        MessageBoxW(NULL, L"Текстуры не загрузились! Проверь файлы wall.bmp и grass.bmp", L"Ошибка", MB_ICONERROR);
    }


   

    // 1. Заливаем ВСЮ карту стенами
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map[y][x] = 1;
        }
    }

    for (int i = 0; i < 15; i++) { // пытаемся создать 15 комнат
        int w = 4 + rand() % 6;
        int h = 4 + rand() % 6;
        int x = 1 + rand() % (MAP_W - w - 2); // чтобы не прилипали к краю
        int y = 1 + rand() % (MAP_H - h - 2);

        bool intersects = false;
        for (const Room& r : rooms) {
            // Проверка на пересечение с отступом в 1 тайл
            if (x < r.x + r.w + 1 && x + w + 1 > r.x &&
                y < r.y + r.h + 1 && y + h + 1 > r.y) {
                intersects = true;
                break;
            }
        }

        if (!intersects) {
            // Выкапываем и добавляем в rooms
            for (int ty = y; ty < y + h; ty++) {
                for (int tx = x; tx < x + w; tx++) {
                    map[ty][tx] = 0;
                }
            }
            rooms.push_back({ x, y, w, h });
        }
    }

    // 3. Ставим героя в центр ПЕРВОЙ комнаты
    Room firstRoom = rooms[0];
    // Центр комнаты в тайлах: x + w/2, y + h/2
    // Переводим в пиксели: * TILE
    Hero.x = (firstRoom.x + firstRoom.w / 2.0f) * TILE - Hero.widht / 2.0f;
    Hero.y = (firstRoom.y + firstRoom.h / 2.0f) * TILE - Hero.height / 2.0f;
    Hero.widht = 40;
    Hero.height = 40;
    Hero.speed = 60.0f;

    for (int i = 0; i < rooms.size() - 1; i++) {
        int cx1 = rooms[i].x + rooms[i].w / 2;
        int cy1 = rooms[i].y + rooms[i].h / 2;
        int cx2 = rooms[i + 1].x + rooms[i + 1].w / 2;
        int cy2 = rooms[i + 1].y + rooms[i + 1].h / 2;

        // Копаем по X от cx1 до cx2 на высоте cy1
        int stepX = (cx1 < cx2) ? 1 : -1;
        for (int x = cx1; x != cx2; x += stepX) {
            map[cy1][x] = 0;
        }

        // Копаем по Y от cy1 до cy2 на высоте cx2
        int stepY = (cy1 < cy2) ? 1 : -1;
        for (int y = cy1; y != cy2; y += stepY) {
            map[y][cx2] = 0;
        }
    }

}

int ScreenX(float worldX) { return (int)((worldX - camera.x) * camera.ZOOM); }
int ScreenY(float worldY) { return (int)((worldY - camera.y) * camera.ZOOM); }

void ShowSprite(int x, int y, int w, int h, HBITMAP hBitmap, bool transparent) {

    if (!hBitmap) return;

    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);

    HDC memDC = CreateCompatibleDC(window.mem_dc);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

    SetStretchBltMode(window.mem_dc, COLORONCOLOR);

    if (transparent) TransparentBlt(window.mem_dc, x, y, w, h, memDC, 0, 0, bm.bmWidth, bm.bmHeight, RGB(0, 0, 0));
    else StretchBlt(window.mem_dc, x, y, w, h, memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    SelectObject(memDC, hOldBitmap);
    DeleteDC(memDC);

}

void UpdateCamera() {

    const float viewW = window.width / camera.ZOOM;
    const float viewH = window.height / camera.ZOOM;

    camera.targetX = Hero.x + Hero.widht / 2.0f - viewW / 2.0f;
    camera.targetY = Hero.y + Hero.height / 2.0f - viewH / 2.0f;
    
    camera.x += (camera.targetX - camera.x) * camera.smooth;
    camera.y += (camera.targetY - camera.y) * camera.smooth;

    const float MAP_H_px = MAP_H * TILE;
    const float MAP_W_px = MAP_W * TILE;

    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x > MAP_W_px - viewW) camera.x = MAP_W_px - viewW;
    if (camera.y > MAP_H_px - viewH) camera.y = MAP_H_px - viewH;

}

void ShowMap() {

    RECT fullRect = { 0, 0, window.width, window.height };
    HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 20));
    FillRect(window.mem_dc, &fullRect, bgBrush);
    DeleteObject(bgBrush);

    float viewW = (float)window.width / camera.ZOOM;
    float viewH = (float)window.height / camera.ZOOM;

    int startCol = max(0, (int)(camera.x / TILE));
    int endCol = min(MAP_W, (int)((camera.x + viewW) / TILE) + 1);
    int startRow = max(0, (int)(camera.y / TILE));
    int endRow = min(MAP_H, (int)((camera.y + viewH) / TILE) + 1);

    int size = TILE * camera.ZOOM; // Размер тайла на экране

    for (int ty = startRow; ty < endRow; ++ty) {
        for (int tx = startCol; tx < endCol; ++tx) {

            int sx = ScreenX(tx * TILE);
            int sy = ScreenY(ty * TILE);

            // Выбираем текстуру в зависимости от значения в массиве
            HBITMAP tex = (map[ty][tx] == 1) ? hTexWall : hTexGrass;

            // Рисуем тайл (false = без прозрачности, StretchBlt растянет 32x32 до 64x64 при ZOOM=2)
            ShowSprite(sx, sy, size, size, tex, false);
        }
    }
}

void DrawDebugHitbox(float worldX, float worldY, float w, float h, COLORREF color) {
    int sx = ScreenX(worldX);
    int sy = ScreenY(worldY);
    int sw = (int)(w * camera.ZOOM);
    int sh = (int)(h * camera.ZOOM);

    HPEN pen = CreatePen(PS_SOLID, 2, color);
    HPEN oldPen = (HPEN)SelectObject(window.mem_dc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(window.mem_dc, GetStockObject(NULL_BRUSH));

    Rectangle(window.mem_dc, sx, sy, sx + sw, sy + sh);

    SelectObject(window.mem_dc, oldPen);
    SelectObject(window.mem_dc, oldBrush);
    DeleteObject(pen);
}

void DrawGrid() {
    if (!showGrid) return;

    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // Чёрные линии
    HPEN oldPen = (HPEN)SelectObject(window.mem_dc, pen);


    float viewW = (float)window.width / camera.ZOOM;
    float viewH = (float)window.height / camera.ZOOM;


    int startCol = max(0, (int)(camera.x / TILE));
    int endCol = min(MAP_W, (int)((camera.x + viewW) / TILE) + 1);

    int startRow = max(0, (int)(camera.y / TILE));
    int endRow = min(MAP_H, (int)((camera.y + viewH) / TILE) + 1);
    
    // Вертикальные линии
    for (int tx = startCol; tx <= endCol; ++tx) {
        int sx = ScreenX(tx * TILE);
        MoveToEx(window.mem_dc, sx, 0, NULL);
        LineTo(window.mem_dc, sx, window.height);
    }

    // Горизонтальные линии
    for (int ty = startRow; ty <= endRow; ++ty) {
        int sy = ScreenY(ty * TILE);
        MoveToEx(window.mem_dc, 0, sy, NULL);
        LineTo(window.mem_dc, window.width, sy);
    }

    SelectObject(window.mem_dc, oldPen);
    DeleteObject(pen);

}

void ShowObject() {

    ShowMap();
    ShowSprite(ScreenX(Hero.x), ScreenY(Hero.y), Hero.widht * camera.ZOOM, Hero.height * camera.ZOOM, Hero.hBitmap, true);
    DrawGrid();

    //DrawDebugHitbox(Hero.x, Hero.y, Hero.widht, Hero.height, RGB(255, 0, 0));  

}


void ShowGame() {

    window.mem_dc = CreateCompatibleDC(window.hdc);
    HBITMAP hMemBmp = CreateCompatibleBitmap(window.hdc, window.width, window.height);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(window.mem_dc, hMemBmp);

    ShowObject();
    BitBlt(window.hdc, 0, 0, window.width, window.height, window.mem_dc, 0, 0, SRCCOPY);

    SelectObject(window.mem_dc, hOldBmp);
    DeleteObject(hOldBmp);
    DeleteDC(window.mem_dc);
}

bool IsWallAt(float worldX, float worldY) {

    int tx = (int)(worldX / TILE);
    int ty = (int)(worldY / TILE);
    
    if (tx < 0 || tx >= MAP_W || ty < 0 || ty >= MAP_H) return true;
    
    return map[ty][tx] == 1; 

}

void MoveHero(float dx, float dy) {

    float newX = Hero.x + dx; 

    if (!IsWallAt(newX, Hero.y) &&
        !IsWallAt(newX + Hero.widht, Hero.y) &&
        !IsWallAt(newX, Hero.y + Hero.height) &&
        !IsWallAt(newX + Hero.widht, Hero.y + Hero.height))
    {

        Hero.x = newX;

    }

    float newY = Hero.y + dy;

    if (!IsWallAt(Hero.x, newY) &&
        !IsWallAt(Hero.x + Hero.widht, newY) &&
        !IsWallAt(Hero.x, newY + Hero.height) &&
        !IsWallAt(Hero.x + Hero.widht, newY + Hero.height))
    {

        Hero.y = newY;

    }
}

void ProcessInput() {

    if (keys['G']) {
        showGrid = !showGrid;
        keys['G'] = false; 
    }

    float step = Hero.speed * 0.15f;
    float dx = 0, dy = 0;

    if (keys[VK_ESCAPE]) DestroyWindow(window.hwnd); // уничтожаем окно
    if (keys['W']) dy -= step;
    if (keys['A']) dx -= step;
    if (keys['S']) dy += step;
    if (keys['D']) dx += step;
    
    if (dx != 0 && dy != 0) {
        dx *= 0.7071f;
        dy *= 0.7071f;
    }

    MoveHero(dx, dy);
}

void ProcessGame() {

    ProcessInput();
    UpdateCamera();
    

}

void ClearGame() {

    DeleteObject(Hero.hBitmap);
    DeleteObject(hTexWall);
    DeleteObject(hTexGrass);


}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); 

int APIENTRY wWinMain( 
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow  
) 
{ 
    const wchar_t* CLASS_NAME = L"Main";  
    WNDCLASSEX wc = {}; 
    
    wc.cbSize = sizeof(WNDCLASSEX); 
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance; 
    wc.hCursor = NULL; 
    wc.lpszClassName = CLASS_NAME;  

    if (!RegisterClassEx(&wc)) { 
        MessageBox(NULL, L"Ошибка регистрации класса окна!", L"Ошибка", MB_ICONERROR);
        return 0;
    }

    window.width = GetSystemMetrics(SM_CXSCREEN); 
    window.height = GetSystemMetrics(SM_CYSCREEN);

    window.hwnd = CreateWindowEx
    (
        0, 
        CLASS_NAME, 
        L"Roguelike prototype", 
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        window.width, window.height, 
        NULL, NULL, hInstance, NULL
    );

    if (!window.hwnd) {

        MessageBox(
            NULL, 
            L"Ошибка создания окна!", 
            L"Ошибка", 
            MB_ICONERROR 
        );
        return 0;  

    }

    InitWindow();

    ShowWindow(window.hwnd, SW_SHOW);
    UpdateWindow(window.hwnd);

    MSG msg = {}; 

    while (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg); 
        DispatchMessage(&msg); 

    }
}

LRESULT CALLBACK WndProc(

    HWND hwnd, 
    UINT msg, 
    WPARAM wParam, 
    LPARAM lParam  

) {

    switch (msg) {

    case WM_CREATE:

        InitGame();
        ShowCursor(FALSE); 
        SetTimer(hwnd, 1, 16, NULL);

        break;

    case WM_PAINT: 
    {   
        PAINTSTRUCT ps;
        window.hdc = BeginPaint(hwnd, &ps);
        ShowGame();
        EndPaint(hwnd, &ps);

    }

        break;

    case WM_TIMER: 
    {
        ProcessGame();
        InvalidateRect(hwnd, NULL, FALSE);
    }

        break;

    case WM_KEYDOWN: 

        keys[wParam] = true;
        InvalidateRect(hwnd, NULL, TRUE);

        break;
    case WM_KEYUP:
        keys[wParam] = false;
        break;

    case WM_DESTROY: 

        ClearGame();
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        break;


    default: 
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
