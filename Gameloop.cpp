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

struct AI { int type; float speed; float aggroRadius;  int state; float timer; };
AI ais[MAX_ENTITIES];
bool hasAI[MAX_ENTITIES] = {};


// object
uint32_t HeroID = 0;

float ZOOM = 4.0f;
const int TILE = 128; // tiles in world px
const int MAP_W = 64;
const int MAP_H = 64;

struct { 
    HWND hwnd;
    HDC hdc, mem_dc;
    int width, height;

} window;

struct Room {
    int x, y; // in tiles
    int w, h; 
};

struct {
    float x, y;              
    float targetX, targetY;  
    float smooth = 0.30f;    

} camera;
 
int map[MAP_H][MAP_W];
bool keys[256] = {};

bool showGrid = false;
HBITMAP hTexWall = NULL;
HBITMAP hTexGrass = NULL;
HBITMAP hTexHero = NULL;
HBITMAP hTexSpider = NULL;

vector<Room> rooms;

int ScreenX(float worldX) { return (int)((worldX - camera.x) * ZOOM); }
int ScreenY(float worldY) { return (int)((worldY - camera.y) * ZOOM); }

uint32_t CreateEntity() {
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id]) {
            active[id] = true;
            return id; // free id return
        }
    }
    return -1; 
}

void InitWindow() {

    RECT r;
    GetClientRect(window.hwnd, &r);

    window.width = r.right - r.left;
    window.height = r.bottom - r.top;

} 

