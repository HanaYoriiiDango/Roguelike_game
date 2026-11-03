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

struct Collider { float w, h; float offsetX, offsetY; };
Collider colliders[MAX_ENTITIES];

struct AI { int type; float speed; float aggroRadius;  int state; float timer; float attackTimer; };
AI ais[MAX_ENTITIES];
bool hasAI[MAX_ENTITIES] = {};

struct Health { float hp; float maxHp; };
Health healths[MAX_ENTITIES];
bool hasHealth[MAX_ENTITIES] = {};

struct Weapon { float damage; float cooldown; float timer; };
Weapon weapons[MAX_ENTITIES];
bool hasWeapon[MAX_ENTITIES] = {};

struct Direction {
    int facing; // 0 = Down, 1 = Up, 2 = Left, 3 = Right
};
Direction directions[MAX_ENTITIES];
bool hasDirection[MAX_ENTITIES] = {};

struct Animation {
    HBITMAP frames[8];   // До 8 кадров
    int frameCount;      // Сколько кадров реально
    int currentFrame;    // Текущий кадр
    float frameTimer;    // Таймер
    float frameDuration; // Задержка (в кадрах игры)
    int state;           // 0 = idle, 1 = walk, 2 = attack
};
Animation animations[MAX_ENTITIES];
bool hasAnimation[MAX_ENTITIES] = {};

// object
uint32_t HeroID = 0;

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

// SUPPORT ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
float ZOOM = 4.0f;
const int TILE = 128; // tiles in world px
const int MAP_W = 64;
const int MAP_H = 64;

const float HERO_SPEED = 60.0f;
const float HERO_ATTACK_RADIUS = 150.0f;
const float HERO_DAMAGE = 20.0f;
const float HERO_ATTACK_COOLDOWN = 30.0f;

const float SPIDER_SPEED = 5.0f;
const float SPIDER_HP = 30.0f;
const float SPIDER_DAMAGE = 5.0f;
const float SPIDER_ATTACK_COOLDOWN = 30.0f;
const float SPIDER_AGGRO_RADIUS = 500.0f;

int map[MAP_H][MAP_W];
bool keys[256] = {};

bool showGrid = false;
bool mouseLeftDown = false;

// animations sprites
HBITMAP hTexWall = NULL;
HBITMAP hTexGrass = NULL;
HBITMAP hTexHero = NULL;
HBITMAP hTexSpider = NULL;

HBITMAP hTexHeroRightIdle = NULL;
HBITMAP hTexHeroRightWalk[8] = { NULL }; 

HBITMAP hTexHeroLeftIdle = NULL;
HBITMAP hTexHeroLeftWalk[8] = { NULL };

HBITMAP hTexHeroUpIdle = NULL;
HBITMAP hTexHeroUpWalk[8] = { NULL };

HBITMAP hTexHeroDownIdle = NULL;
HBITMAP hTexHeroDownWalk[8] = { NULL };

HBITMAP hTexHeroDownAttack[4] = { NULL };
HBITMAP hTexHeroUpAttack[4] = { NULL };
HBITMAP hTexHeroLeftAttack[4] = { NULL };
HBITMAP hTexHeroRightAttack[4] = { NULL };


vector<Room> rooms;

int ScreenX(float worldX) { return (int)((worldX - camera.x) * ZOOM); }
int ScreenY(float worldY) { return (int)((worldY - camera.y) * ZOOM); }

