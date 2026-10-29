#include <windows.h>
#include <algorithm>
#include <wingdi.h> 
#include <vector>
#include <iostream>
#include <cmath>
#pragma comment(lib, "Msimg32.lib")

using namespace std;

// enemy
const int MAX_ENTITIES = 1000;
bool active[MAX_ENTITIES] = {};

// flags
bool hasPosition[MAX_ENTITIES] = {};
bool hasVelocity[MAX_ENTITIES] = {};
bool hasRenderable[MAX_ENTITIES] = {};
bool hasCollider[MAX_ENTITIES] = {};

// components
struct Position { float x, y; };
Position positions[MAX_ENTITIES];

struct Velocity { float dx, dy; };
Velocity velocities[MAX_ENTITIES];

struct Renderable { int w, h; HBITMAP bmp; };
Renderable renderables[MAX_ENTITIES];

struct Collider { float w, h; };
Collider colliders[MAX_ENTITIES];

uint32_t HeroID = 0;

const int ZOOM = 4;
const int TILE = 128; // tiles in world px
const int MAP_W = 64;
const int MAP_H = 64;

struct { 
    HWND hwnd;
    HDC hdc, mem_dc;
    int width, height;

} window;

struct sprite {
    float x, y, w, h, speed;
    float HitboxW, HitboxH;
    HBITMAP hBitmap;

};

struct Room {
    int x, y; // in tiles
    int w, h; 
};

struct {
    float x, y;              
    float targetX, targetY;  
    float smooth = 0.30f;    

} camera;

sprite Hero;
 
int map[MAP_H][MAP_W];
bool keys[256] = {};

bool showGrid = false;
HBITMAP hTexWall = NULL;
HBITMAP hTexGrass = NULL;
HBITMAP hTexHero = NULL;

vector<Room> rooms;

int ScreenX(float worldX) { return (int)((worldX - camera.x) * ZOOM); }
int ScreenY(float worldY) { return (int)((worldY - camera.y) * ZOOM); }

uint32_t CreateEntity() {
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id]) {
            active[id] = true;
            return id; // Нашли свободный ID, возвращаем
        }
    }
    return -1; // Мест нет (на 1000 сущностей в прототипе хватит с головой)
}

void InitWindow() {

    RECT r;
    GetClientRect(window.hwnd, &r);

    window.width = r.right - r.left;
    window.height = r.bottom - r.top;

} 

void LoadBMP() {

    hTexWall = (HBITMAP)LoadImageW(NULL, L"wall_tile128px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexGrass = (HBITMAP)LoadImageW(NULL, L"grass_tile128px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHero = (HBITMAP)LoadImageW(NULL, L"fiol_girl64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    if (!hTexHero) {
        MessageBoxW(NULL, L"Спрайт не загружен! Герой остался без кожи ((", L"Ошибка", MB_ICONERROR);
    }

    if (!hTexWall) {
        MessageBoxW(NULL, L"Текстуры не загрузились!  --> wall.bmp", L"Ошибка", MB_ICONERROR);
    }

    if (!hTexGrass) {
        MessageBoxW(NULL, L"Текстуры не загрузились!  --> grass.bmp", L"Ошибка", MB_ICONERROR);
    }

}

void GenerateMap() {

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map[y][x] = 1;
        }
    }

    for (int i = 0; i < 15; i++) { 
        int w = 4 + rand() % 6;
        int h = 4 + rand() % 6;
        int x = 1 + rand() % (MAP_W - w - 2); 
        int y = 1 + rand() % (MAP_H - h - 2);

        bool intersects = false;
        for (const Room& r : rooms) {

            if (x < r.x + r.w + 1 && x + w + 1 > r.x &&
                y < r.y + r.h + 1 && y + h + 1 > r.y) {
                intersects = true;
                break;
            }
        }

        if (!intersects) {

            for (int ty = y; ty < y + h; ty++) {
                for (int tx = x; tx < x + w; tx++) {
                    map[ty][tx] = 0;
                }
            }
            rooms.push_back({ x, y, w, h });
        }
    }

    for (int i = 0; i < rooms.size() - 1; i++) {
        int cx1 = rooms[i].x + rooms[i].w / 2;
        int cy1 = rooms[i].y + rooms[i].h / 2;
        int cx2 = rooms[i + 1].x + rooms[i + 1].w / 2;
        int cy2 = rooms[i + 1].y + rooms[i + 1].h / 2;

        int stepX = (cx1 < cx2) ? 1 : -1;
        for (int x = cx1; x != cx2; x += stepX) {
            map[cy1][x] = 0;
        }

        int stepY = (cy1 < cy2) ? 1 : -1;
        for (int y = cy1; y != cy2; y += stepY) {
            map[y][cx2] = 0;
        }
    }


}

