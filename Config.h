#pragma once
// Config.h

// zoom and map
extern float ZOOM;
const int TILE = 128;
const int MAP_W = 64;
const int MAP_H = 64;

// hero
const float HERO_SPEED = 60.0f;
const float HERO_ATTACK_RADIUS = 80.0f;
const float HERO_ATTACK_WIDTH = 40.0f;
const float HERO_DAMAGE = 20.0f;
const float HERO_ATTACK_COOLDOWN = 30.0f;
const float KNOCKBACK_FORCE = 50.0f;   // Сила отталкивания (пикселей в секунду)
const float KNOCKBACK_DURATION = 10.0f; // Длительность (10 кадров ≈ 0.16 сек)
const float DASH_SPEED = 20.0f;       // Скорость (в 5 раз быстрее обычной)
const float DASH_DURATION = 10.0f;     // Длительность (10 кадров ≈ 0.16 сек)
const float DASH_COOLDOWN = 30.0f;     // Кулдаун (30 кадров ≈ 0.5 сек)

// spider
const float SPIDER_SPEED = 5.0f;
const float SPIDER_HP = 30.0f;
const float SPIDER_DAMAGE = 5.0f;
const float SPIDER_ATTACK_COOLDOWN = 30.0f;
const float SPIDER_AGGRO_RADIUS = 500.0f;

// size sprite
const int SPRITE_SIZE = 64;

// frame counter
const int WALK_FRAME_COUNT = 7;
const int ATTACK_FRAME_COUNT = 4;

// hero hitbox
const float HERO_HITBOX_W = 24.0f;
const float HERO_HITBOX_H = 24.0f;
const float HERO_HITBOX_OFFSET_Y = 15.0f;

// spider hitbox
const float SPIDER_HITBOX_W = 20.0f;
const float SPIDER_HITBOX_H = 20.0f;
const float SPIDER_HITBOX_OFFSET_Y = 5.0f;