bool IsWallAt(float worldX, float worldY) {

    int tx = (int)(worldX / TILE);
    int ty = (int)(worldY / TILE);
    
    if (tx < 0 || tx >= MAP_W || ty < 0 || ty >= MAP_H) return true;
    
    return map[ty][tx] == 1; 

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

// Расстояние между центрами спрайтов (для ИИ)
float DistanceBetweenSprites(uint32_t a, uint32_t b) {
    float dx = positions[a].x - positions[b].x;
    float dy = positions[a].y - positions[b].y;
    return sqrt(dx * dx + dy * dy);
}

// Расстояние между центрами хитбоксов (для боя)
float DistanceBetweenHitboxes(uint32_t a, uint32_t b) {
    float ax = positions[a].x + (hasCollider[a] ? colliders[a].offsetX : 0.0f);
    float ay = positions[a].y + (hasCollider[a] ? colliders[a].offsetY : 0.0f);
    float bx = positions[b].x + (hasCollider[b] ? colliders[b].offsetX : 0.0f);
    float by = positions[b].y + (hasCollider[b] ? colliders[b].offsetY : 0.0f);
    float dx = ax - bx;
    float dy = ay - by;
    return sqrt(dx * dx + dy * dy);
}

// INITIALIZATION ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

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

    hTexHeroRightIdle = (HBITMAP)LoadImageW(NULL, L"hero_right_idle.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hTexHeroRightWalk[0] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[1] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[2] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[3] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[4] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[5] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk6.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[6] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk7.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hTexHeroLeftIdle = (HBITMAP)LoadImageW(NULL, L"hero_left_idle.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hTexHeroLeftWalk[0] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[1] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[2] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[3] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[4] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[5] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk6.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[6] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk7.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hTexHeroUpIdle = (HBITMAP)LoadImageW(NULL, L"hero_up_idle.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hTexHeroUpWalk[0] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[1] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[2] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[3] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[4] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[5] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk6.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[6] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk7.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hTexHeroDownIdle = (HBITMAP)LoadImageW(NULL, L"hero_down_idle.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hTexHeroDownWalk[0] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[1] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[2] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[3] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[4] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[5] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk6.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[6] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk7.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hTexHeroDownAttack[0] = (HBITMAP)LoadImageW(NULL, L"down_attack1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownAttack[1] = (HBITMAP)LoadImageW(NULL, L"down_attack2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownAttack[2] = (HBITMAP)LoadImageW(NULL, L"down_attack3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownAttack[3] = (HBITMAP)LoadImageW(NULL, L"down_attack4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hTexHeroUpAttack[0] = (HBITMAP)LoadImageW(NULL, L"up_attack1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpAttack[1] = (HBITMAP)LoadImageW(NULL, L"up_attack2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpAttack[2] = (HBITMAP)LoadImageW(NULL, L"up_attack3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpAttack[3] = (HBITMAP)LoadImageW(NULL, L"up_attack4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hTexHeroLeftAttack[0] = (HBITMAP)LoadImageW(NULL, L"left_attack1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftAttack[1] = (HBITMAP)LoadImageW(NULL, L"left_attack2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftAttack[2] = (HBITMAP)LoadImageW(NULL, L"left_attack3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftAttack[3] = (HBITMAP)LoadImageW(NULL, L"left_attack4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    hTexHeroRightAttack[0] = (HBITMAP)LoadImageW(NULL, L"right_attack1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightAttack[1] = (HBITMAP)LoadImageW(NULL, L"right_attack2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightAttack[2] = (HBITMAP)LoadImageW(NULL, L"right_attack3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightAttack[3] = (HBITMAP)LoadImageW(NULL, L"right_attack4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    for (int i = 0; i < 7; i++) {
        if (!hTexHeroDownWalk[i]) {
            wchar_t msg[64];
            swprintf_s(msg, L"Failed to load up walk %d", i + 1);
            MessageBoxW(NULL, msg, L"LoadBMP Error", MB_ICONERROR);
        }
    }

    for (int i = 0; i < 7; i++) {
        if (!hTexHeroUpWalk[i]) {
            wchar_t msg[64];
            swprintf_s(msg, L"Failed to load up walk %d", i + 1);
            MessageBoxW(NULL, msg, L"LoadBMP Error", MB_ICONERROR);
        }
    }

    for (int i = 0; i < 7; i++) {
        if (!hTexHeroRightWalk[i]) {
            wchar_t msg[64];
            swprintf_s(msg, L"Failed to load right walk %d", i + 1);
            MessageBoxW(NULL, msg, L"LoadBMP Error", MB_ICONERROR);
        }
    }

    for (int i = 0; i < 7; i++) {
        if (!hTexHeroLeftWalk[i]) {
            wchar_t msg[64];
            swprintf_s(msg, L"Failed to load left walk %d", i + 1);
            MessageBoxW(NULL, msg, L"LoadBMP Error", MB_ICONERROR);
        }
    }

    if (!hTexHero) {
        MessageBoxW(NULL, L"The sprite is not loaded! The hero is left without skin ((", L"Error:LoadBMP", MB_ICONERROR);
    }

    if (!hTexWall) {
        MessageBoxW(NULL, L"The textures didn’t load! --> wall.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }

    if (!hTexGrass) {
        MessageBoxW(NULL, L"The textures didn’t load! --> grass.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }

    if (!hTexSpider) {
        MessageBoxW(NULL, L"The textures didn’t load! --> spider.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }
}

// ENTITY SPAWN ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

uint32_t CreateEntity() {
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id]) {
            active[id] = true;
            return id; // free id return
        }
    }
    return -1; 
}

uint32_t SpawnEntity(float x, float y, int w, int h, HBITMAP hBitmap, float HitBoxW, float HitBoxH, float offsetX = 0, float offsetY = 0) {
    uint32_t id = CreateEntity();

    hasVelocity[id] = true;
    velocities[id] = { 0, 0 };

    hasPosition[id] = true;
    positions[id] = { x, y };

    hasRenderable[id] = true;
    renderables[id] = { w, h, hBitmap };

    hasCollider[id] = true;
    colliders[id] = { HitBoxW, HitBoxH, offsetX, offsetY };

    return id; 
}


// GENERATION ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

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

void SpawnEntityInRooms() {

    for (int i = 1; i < rooms.size(); i++) { 
        int enemiesCount = 1 + rand() % 3; 

        for (int j = 0; j < enemiesCount; j++) {

            int tx = rooms[i].x + (rand() % rooms[i].w);
            int ty = rooms[i].y + (rand() % rooms[i].h);

            float spawnX = (tx + 0.5f) * TILE;
            float spawnY = (ty + 0.5f) * TILE;

            uint32_t enemyID = SpawnEntity(spawnX, spawnY, 64, 64, hTexSpider, 20, 20, 0, 5);
            hasAI[enemyID] = true;
            ais[enemyID] = { 1, SPIDER_SPEED * 0.15f, SPIDER_AGGRO_RADIUS, 0, 0.0f, 0.0f };
            hasHealth[enemyID] = true;
            healths[enemyID] = { SPIDER_HP, SPIDER_HP };
        }
    }
}

void EntityInit() {
    Room firstRoom = rooms[0];
    float startX = (firstRoom.x + firstRoom.w / 2.0f) * TILE;
    float startY = (firstRoom.y + firstRoom.h / 2.0f) * TILE;

    HeroID = SpawnEntity(startX, startY, 64, 64, hTexHero, 24, 24, 0, 15);
    hasHealth[HeroID] = true;
    healths[HeroID] = { 100.0f, 100.0f }; 
    hasWeapon[HeroID] = true;
    weapons[HeroID] = { HERO_DAMAGE, HERO_ATTACK_COOLDOWN, 0.0f };

    hasDirection[HeroID] = true;
    directions[HeroID].facing = 0; // Смотрит вниз по умолчанию

    hasAnimation[HeroID] = true;
    animations[HeroID].frameCount = 1;
    animations[HeroID].currentFrame = 0;
    animations[HeroID].frameTimer = 0.0f;
    animations[HeroID].frameDuration = 8.0f; // Скорость смены кадров
    animations[HeroID].state = 0; // 0 = idle
    animations[HeroID].frames[0] = hTexHero; // Пока дефолтный спрайт

    SpawnEntityInRooms();
} 

// RENDERING ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

void ShowSprite(int x, int y, int w, int h, HBITMAP hBitmap, bool transparent) {

    if (!hBitmap) return;

    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);

    HDC memDC = CreateCompatibleDC(window.mem_dc);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

    SetStretchBltMode(window.mem_dc, COLORONCOLOR);

    if (transparent) TransparentBlt(window.mem_dc, x, y, w, h, memDC, 0, 0, bm.bmWidth, bm.bmHeight, RGB(255, 255, 255));
    else StretchBlt(window.mem_dc, x, y, w, h, memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    SelectObject(memDC, hOldBitmap);
    DeleteDC(memDC);

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

void DrawEntity(uint32_t id) {
    if (!active[id] || !hasPosition[id]) return;

    // Выбираем картинку: анимация или обычный renderable
    HBITMAP bmp = NULL;
    if (hasAnimation[id] && animations[id].frameCount > 0) {
        bmp = animations[id].frames[animations[id].currentFrame];
    }
    else if (hasRenderable[id]) {
        bmp = renderables[id].bmp;
    }

    if (!bmp) return;

    int w = 64, h = 64; // Размер спрайта (у нас 64x64)
    if (hasRenderable[id]) {
        w = renderables[id].w;
        h = renderables[id].h;
    }

    float x = positions[id].x;
    float y = positions[id].y;

    int drawX = ScreenX(x) - (int)(w * ZOOM) / 2;
    int drawY = ScreenY(y) - (int)(h * ZOOM) / 2;
    ShowSprite(drawX, drawY, (int)(w * ZOOM), (int)(h * ZOOM), bmp, true);

    // Хитбокс для дебага
    if (showGrid && hasCollider[id]) {
        float hw = colliders[id].w;
        float hh = colliders[id].h;
        float hx = x + colliders[id].offsetX - hw / 2.0f;
        float hy = y + colliders[id].offsetY - hh / 2.0f;
        DrawDebugHitbox(hx, hy, hw, hh, RGB(255, 0, 0));
    }
}

void SystemRender() {
    ShowMap();
    DrawGrid();

    // Draw all except hero
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (id == HeroID) continue;
        DrawEntity(id);
    }

    // Draw hero on top
    DrawEntity(HeroID);

    // HUD
    wchar_t buf[64];
    swprintf_s(buf, L"HP: %.0f", healths[HeroID].hp);
    SetTextColor(window.mem_dc, RGB(255, 255, 255));
    SetBkMode(window.mem_dc, TRANSPARENT);
    TextOutW(window.mem_dc, 20, 20, buf, wcslen(buf));
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

// INPUT ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

void SystemInput() {

    if (keys['G']) {
        showGrid = !showGrid;
        keys['G'] = false;
    }

    if (keys[VK_ESCAPE]) DestroyWindow(window.hwnd);

    if (!hasVelocity[HeroID]) return;

    float speed = HERO_SPEED * 0.15f;
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

    // --- ANIMATION ---
    if (hasAnimation[HeroID]) {
        if (animations[HeroID].state == 2) {
        
        }
        else {

            bool isMoving = (dx != 0 || dy != 0);
            int targetState = isMoving ? 1 : 0;

            // Запоминаем старое направление
            int oldFacing = directions[HeroID].facing;

            // Определяем новое направление
            if (isMoving) {
                if (abs(dx) > abs(dy)) {
                    directions[HeroID].facing = (dx < 0) ? 2 : 3; // Left : Right
                }
                else {
                    directions[HeroID].facing = (dy < 0) ? 1 : 0; // Up : Down
                }
            }

            if (animations[HeroID].state != targetState || oldFacing != directions[HeroID].facing) {
                animations[HeroID].state = targetState;
                animations[HeroID].currentFrame = 0;
                animations[HeroID].frameTimer = 0.0f;

                // Настраиваем кадры в зависимости от направления и состояния
                if (directions[HeroID].facing == 1) { // up
                    if (targetState == 1) { // Walk
                        animations[HeroID].frameCount = 7;
                        for (int i = 0; i < 7; i++) {
                            animations[HeroID].frames[i] = hTexHeroUpWalk[i];
                        }
                    }
                    else { // Idle
                        animations[HeroID].frameCount = 1;
                        animations[HeroID].frames[0] = hTexHeroUpIdle;
                    }
                }
                else if (directions[HeroID].facing == 2) { // Left
                    if (targetState == 1) { // Walk
                        animations[HeroID].frameCount = 7;
                        for (int i = 0; i < 7; i++) {
                            animations[HeroID].frames[i] = hTexHeroLeftWalk[i];
                        }
                    }
                    else { // Idle
                        animations[HeroID].frameCount = 1;
                        animations[HeroID].frames[0] = hTexHeroLeftIdle;
                    }
                }
                else if (directions[HeroID].facing == 3) { // Right
                    if (targetState == 1) { // Walk
                        animations[HeroID].frameCount = 7;
                        for (int i = 0; i < 7; i++) {
                            animations[HeroID].frames[i] = hTexHeroRightWalk[i];
                        }
                    }
                    else { // Idle
                        animations[HeroID].frameCount = 1;
                        animations[HeroID].frames[0] = hTexHeroRightIdle; // Твой idle для right
                    }
                }
                else if (directions[HeroID].facing == 0) { // down
                    if (targetState == 1) { // Walk
                        animations[HeroID].frameCount = 7;
                        for (int i = 0; i < 7; i++) {
                            animations[HeroID].frames[i] = hTexHeroDownWalk[i];
                        }
                    }
                    else { // Idle
                        animations[HeroID].frameCount = 1;
                        animations[HeroID].frames[0] = hTexHeroDownIdle; // Твой idle для right
                    }
                }
                else {
                    // Остальные направления пока используют дефолтный спрайт
                    animations[HeroID].frameCount = 1;
                    animations[HeroID].frames[0] = hTexHero;
                }
            }
        }
    }
}

// GAME SYSTEMS ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||


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
            float cx = positions[id].x + colliders[id].offsetX; 
            float cy = positions[id].y + colliders[id].offsetY; 

            float newCx = newX + colliders[id].offsetX;
            float newHx = newCx - hw;
            float hy = cy - hh;
            if (!IsWallAt(newHx, hy) &&
                !IsWallAt(newHx + colliders[id].w, hy) &&
                !IsWallAt(newHx, hy + colliders[id].h) &&
                !IsWallAt(newHx + colliders[id].w, hy + colliders[id].h)) {
                positions[id].x = newX;
            }

            float hx = positions[id].x + colliders[id].offsetX - hw;
            float newCy = newY + colliders[id].offsetY;
            float newHy = newCy - hh;
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

bool EnemyCanSeeHero(uint32_t id) {
    float dx = positions[HeroID].x - positions[id].x;
    float dy = positions[HeroID].y - positions[id].y;
    float length = sqrt(dx * dx + dy * dy);
    return (length < ais[id].aggroRadius)
        && !IsWallBetween(positions[id].x, positions[id].y, positions[HeroID].x, positions[HeroID].y);
}

void SystemAI() {
    if (!hasPosition[HeroID]) return;

    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasAI[id] || !hasPosition[id] || !hasVelocity[id]) continue;

        if (ais[id].type == 1) { 
            
            float dx = positions[HeroID].x - positions[id].x;
            float dy = positions[HeroID].y - positions[id].y;
            float length = sqrt(dx * dx + dy * dy);

            bool canSee = EnemyCanSeeHero(id);

            if (canSee) {
                ais[id].state = 1; // persecution
                ais[id].timer = 180.0f; // 3 seconds
            }

            if (ais[id].state == 1) {
                float minDist = 5.0f;
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

void SystemCombat() {
    if (hasWeapon[HeroID] && hasPosition[HeroID] && hasAnimation[HeroID]) {
        weapons[HeroID].timer -= 1.0f;

        if (mouseLeftDown && weapons[HeroID].timer <= 0 && animations[HeroID].state != 2) {
            weapons[HeroID].timer = weapons[HeroID].cooldown;
            mouseLeftDown = false;

            // 1. Переключаем анимацию в состояние атаки
            animations[HeroID].state = 2; // attack
            animations[HeroID].currentFrame = 0;
            animations[HeroID].frameTimer = 0.0f;
            animations[HeroID].frameDuration = 6.0f; // Скорость атаки

            // 2. Загружаем кадры атаки для текущего направления
            int facing = directions[HeroID].facing;
            int frameCount = 4; // Сколько кадров атаки у тебя нарисовано (поменяй на 3, если 3)

            if (facing == 0) { // Down
                animations[HeroID].frameCount = frameCount;
                for (int i = 0; i < frameCount; i++) animations[HeroID].frames[i] = hTexHeroDownAttack[i];
            }
            else if (facing == 1) { // Up
                animations[HeroID].frameCount = frameCount;
                for (int i = 0; i < frameCount; i++) animations[HeroID].frames[i] = hTexHeroUpAttack[i];
            }
            else if (facing == 2) { // Left
                animations[HeroID].frameCount = frameCount;
                for (int i = 0; i < frameCount; i++) animations[HeroID].frames[i] = hTexHeroLeftAttack[i];
            }
            else if (facing == 3) { // Right
                animations[HeroID].frameCount = frameCount;
                for (int i = 0; i < frameCount; i++) animations[HeroID].frames[i] = hTexHeroRightAttack[i];
            }

            for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
                if (!active[id] || !hasHealth[id] || !hasPosition[id]) continue;
                if (id == HeroID) continue;

                float dist = DistanceBetweenHitboxes(id, HeroID);

                if (dist < HERO_ATTACK_RADIUS) {
                    healths[id].hp -= weapons[HeroID].damage;
                }
            }
        }
    }

    if (hasHealth[HeroID] && hasPosition[HeroID]) {
        for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
            if (!active[id] || !hasAI[id] || !hasHealth[id] || !hasPosition[id]) continue;
            if (id == HeroID) continue;

            ais[id].attackTimer -= 1.0f;

            float heroHalfW = colliders[HeroID].w / 2.0f;
            float heroHalfH = colliders[HeroID].h / 2.0f;
            float enemyHalfW = colliders[id].w / 2.0f;
            float enemyHalfH = colliders[id].h / 2.0f;

            float heroCx = positions[HeroID].x + colliders[HeroID].offsetX;
            float heroCy = positions[HeroID].y + colliders[HeroID].offsetY;
            float enemyCx = positions[id].x + colliders[id].offsetX;
            float enemyCy = positions[id].y + colliders[id].offsetY;

            bool overlapX = abs(enemyCx - heroCx) < (heroHalfW + enemyHalfW);
            bool overlapY = abs(enemyCy - heroCy) < (heroHalfH + enemyHalfH);

            if (overlapX && overlapY) {
                if (ais[id].attackTimer <= 0) {
                    healths[HeroID].hp -= SPIDER_DAMAGE;
                    ais[id].attackTimer = SPIDER_ATTACK_COOLDOWN;
                }
            }
        }
    }
}

void SystemDeath() {
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id]) continue;
        if (hasHealth[id] && healths[id].hp <= 0) {
            active[id] = false; 
            hasPosition[id] = false;
            hasVelocity[id] = false;
            hasRenderable[id] = false;
            hasCollider[id] = false;
            hasHealth[id] = false;
            hasAI[id] = false;
            hasWeapon[id] = false;
        }
    }
}

void SystemAnimation() {
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasAnimation[id]) continue;
        Animation& anim = animations[id];

        if (anim.frameCount <= 1) continue;

        anim.frameTimer += 1.0f;
        if (anim.frameTimer >= anim.frameDuration) {
            anim.frameTimer = 0.0f;
            anim.currentFrame++;

            // Если анимация атаки закончилась — возвращаемся в idle
            if (anim.state == 2 && anim.currentFrame >= anim.frameCount) {
                anim.state = 0;
                anim.currentFrame = 0;
                anim.frameCount = 1;
                anim.frameTimer = 0.0f;

                // Принудительно ставим idle-кадр для текущего направления
                int facing = directions[id].facing;
                if (facing == 0) anim.frames[0] = hTexHeroDownIdle;
                else if (facing == 1) anim.frames[0] = hTexHeroUpIdle;
                else if (facing == 2) anim.frames[0] = hTexHeroLeftIdle;
                else if (facing == 3) anim.frames[0] = hTexHeroRightIdle;
            }
            else if (anim.state != 2) {
                // Обычная цикличная анимация (walk)
                anim.currentFrame = anim.currentFrame % anim.frameCount;
            }
        }
    }
}

// GAMELOOP ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

void InitGame() {

    srand(time(nullptr));
    LoadBMP();
    GenerateMap();
    EntityInit();

}

void ProcessGame() {
    SystemInput();
    SystemAI();
    SystemMovement();
    SystemCombat();   
    SystemAnimation();
    SystemDeath();
    UpdateCamera();
}

void ClearGame() {
    DeleteObject(hTexHero);
    DeleteObject(hTexWall);
    DeleteObject(hTexGrass);
    DeleteObject(hTexSpider);

    DeleteObject(hTexHeroRightIdle);
    for (int i = 0; i < 7; i++) {
        if (hTexHeroRightWalk[i]) DeleteObject(hTexHeroRightWalk[i]);
    }

    DeleteObject(hTexHeroLeftIdle);
    for (int i = 0; i < 7; i++) {
        if (hTexHeroLeftWalk[i]) DeleteObject(hTexHeroLeftWalk[i]);
    }

    DeleteObject(hTexHeroUpIdle);
    for (int i = 0; i < 7; i++) {
        if (hTexHeroUpWalk[i]) DeleteObject(hTexHeroUpWalk[i]);
    }

    DeleteObject(hTexHeroDownIdle);
    for (int i = 0; i < 7; i++) {
        if (hTexHeroDownWalk[i]) DeleteObject(hTexHeroDownWalk[i]);
    }

    for (int i = 0; i < 4; i++) {
        if (hTexHeroDownAttack[i]) DeleteObject(hTexHeroDownAttack[i]);
    }

    for (int i = 0; i < 4; i++) {
        if (hTexHeroUpAttack[i]) DeleteObject(hTexHeroUpAttack[i]);
    }

    for (int i = 0; i < 4; i++) {
        if (hTexHeroLeftAttack[i]) DeleteObject(hTexHeroLeftAttack[i]);
    }

    for (int i = 0; i < 4; i++) {
        if (hTexHeroRightAttack[i]) DeleteObject(hTexHeroRightAttack[i]);
    }

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
        MessageBox(NULL, L"Window class registration error!", L"Error:Window", MB_ICONERROR);
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
            L"Error creating window!", 
            L"Error:Window", 
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

    case WM_LBUTTONDOWN:
        mouseLeftDown = true;
        break;

    case WM_LBUTTONUP:
        mouseLeftDown = false;
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