void EntityInit() {

    HeroID = CreateEntity();

    Room firstRoom = rooms[0];
    float startX = (firstRoom.x + firstRoom.w / 2.0f) * TILE;
    float startY = (firstRoom.y + firstRoom.h / 2.0f) * TILE;

    // position
    hasPosition[HeroID] = true;
    positions[HeroID] = { startX, startY };

    // speed
    hasVelocity[HeroID] = true;
    velocities[HeroID] = { 0, 0 };

    // render
    hasRenderable[HeroID] = true;
    renderables[HeroID] = { 64, 64, hTexHero };

    // Collider (hitbox)
    hasCollider[HeroID] = true;
    colliders[HeroID] = { 20, 50 }; // ширина, высота

    //Hero.x = (firstRoom.x + firstRoom.w / 2.0f) * TILE;
    //Hero.y = (firstRoom.y + firstRoom.h / 2.0f) * TILE;
    //Hero.w = 64;
    //Hero.h = 64;
    //Hero.HitboxW = 20;
    //Hero.HitboxH = 50;
    //Hero.speed = 60.0f;

}

void InitGame() {

    srand(time(nullptr));
    LoadBMP();
    GenerateMap();
    EntityInit();

}


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
    if (!hasPosition[HeroID]) return;

    const float viewW = window.width / ZOOM;
    const float viewH = window.height / ZOOM;

    // Берем позицию из компонента (это центр героя)
    camera.targetX = positions[HeroID].x - viewW / 2.0f;
    camera.targetY = positions[HeroID].y - viewH / 2.0f;

    camera.x += (camera.targetX - camera.x) * camera.smooth;
    camera.y += (camera.targetY - camera.y) * camera.smooth;

    // Клэмп камеры
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

    float viewW = (float)window.width / ZOOM;
    float viewH = (float)window.height / ZOOM;

    int startCol = max(0, (int)(camera.x / TILE));
    int endCol = min(MAP_W, (int)((camera.x + viewW) / TILE) + 1);
    int startRow = max(0, (int)(camera.y / TILE));
    int endRow = min(MAP_H, (int)((camera.y + viewH) / TILE) + 1);

    int size = TILE * ZOOM; // Размер тайла на экране

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

    if (!showGrid) return;
    int sx = ScreenX(worldX);
    int sy = ScreenY(worldY);
    int sw = (int)(w * ZOOM);
    int sh = (int)(h * ZOOM);

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


    float viewW = (float)window.width / ZOOM;
    float viewH = (float)window.height / ZOOM;


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

//void ShowObject() {
//
//    ShowMap();
//    DrawGrid();
//
//    int drawX = ScreenX(Hero.x) - (Hero.w * ZOOM) / 2;
//    int drawY = ScreenY(Hero.y) - (Hero.h * ZOOM) / 2;
//
//    ShowSprite(drawX, drawY, Hero.w * ZOOM, Hero.h * ZOOM, Hero.hBitmap, true);
//
//    float hx = Hero.x - Hero.HitboxW / 2.0f;
//    float hy = Hero.y - Hero.HitboxH / 2.0f;
//    DrawDebugHitbox(hx, hy, Hero.HitboxW, Hero.HitboxH, RGB(255, 0, 0));
//
//}

void SystemRender() {
    ShowMap();
    DrawGrid();

    // Перебираем все сущности
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id]) continue;
        if (!hasPosition[id] || !hasRenderable[id]) continue;

        // Берем данные из компонентов
        float x = positions[id].x;
        float y = positions[id].y;
        int w = renderables[id].w;
        int h = renderables[id].h;
        HBITMAP bmp = renderables[id].bmp;

        // Рисуем центрированно
        int drawX = ScreenX(x) - (w * ZOOM) / 2;
        int drawY = ScreenY(y) - (h * ZOOM) / 2;
        ShowSprite(drawX, drawY, w * ZOOM, h * ZOOM, bmp, true);

        // Отрисовка хитбокса (если включен дебаг)
        if (showGrid && hasCollider[id]) {
            float hw = colliders[id].w;
            float hh = colliders[id].h;
            float hx = x - hw / 2.0f;
            float hy = y - hh / 2.0f;
            DrawDebugHitbox(hx, hy, hw, hh, RGB(255, 0, 0));
        }
    }
}

