#pragma once
#include <windows.h>
#include <cstdint>
#include <vector>
#include "Config.h"


const int MAX_ENTITIES = 1000;




// flags
extern bool active[MAX_ENTITIES];
extern bool hasPosition[MAX_ENTITIES];
extern bool hasVelocity[MAX_ENTITIES];
extern bool hasRenderable[MAX_ENTITIES];
extern bool hasCollider[MAX_ENTITIES];
extern bool hasAI[MAX_ENTITIES];
extern bool hasHealth[MAX_ENTITIES];
extern bool hasWeapon[MAX_ENTITIES];
extern bool hasDirection[MAX_ENTITIES];
extern bool hasAnimation[MAX_ENTITIES];
extern bool hasHurtFlash[MAX_ENTITIES];
extern bool hasKnockback[MAX_ENTITIES];
extern bool hasDash[MAX_ENTITIES];

// structures
struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Renderable { int w, h; HBITMAP bmp; };
struct Collider { float w, h; float offsetX, offsetY; };
struct AI { int type; float speed; float aggroRadius; int state; float timer; float attackTimer; };
struct Health { float hp, maxHp; };
struct Weapon { float damage, cooldown, timer; };
struct Direction { int facing; };
struct HurtFlash { float timer; };
struct Animation {
    HBITMAP frames[8];
    int frameCount;
    int currentFrame;
    float frameTimer;
    float frameDuration;
    int state;
};
struct Display{
    HWND hwnd;
    HDC hdc, mem_dc;
    int width, height;

};

struct Viewport{
    float x, y;
    float targetX, targetY;
    float smooth = 0.30f;

};

struct Room {
    int x, y;
    int w, h;
};

struct Knockback {
    float timer;   // Сколько кадров ещё летит
    float dx, dy;  // Направление отталкивания
};

struct Dash {
    float timer;         // Сколько кадров ещё идёт дэш
    float cooldownTimer; // Сколько кадров до следующего дэша
    float dx, dy;        // Направление дэша (нормализованное)
};

// component arrays
extern Position positions[MAX_ENTITIES];
extern Velocity velocities[MAX_ENTITIES];
extern Renderable renderables[MAX_ENTITIES];
extern Collider colliders[MAX_ENTITIES];
extern AI ais[MAX_ENTITIES];
extern Health healths[MAX_ENTITIES];
extern Weapon weapons[MAX_ENTITIES];
extern Direction directions[MAX_ENTITIES];
extern Animation animations[MAX_ENTITIES];
extern HurtFlash hurtFlashes[MAX_ENTITIES];
extern Knockback knockbacks[MAX_ENTITIES];
extern Dash dashes[MAX_ENTITIES];

// another
extern uint32_t HeroID;
extern Viewport camera;
extern Display window;

extern bool keys[256];
extern bool showGrid;
extern bool mouseLeftDown;
extern std::vector<Room> rooms;
extern int map[MAP_H][MAP_W];
