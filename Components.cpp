#include "Components.h"


// flags
bool active[MAX_ENTITIES] = {};
bool hasPosition[MAX_ENTITIES] = {};
bool hasVelocity[MAX_ENTITIES] = {};
bool hasRenderable[MAX_ENTITIES] = {};
bool hasCollider[MAX_ENTITIES] = {};
bool hasAI[MAX_ENTITIES] = {};
bool hasHealth[MAX_ENTITIES] = {};
bool hasWeapon[MAX_ENTITIES] = {};
bool hasDirection[MAX_ENTITIES] = {};
bool hasAnimation[MAX_ENTITIES] = {};
bool hasHurtFlash[MAX_ENTITIES] = {};
bool hasKnockback[MAX_ENTITIES] = {};
bool hasDash[MAX_ENTITIES] = {};
bool hasLoot[MAX_ENTITIES] = {};

// component arrays
Position positions[MAX_ENTITIES];
Velocity velocities[MAX_ENTITIES];
Renderable renderables[MAX_ENTITIES];
Collider colliders[MAX_ENTITIES];
AI ais[MAX_ENTITIES];
Health healths[MAX_ENTITIES];
Weapon weapons[MAX_ENTITIES];
Direction directions[MAX_ENTITIES];
Animation animations[MAX_ENTITIES];
HurtFlash hurtFlashes[MAX_ENTITIES];
Knockback knockbacks[MAX_ENTITIES];
Dash dashes[MAX_ENTITIES];
Loot loots[MAX_ENTITIES];

// another
uint32_t HeroID = 0;
Display window;
Viewport camera = { 0, 0, 0, 0, 0.30f };

bool keys[256] = {};
bool showGrid = false;
bool mouseLeftDown = false;
std::vector<Room> rooms;
int map[MAP_H][MAP_W];
int gold = 0;
int floorLevel = 1;