void LoadBMP() {

    hTexWall = (HBITMAP)LoadImageW(NULL, L"wall_tile64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexGrass = (HBITMAP)LoadImageW(NULL, L"grass_tile64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHero = (HBITMAP)LoadImageW(NULL, L"fiol_girl64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexSpider = (HBITMAP)LoadImageW(NULL, L"spider64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    if (!hTexHero) {
        MessageBoxW(NULL, L"Спрайт не загружен! Герой остался без кожи ((", L"Ошибка", MB_ICONERROR);
    }

    if (!hTexWall) {
        MessageBoxW(NULL, L"Текстуры не загрузились!  --> wall.bmp", L"Ошибка", MB_ICONERROR);
    }

    if (!hTexGrass) {
        MessageBoxW(NULL, L"Текстуры не загрузились!  --> grass.bmp", L"Ошибка", MB_ICONERROR);
    }

    if (!hTexSpider) {
        MessageBoxW(NULL, L"Спрайт паука не загружен!", L"Ошибка", MB_ICONERROR);
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

uint32_t SpawnEntity(float x, float y, int w, int h, HBITMAP hBitmap, float HitBoxW, float HitBoxH) {
    uint32_t id = CreateEntity();

    hasVelocity[id] = true;
    velocities[id] = { 0, 0 };

    hasPosition[id] = true;
    positions[id] = { x, y };

    hasRenderable[id] = true;
    renderables[id] = { w, h, hBitmap };

    hasCollider[id] = true;
    colliders[id] = { HitBoxW, HitBoxH };

    return id; 
}

void SpawnEntityInRooms() {

    for (int i = 1; i < rooms.size(); i++) { 
        int enemiesCount = 1 + rand() % 3; 

        for (int j = 0; j < enemiesCount; j++) {

            int tx = rooms[i].x + (rand() % rooms[i].w);
            int ty = rooms[i].y + (rand() % rooms[i].h);

            float spawnX = (tx + 0.5f) * TILE;
            float spawnY = (ty + 0.5f) * TILE;

            uint32_t enemyID = SpawnEntity(spawnX, spawnY, 64, 64, hTexSpider, 40, 40);
            hasAI[enemyID] = true;
            ais[enemyID] = { 1, 5.0f, 500.0f, 0, 0.0f };
        }
    }
}

void EntityInit() {
    Room firstRoom = rooms[0];
    float startX = (firstRoom.x + firstRoom.w / 2.0f) * TILE;
    float startY = (firstRoom.y + firstRoom.h / 2.0f) * TILE;

    HeroID = SpawnEntity(startX, startY, 64, 64, hTexHero, 20, 50);
    
    SpawnEntityInRooms();
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

    camera.targetX = positions[HeroID].x - viewW / 2.0f;
    camera.targetY = positions[HeroID].y - viewH / 2.0f;

    camera.x += (camera.targetX - camera.x) * camera.smooth;
    camera.y += (camera.targetY - camera.y) * camera.smooth;

    // clamp
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

    int size = (int)TILE * ZOOM; 

    for (int ty = startRow; ty < endRow; ++ty) {
        for (int tx = startCol; tx < endCol; ++tx) {

            int sx = ScreenX(tx * TILE);
            int sy = ScreenY(ty * TILE);

            HBITMAP tex = (map[ty][tx] == 1) ? hTexWall : hTexGrass;

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

    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); 
    HPEN oldPen = (HPEN)SelectObject(window.mem_dc, pen);


    float viewW = (float)window.width / ZOOM;
    float viewH = (float)window.height / ZOOM;


    int startCol = max(0, (int)(camera.x / TILE));
    int endCol = min(MAP_W, (int)((camera.x + viewW) / TILE) + 1);

    int startRow = max(0, (int)(camera.y / TILE));
    int endRow = min(MAP_H, (int)((camera.y + viewH) / TILE) + 1);
    
    for (int tx = startCol; tx <= endCol; ++tx) {
        int sx = ScreenX(tx * TILE);
        MoveToEx(window.mem_dc, sx, 0, NULL);
        LineTo(window.mem_dc, sx, window.height);
    }

    for (int ty = startRow; ty <= endRow; ++ty) {
        int sy = ScreenY(ty * TILE);
        MoveToEx(window.mem_dc, 0, sy, NULL);
        LineTo(window.mem_dc, window.width, sy);
    }

    SelectObject(window.mem_dc, oldPen);
    DeleteObject(pen);

}

void SystemRender() {
    ShowMap();
    DrawGrid();

    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id]) continue;
        if (!hasPosition[id] || !hasRenderable[id]) continue;

        float x = positions[id].x;
        float y = positions[id].y;
        int w = renderables[id].w;
        int h = renderables[id].h;
        HBITMAP bmp = renderables[id].bmp;

        int drawX = ScreenX(x) - (int)(w * ZOOM) / 2;
        int drawY = ScreenY(y) - (int)(h * ZOOM) / 2;
        ShowSprite(drawX, drawY, (int)(w * ZOOM), (int)(h * ZOOM), bmp, true);

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

        if (dx == 0 && dy == 0) continue;

        float newX = positions[id].x + dx;
        float newY = positions[id].y + dy;

        if (hasCollider[id]) {
            float hw = colliders[id].w / 2.0f;
            float hh = colliders[id].h / 2.0f;

            float newHx = newX - hw;
            float hy = positions[id].y - hh;
            if (!IsWallAt(newHx, hy) &&
                !IsWallAt(newHx + colliders[id].w, hy) &&
                !IsWallAt(newHx, hy + colliders[id].h) &&
                !IsWallAt(newHx + colliders[id].w, hy + colliders[id].h)) {
                positions[id].x = newX;
            }

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
            positions[id].x = newX;
            positions[id].y = newY;
        }
    }
}

void SystemInput() {

    if (keys['G']) {
        showGrid = !showGrid;
        keys['G'] = false;
    }

    if (keys[VK_ESCAPE]) DestroyWindow(window.hwnd); 

    if (!hasVelocity[HeroID]) return;

    float speed = 60.0f * 0.15f; 
    float dx = 0, dy = 0;

    if (keys['W']) dy -= speed;
    if (keys['S']) dy += speed;
    if (keys['A']) dx -= speed;
    if (keys['D']) dx += speed;

    if (dx != 0 && dy != 0) {
        dx *= 0.7071f;
        dy *= 0.7071f;
    }

    velocities[HeroID].dx = dx;
    velocities[HeroID].dy = dy;

}

bool IsWallBetween(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dist = sqrt(dx * dx + dy * dy);
    if (dist < 1.0f) return false;

    int steps = (int)(dist / 20.0f);
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        float px = x1 + dx * t;
        float py = y1 + dy * t;
        if (IsWallAt(px, py)) return true;
    }
    return false;
}

void SystemAI() {
    if (!hasPosition[HeroID]) return;

    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasAI[id] || !hasPosition[id] || !hasVelocity[id]) continue;

        if (ais[id].type == 1) { 
            float dx = positions[HeroID].x - positions[id].x;
            float dy = positions[HeroID].y - positions[id].y;
            float length = sqrt(dx * dx + dy * dy);

            bool canSee = (length < ais[id].aggroRadius) && !IsWallBetween(positions[id].x, positions[id].y, positions[HeroID].x, positions[HeroID].y);

            if (canSee) {
                ais[id].state = 1; // persecution
                ais[id].timer = 180.0f; // 3 seconds
            }

            if (ais[id].state == 1) {
                float minDist = 25.0f;
                if (length > minDist) {
                    velocities[id].dx = (dx / length) * ais[id].speed;
                    velocities[id].dy = (dy / length) * ais[id].speed;
                }
                else {
                    velocities[id] = { 0, 0 };
                }

                ais[id].timer -= 1.0f;
                if (ais[id].timer <= 0) {
                    ais[id].state = 0; 
                    velocities[id] = { 0, 0 };
                }
            }
            else {
                velocities[id] = { 0, 0 }; // stop!
            }
        }
    }
}

void ProcessGame() {
    SystemInput();
    SystemAI();
    SystemMovement();
    UpdateCamera();
}

void ClearGame() {

    DeleteObject(hTexHero);
    DeleteObject(hTexWall);
    DeleteObject(hTexGrass);
    DeleteObject(hTexSpider);

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

    case WM_MOUSEWHEEL:
    {
        short delta = GET_WHEEL_DELTA_WPARAM(wParam);

        ZOOM += delta / 120.0f * 0.2f;

        if (ZOOM < 1.0f) ZOOM = 1.0f;
        if (ZOOM > 10.0f) ZOOM = 10.0f;

        return 0;
    }
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