void ShowGame() {

    window.mem_dc = CreateCompatibleDC(window.hdc);
    HBITMAP hMemBmp = CreateCompatibleBitmap(window.hdc, window.width, window.height);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(window.mem_dc, hMemBmp);

    //ShowObject();
    SystemRender();
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

void SystemMovement() {

    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id]) continue;
        if (!hasPosition[id] || !hasVelocity[id]) continue;

        float dx = velocities[id].dx;
        float dy = velocities[id].dy;

        // Если скорость нулевая — нечего считать
        if (dx == 0 && dy == 0) continue;

        float newX = positions[id].x + dx;
        float newY = positions[id].y + dy;

        // Коллизии (пока только для Героя)
        if (id == HeroID && hasCollider[id]) {
            float hw = colliders[id].w / 2.0f;
            float hh = colliders[id].h / 2.0f;

            // Проверка по X
            float newHx = newX - hw;
            float hy = positions[id].y - hh;
            if (!IsWallAt(newHx, hy) &&
                !IsWallAt(newHx + colliders[id].w, hy) &&
                !IsWallAt(newHx, hy + colliders[id].h) &&
                !IsWallAt(newHx + colliders[id].w, hy + colliders[id].h)) {
                positions[id].x = newX;
            }

            // Проверка по Y
            float hx = positions[id].x - hw;
            float newHy = newY - hh;
            if (!IsWallAt(hx, newHy) &&
                !IsWallAt(hx + colliders[id].w, newHy) &&
                !IsWallAt(hx, newHy + colliders[id].h) &&
                !IsWallAt(hx + colliders[id].w, newHy + colliders[id].h)) {
                positions[id].y = newY;
            }
        }
        else {
            // Для всех остальных (враги, пули) — просто двигаем без коллизий
            positions[id].x = newX;
            positions[id].y = newY;
        }
    }
}

void MoveHero(float dx, float dy) {

    float hx = Hero.x - Hero.HitboxW / 2.0f;
    float hy = Hero.y - Hero.HitboxH / 2.0f;
    float newX = Hero.x + dx; 
    float newHx = newX - Hero.HitboxW / 2.0f;

    if (!IsWallAt(newHx, hy) &&
        !IsWallAt(newHx + Hero.HitboxW, hy) &&
        !IsWallAt(newHx, hy + Hero.HitboxH) &&
        !IsWallAt(newHx + Hero.HitboxW, hy + Hero.HitboxH)) 
    {
        Hero.x = newX;
    }

    float newY = Hero.y + dy;
    float newHy = newY - Hero.HitboxH / 2.0f;
    if (!IsWallAt(hx, newHy) &&
        !IsWallAt(hx + Hero.HitboxW, newHy) &&
        !IsWallAt(hx, newHy + Hero.HitboxH) &&
        !IsWallAt(hx + Hero.HitboxW, newHy + Hero.HitboxH)) 
    {
        Hero.y = newY;
    }
}

void SystemInput() {

    if (keys['G']) {
        showGrid = !showGrid;
        keys['G'] = false;
    }

    if (keys[VK_ESCAPE]) DestroyWindow(window.hwnd); // уничтожаем окно

    if (!hasVelocity[HeroID]) return;

    float speed = 60.0f * 0.15f; // Наша старая скорость
    float dx = 0, dy = 0;

    if (keys['W']) dy -= speed;
    if (keys['S']) dy += speed;
    if (keys['A']) dx -= speed;
    if (keys['D']) dx += speed;

    if (dx != 0 && dy != 0) {
        dx *= 0.7071f;
        dy *= 0.7071f;
    }

    // Записываем скорость в компонент!
    velocities[HeroID].dx = dx;
    velocities[HeroID].dy = dy;

}

//void ProcessInput() {
//
//    if (keys['G']) {
//        showGrid = !showGrid;
//        keys['G'] = false; 
//    }
//
//    float step = Hero.speed * 0.15f;
//    float dx = 0, dy = 0;
//
//    if (keys[VK_ESCAPE]) DestroyWindow(window.hwnd); // уничтожаем окно
//    if (keys['W']) dy -= step;
//    if (keys['A']) dx -= step;
//    if (keys['S']) dy += step;
//    if (keys['D']) dx += step;
//    
//    if (dx != 0 && dy != 0) {
//        dx *= 0.7071f;
//        dy *= 0.7071f;
//    }
//
//    MoveHero(dx, dy);
//}

void ProcessGame() {
    SystemInput();
    SystemMovement();
    UpdateCamera();
}

void ClearGame() {

    DeleteObject(hTexHero);
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
