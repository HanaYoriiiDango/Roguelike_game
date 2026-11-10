#include "System.h"

// map generation 
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

        // Портал в последней комнате
        Room lastRoom = rooms[rooms.size() - 1];
        int portalX = lastRoom.x + lastRoom.w / 2;
        int portalY = lastRoom.y + lastRoom.h / 2;
        map[portalY][portalX] = TILE_PORTAL;
    }


}
void SpawnEntityInRooms() {

    float hpMultiplier = 1.0f + (floorLevel - 1) * 0.3f;  // +30% HP за этаж
    int enemiesPerRoom = 1 + (floorLevel - 1) / 2;        // +1 враг каждые 2 этажа

    for (int i = 1; i < rooms.size(); i++) {
        int enemiesCount = 1 + rand() % enemiesPerRoom;

        for (int j = 0; j < enemiesCount; j++) {
            int tx = rooms[i].x + (rand() % rooms[i].w);
            int ty = rooms[i].y + (rand() % rooms[i].h);

            float spawnX = (tx + 0.5f) * TILE;
            float spawnY = (ty + 0.5f) * TILE;

            int type = rand() % 3; // 0 = паук, 1 = мышь, 2 = некромант

            if (type == 0) {
                // Паук
                uint32_t id = SpawnEntity(spawnX, spawnY, SPRITE_SIZE, SPRITE_SIZE, hTexSpider,
                    SPIDER_HITBOX_W, SPIDER_HITBOX_H, 0, SPIDER_HITBOX_OFFSET_Y);
                hasAI[id] = true;
                ais[id] = { 1, SPIDER_SPEED * 0.15f, SPIDER_AGGRO_RADIUS, 0, 0.0f, 0.0f };
                hasHealth[id] = true;
                healths[id] = { SPIDER_HP * hpMultiplier, SPIDER_HP * hpMultiplier };
            }
            else if (type == 1) {
                // Мышь
                uint32_t id = SpawnEntity(spawnX, spawnY, SPRITE_SIZE, SPRITE_SIZE, hTexBat,
                    BAT_HITBOX_W, BAT_HITBOX_H, 0, BAT_HITBOX_OFFSET_Y);
                hasAI[id] = true;
                ais[id] = { 2, BAT_SPEED * 0.15f, BAT_AGGRO_RADIUS, 0, 0.0f, 0.0f };
                hasHealth[id] = true;
                healths[id] = { BAT_HP * hpMultiplier, BAT_HP * hpMultiplier };
            }
            else {
                // Некромант
                uint32_t id = SpawnEntity(spawnX, spawnY, SPRITE_SIZE, SPRITE_SIZE, hTexNecromancer,
                    SHOOTER_HITBOX_W, SHOOTER_HITBOX_H, 0, SHOOTER_HITBOX_OFFSET_Y);
                hasAI[id] = true;
                ais[id] = { 3, 0, SHOOTER_AGGRO_RADIUS, 0, 0.0f, SHOOTER_SHOOT_COOLDOWN };
                hasHealth[id] = true;
                healths[id] = { SHOOTER_HP * hpMultiplier, SHOOTER_HP * hpMultiplier };
            }
        }
    }
}
void EntityInit() {
    Room firstRoom = rooms[0];
    float startX = (firstRoom.x + firstRoom.w / 2.0f) * TILE;
    float startY = (firstRoom.y + firstRoom.h / 2.0f) * TILE;

    HeroID = SpawnEntity(startX, startY, SPRITE_SIZE, SPRITE_SIZE, hTexHero,
        HERO_HITBOX_W, HERO_HITBOX_H, 0, HERO_HITBOX_OFFSET_Y);
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

    hasDash[HeroID] = true;
    dashes[HeroID] = { 0, 0, 0, 0 };

    gold = 0;

    SpawnEntityInRooms();
}

// download texures 
void LoadMapTextures() {
    hTexWall = (HBITMAP)LoadImageW(NULL, L"wall_tile64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexGrass = (HBITMAP)LoadImageW(NULL, L"grass_tile64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexPortal = (HBITMAP)LoadImageW(NULL, L"portal_tile64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}
void LoadHeroIdle() {
    hTexHeroRightIdle = (HBITMAP)LoadImageW(NULL, L"hero_right_idle.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftIdle = (HBITMAP)LoadImageW(NULL, L"hero_left_idle.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpIdle = (HBITMAP)LoadImageW(NULL, L"hero_up_idle.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownIdle = (HBITMAP)LoadImageW(NULL, L"hero_down_idle.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

}
void LoadHeroWalk() {
    hTexHeroRightWalk[0] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[1] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[2] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[3] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[4] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[5] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk6.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightWalk[6] = (HBITMAP)LoadImageW(NULL, L"hero_right_walk7.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);


    hTexHeroLeftWalk[0] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[1] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[2] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[3] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[4] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[5] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk6.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftWalk[6] = (HBITMAP)LoadImageW(NULL, L"hero_left_walk7.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);


    hTexHeroUpWalk[0] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[1] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[2] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[3] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[4] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[5] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk6.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpWalk[6] = (HBITMAP)LoadImageW(NULL, L"hero_up_walk7.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);


    hTexHeroDownWalk[0] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[1] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[2] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[3] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[4] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[5] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk6.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroDownWalk[6] = (HBITMAP)LoadImageW(NULL, L"hero_down_walk7.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

}
void LoadHeroAttack() {
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

}
void LoadHeroDash() {
    hTexHeroDownDash = (HBITMAP)LoadImageW(NULL, L"hero_down_dash.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroUpDash = (HBITMAP)LoadImageW(NULL, L"hero_up_dash.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroLeftDash = (HBITMAP)LoadImageW(NULL, L"hero_left_dash.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexHeroRightDash = (HBITMAP)LoadImageW(NULL, L"hero_right_dash.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

}
void LoadHeroTextures() {
    hTexHero = (HBITMAP)LoadImageW(NULL, L"fiol_girl64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    LoadHeroIdle();
    LoadHeroWalk();
    LoadHeroAttack();
    LoadHeroDash();
}
void LoadEnemyTextures() {
    hTexSpider = (HBITMAP)LoadImageW(NULL, L"spider64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexSpiderHurt = (HBITMAP)LoadImageW(NULL, L"spider_hurt.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexBat = (HBITMAP)LoadImageW(NULL, L"bat.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexBatHurt = (HBITMAP)LoadImageW(NULL, L"bat_hurt.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexNecromancer = (HBITMAP)LoadImageW(NULL, L"necromancer.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexNecromancerHurt = (HBITMAP)LoadImageW(NULL, L"necromancer_hurt.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexGhost = (HBITMAP)LoadImageW(NULL, L"soulball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

}
void LoadItemTextures() {
    hTexCoin = (HBITMAP)LoadImageW(NULL, L"gold.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

}
void CheckLoadHeroWalkTextures() {
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
}
void CheckLoadHeroIdleTextures() {
    if (!hTexHero) {
        MessageBoxW(NULL, L"The sprite is not loaded! The hero is left without skin ((", L"Error:LoadBMP", MB_ICONERROR);
    }
    if (!hTexHeroRightIdle) {
        MessageBoxW(NULL, L"The sprite is not loaded! The hero is left without skin ((", L"Error:LoadBMP", MB_ICONERROR);
    }
    if (!hTexHeroLeftIdle) {
        MessageBoxW(NULL, L"The sprite is not loaded! The hero is left without skin ((", L"Error:LoadBMP", MB_ICONERROR);
    }
    if (!hTexHeroUpIdle) {
        MessageBoxW(NULL, L"The sprite is not loaded! The hero is left without skin ((", L"Error:LoadBMP", MB_ICONERROR);
    }
    if (!hTexHeroDownIdle) {
        MessageBoxW(NULL, L"The sprite is not loaded! The hero is left without skin ((", L"Error:LoadBMP", MB_ICONERROR);
    }
}
void CheckLoadAttackTextures() {
    for (int i = 0; i < 4; i++) {
        if (!hTexHeroDownAttack[i]) {
            wchar_t msg[64];
            swprintf_s(msg, L"Failed to load left walk %d", i + 1);
            MessageBoxW(NULL, msg, L"LoadBMP Error", MB_ICONERROR);
        }
    }
    for (int i = 0; i < 4; i++) {
        if (!hTexHeroUpAttack[i]) {
            wchar_t msg[64];
            swprintf_s(msg, L"Failed to load left walk %d", i + 1);
            MessageBoxW(NULL, msg, L"LoadBMP Error", MB_ICONERROR);
        }
    }
    for (int i = 0; i < 4; i++) {
        if (!hTexHeroLeftAttack[i]) {
            wchar_t msg[64];
            swprintf_s(msg, L"Failed to load left walk %d", i + 1);
            MessageBoxW(NULL, msg, L"LoadBMP Error", MB_ICONERROR);
        }
    }
    for (int i = 0; i < 4; i++) {
        if (!hTexHeroRightAttack[i]) {
            wchar_t msg[64];
            swprintf_s(msg, L"Failed to load left walk %d", i + 1);
            MessageBoxW(NULL, msg, L"LoadBMP Error", MB_ICONERROR);
        }
    }

}
void CheckLoadMapTextures() {
    if (!hTexWall) {
        MessageBoxW(NULL, L"The textures didn’t load! --> wall.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }

    if (!hTexGrass) {
        MessageBoxW(NULL, L"The textures didn’t load! --> grass.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }
    if (!hTexPortal) {
        MessageBoxW(NULL, L"The textures didn’t load! --> portal.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }

}
void CheckLoadEnemyTextures() {
    if (!hTexSpider) {
        MessageBoxW(NULL, L"The textures didn’t load! --> spider.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }
    if (!hTexSpiderHurt) {
        MessageBoxW(NULL, L"The textures didn’t load! --> spider_hurt.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }
    if (!hTexBat) {
        MessageBoxW(NULL, L"The textures didn’t load! --> bat.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }
    if (!hTexBatHurt) {
        MessageBoxW(NULL, L"The textures didn’t load! --> bat_hurt.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }
    if (!hTexNecromancer) {
        MessageBoxW(NULL, L"The textures didn’t load! --> necromancer.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }
    if (!hTexNecromancerHurt) {
        MessageBoxW(NULL, L"The textures didn’t load! --> necromancer_hurt.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }
    if (!hTexGhost) {
        MessageBoxW(NULL, L"The textures didn’t load! --> soulball.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }
}
void CheckLoadItemTextures() {
    if (!hTexCoin) MessageBoxW(NULL, L"coin.bmp not found!", L"Error", MB_ICONERROR);

}
void CheckLoadTextures() {
    CheckLoadMapTextures();
    CheckLoadHeroIdleTextures();
    CheckLoadHeroWalkTextures();
    CheckLoadItemTextures();
    CheckLoadAttackTextures();
    CheckLoadEnemyTextures();
}

// entity
uint32_t CreateEntity() {
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id]) {
            active[id] = true;
            return id; // free id return
        }
    }
    return -1;
}
uint32_t SpawnEntity(float x, float y, int w, int h, HBITMAP hBitmap, float HitBoxW, float HitBoxH, float offsetX, float offsetY) {
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

// main game system - logic 
void SystemInput() {
    if (keys['G']) { showGrid = !showGrid; keys['G'] = false; }
    if (keys[VK_ESCAPE]) DestroyWindow(window.hwnd);

    if (keys['R']) {
        RestartLevel();
        keys['R'] = false;
        floorLevel = 1;
        return;
    }

    UpdateHeroVelocity();
    UpdateHeroAnimation(velocities[HeroID].dx, velocities[HeroID].dy);
}
void SystemAI() {
    if (!hasPosition[HeroID]) return;

    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasAI[id] || !hasPosition[id] || !hasVelocity[id]) continue;
        if (hasKnockback[id] && knockbacks[id].timer > 0) continue;

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
        else if (ais[id].type == 2) {
            // Летучая мышь — орбита + рывок
            float dx = positions[HeroID].x - positions[id].x;
            float dy = positions[HeroID].y - positions[id].y;
            float length = sqrt(dx * dx + dy * dy);

            bool canSee = EnemyCanSeeHero(id);

            if (canSee) {
                // Первый раз увидел — начинаем с орбиты
                if (ais[id].state == 0) {
                    ais[id].state = 1; // Орбита
                    ais[id].timer = 120.0f; // 2 секунды орбиты
                }
            }

            if (ais[id].state == 1) {
                // ОРБИТА вокруг игрока
                static float orbitTime = 0;
                orbitTime += 0.1f;

                float orbitRadius = 150.0f; // Радиус орбиты
                float orbitX = positions[HeroID].x + cos(orbitTime) * orbitRadius;
                float orbitY = positions[HeroID].y + sin(orbitTime) * orbitRadius;

                float targetDx = orbitX - positions[id].x;
                float targetDy = orbitY - positions[id].y;
                float targetLen = sqrt(targetDx * targetDx + targetDy * targetDy);

                if (targetLen > 1.0f) {
                    velocities[id].dx = (targetDx / targetLen) * ais[id].speed;
                    velocities[id].dy = (targetDy / targetLen) * ais[id].speed;
                }

                // Таймер орбиты
                ais[id].timer -= 1.0f;
                if (ais[id].timer <= 0) {
                    ais[id].state = 2; // Рывок
                    ais[id].timer = 20.0f; // 20 кадров рывка
                }
            }
            else if (ais[id].state == 2) {
                // РЫВОК к игроку
                if (length > 1.0f) {
                    float dashSpeed = ais[id].speed * 6.0f; // В 4 раза быстрее
                    velocities[id].dx = (dx / length) * dashSpeed;
                    velocities[id].dy = (dy / length) * dashSpeed;
                }

                ais[id].timer -= 1.0f;
                if (ais[id].timer <= 0) {
                    ais[id].state = 1; // Возврат к орбите
                    ais[id].timer = 120.0f;
                }
            }
            else {
                velocities[id] = { 0, 0 };
            }

        }
        else if (ais[id].type == 3) {
            // Некромант — стоит и стреляет
            velocities[id] = { 0, 0 };

            if (EnemyCanSeeHero(id)) {
                ais[id].timer -= 1.0f;
                if (ais[id].timer <= 0) {
                    SpawnGhost(id);
                    ais[id].timer = SHOOTER_SHOOT_COOLDOWN;
                }
            }
        }
    }
}
void SystemDash() {
    if (!hasDash[HeroID]) return;

    // 1. Кулдаун (уменьшается всегда)
    if (dashes[HeroID].cooldownTimer > 0) {
        dashes[HeroID].cooldownTimer -= 1.0f;
    }

    // 2. Активация (по Space)
    if (keys[VK_SPACE] && dashes[HeroID].cooldownTimer <= 0 && dashes[HeroID].timer <= 0) {
        float dx = 0, dy = 0;
        float speed = HERO_SPEED * 0.15f;
        PlaySoundW(L"whoosh.wav", NULL, SND_FILENAME | SND_ASYNC);

        if (keys['W']) dy -= 1;
        if (keys['S']) dy += 1;
        if (keys['A']) dx -= 1;
        if (keys['D']) dx += 1;

        float len = sqrt(dx * dx + dy * dy);
        if (len > 0) {
            dx /= len;
            dy /= len;
        }

        if (dx != 0 && dy != 0) {
            dx *= 0.7071f;
            dy *= 0.7071f;
        }

        if (dx == 0 && dy == 0) {
            int facing = directions[HeroID].facing;
            if (facing == 0) dy = 1;
            else if (facing == 1) dy = -1;
            else if (facing == 2) dx = -1;
            else if (facing == 3) dx = 1;
        }

        dashes[HeroID].timer = DASH_DURATION;
        dashes[HeroID].cooldownTimer = DASH_COOLDOWN;
        dashes[HeroID].dx = dx;
        dashes[HeroID].dy = dy;

        // Анимация дэша
        animations[HeroID].state = 3;
        animations[HeroID].frameCount = 1;
        animations[HeroID].currentFrame = 0;
        animations[HeroID].frameTimer = 0.0f;

        int facing = directions[HeroID].facing;
        if (facing == 0) animations[HeroID].frames[0] = hTexHeroDownDash;
        else if (facing == 1) animations[HeroID].frames[0] = hTexHeroUpDash;
        else if (facing == 2) animations[HeroID].frames[0] = hTexHeroLeftDash;
        else if (facing == 3) animations[HeroID].frames[0] = hTexHeroRightDash;
    }

    // 3. Применение дэша (перезаписываем velocity)
    if (dashes[HeroID].timer > 0) {
        dashes[HeroID].timer -= 1.0f;

        float multiplier = DASH_SPEED / (HERO_SPEED * 0.15f);
        velocities[HeroID].dx = dashes[HeroID].dx * multiplier;
        velocities[HeroID].dy = dashes[HeroID].dy * multiplier;

        // Когда дэш закончился — возвращаем idle
        if (dashes[HeroID].timer <= 0 && animations[HeroID].state == 3) {
            animations[HeroID].state = 0;
            animations[HeroID].currentFrame = 0;
            animations[HeroID].frameCount = 1;

            int facing = directions[HeroID].facing;
            if (facing == 0) animations[HeroID].frames[0] = hTexHeroDownIdle;
            else if (facing == 1) animations[HeroID].frames[0] = hTexHeroUpIdle;
            else if (facing == 2) animations[HeroID].frames[0] = hTexHeroLeftIdle;
            else if (facing == 3) animations[HeroID].frames[0] = hTexHeroRightIdle;
        }
    }
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
void SystemCombat() {
    HeroAttack();
    EnemyAttackHero();
    GhostHitHero();
}
void SystemAnimation() {
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasAnimation[id]) continue;
        UpdateAnimationFrame(id);
    }
}
void SystemDeath() {
    // Проверка смерти героя
    if (hasHealth[HeroID] && healths[HeroID].hp <= 0) {
        RestartLevel();
        floorLevel = 1;

        return;
    }

    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id]) continue;
        if (id == HeroID) continue; // Герой не умирает

        if (hasHealth[id] && healths[id].hp <= 0) {
            // === СПАВН МОНЕТ (ТОЛЬКО ЗДЕСЬ) ===
            int coinsCount = 1 + rand() % 3;
            for (int i = 0; i < coinsCount; i++) {
                float offsetX = (rand() % 40) - 20;
                float offsetY = (rand() % 40) - 20;

                uint32_t coinID = SpawnEntity(
                    positions[id].x + offsetX,
                    positions[id].y + offsetY,
                    16, 16, hTexCoin,
                    16, 16, 0, 0
                );

                if (coinID != -1) { // Проверка на переполнение
                    hasLoot[coinID] = true;
                    loots[coinID] = { 0 };
                }
            }

            // === УДАЛЕНИЕ ВРАГА ===
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
void SystemHurtFlash() {
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasHurtFlash[id]) continue;
        hurtFlashes[id].timer -= 1.0f;
        if (hurtFlashes[id].timer <= 0) {
            hasHurtFlash[id] = false;
        }
    }
}
void SystemKnockback() {
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasKnockback[id]) continue;

        if (knockbacks[id].timer > 0) {
            knockbacks[id].timer -= 1.0f;

            // Применяем отталкивание через velocity
            velocities[id].dx = knockbacks[id].dx * 0.15f;
            velocities[id].dy = knockbacks[id].dy * 0.15f;

            if (knockbacks[id].timer <= 0) {
                hasKnockback[id] = false;
            }
        }
    }
}
void SystemPortal() {
    if (!hasPosition[HeroID] || !hasCollider[HeroID]) return;

    float hx = positions[HeroID].x + colliders[HeroID].offsetX;
    float hy = positions[HeroID].y + colliders[HeroID].offsetY;

    int tx = (int)(hx / TILE);
    int ty = (int)(hy / TILE);

    if (tx >= 0 && tx < MAP_W && ty >= 0 && ty < MAP_H) {
        if (map[ty][tx] == TILE_PORTAL) {
            RestartLevel();
            floorLevel++;
        }
    }
}
void SystemLoot() {
    if (!hasCollider[HeroID] || !hasPosition[HeroID]) return;

    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasLoot[id] || !hasCollider[id] || !hasPosition[id]) continue;

        if (IsHitboxOverlap(id, HeroID)) {
            if (loots[id].type == 0) {
                gold += 1;
                // Звук подбора (опционально)
                // PlaySoundW(L"coin.wav", NULL, SND_FILENAME | SND_ASYNC);
            }

            // Удаляем монету
            active[id] = false;
            hasPosition[id] = false;
            hasRenderable[id] = false;
            hasCollider[id] = false;
            hasLoot[id] = false;
        }
    }
}

// render
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

    if (showGrid) {
        DrawAttackZone();
    }

    // HUD
    wchar_t buf[128];
    swprintf_s(buf, L"HP: %.0f  Gold: %d  DMG: %.0f  Floor: %d",
        healths[HeroID].hp, gold, weapons[HeroID].damage, floorLevel);
    SetTextColor(window.mem_dc, RGB(255, 255, 255));
    SetBkMode(window.mem_dc, TRANSPARENT);
    TextOutW(window.mem_dc, 20, 20, buf, wcslen(buf));
}
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
void DrawAttackZone() {
    if (!showGrid) return;
    if (!hasCollider[HeroID] || !hasDirection[HeroID]) return;

    float hx = positions[HeroID].x + colliders[HeroID].offsetX;
    float hy = positions[HeroID].y + colliders[HeroID].offsetY;

    float R = HERO_ATTACK_RADIUS;
    float W = HERO_ATTACK_WIDTH;
    float rx1, ry1, rx2, ry2;

    int facing = directions[HeroID].facing;
    if (facing == 3) { rx1 = hx; ry1 = hy - W / 2; rx2 = hx + R; ry2 = hy + W / 2; }
    else if (facing == 2) { rx1 = hx - R; ry1 = hy - W / 2; rx2 = hx; ry2 = hy + W / 2; }
    else if (facing == 1) { rx1 = hx - W / 2; ry1 = hy - R; rx2 = hx + W / 2; ry2 = hy; }
    else { rx1 = hx - W / 2; ry1 = hy; rx2 = hx + W / 2; ry2 = hy + R; }

    int sx1 = ScreenX(rx1);
    int sy1 = ScreenY(ry1);
    int sx2 = ScreenX(rx2);
    int sy2 = ScreenY(ry2);

    HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 0));
    HPEN oldPen = (HPEN)SelectObject(window.mem_dc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(window.mem_dc, GetStockObject(NULL_BRUSH));

    Rectangle(window.mem_dc, sx1, sy1, sx2, sy2);

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

            HBITMAP tex;
            if (map[ty][tx] == TILE_WALL) tex = hTexWall;
            else if (map[ty][tx] == TILE_PORTAL) tex = hTexPortal;
            else tex = hTexGrass;

            ShowSprite(sx, sy, size, size, tex, false);
        }
    }
}
void DrawEntity(uint32_t id) {
    if (!active[id] || !hasPosition[id]) return;

    // === ВЫБОР СПРАЙТА ===
    HBITMAP bmp = NULL;

    // 1. Вспышка урона (для врагов — красный спрайт)
    if (hasHurtFlash[id] && hurtFlashes[id].timer > 0) {
        if (id != HeroID && hasAI[id]) {
            // Определяем тип врага и подменяем спрайт
            if (ais[id].type == 1) bmp = hTexSpiderHurt;       // Паук
            else if (ais[id].type == 2) bmp = hTexBatHurt;     // Мышь
            else if (ais[id].type == 3) bmp = hTexNecromancerHurt; 
        }
    }

    // 2. Обычная логика (если не вспышка)
    if (!bmp) {
        if (hasAnimation[id] && animations[id].frameCount > 0) {
            bmp = animations[id].frames[animations[id].currentFrame];
        }
        else if (hasRenderable[id]) {
            bmp = renderables[id].bmp;
        }
    }

    if (!bmp) return;

    // === РАЗМЕРЫ ===
    int w = SPRITE_SIZE, h = SPRITE_SIZE;
    if (hasRenderable[id]) {
        w = renderables[id].w;
        h = renderables[id].h;
    }

    // === ПОЗИЦИЯ ===
    float x = positions[id].x;
    float y = positions[id].y;

    int drawX = ScreenX(x) - (int)(w * ZOOM) / 2;
    int drawY = ScreenY(y) - (int)(h * ZOOM) / 2;

    // === ОТРИСОВКА СПРАЙТА ===
    ShowSprite(drawX, drawY, (int)(w * ZOOM), (int)(h * ZOOM), bmp, true);

    // === ХИТБОКС ДЛЯ ДЕБАГА ===
    if (showGrid && hasCollider[id]) {
        float hw = colliders[id].w;
        float hh = colliders[id].h;
        float hx = x + colliders[id].offsetX - hw / 2.0f;
        float hy = y + colliders[id].offsetY - hh / 2.0f;

        // Цвет: герой — зелёный, враги — красный
        COLORREF hitboxColor = (id == HeroID) ? RGB(0, 255, 0) : RGB(255, 0, 0);
        DrawDebugHitbox(hx, hy, hw, hh, hitboxColor);
    }
}

// gameloop
void InitWindow() {

    RECT r;
    GetClientRect(window.hwnd, &r);

    window.width = r.right - r.left;
    window.height = r.bottom - r.top;

}
void LoadBMP() {
    LoadMapTextures();
    LoadHeroTextures();
    LoadEnemyTextures();
    LoadItemTextures();

    CheckLoadTextures();

}
void InitGame() {

    srand(time(nullptr));
    LoadBMP();
    GenerateMap();
    EntityInit();

    //mciSendString(TEXT("open \"thisskyofmineextended.mp3\" alias music"), NULL, 0, NULL);
    //mciSendString(TEXT("play music repeat"), NULL, 0, NULL);

}
void ProcessGame() {

    // darkritualmix.mp3
    SystemInput();
    SystemAI();
    SystemDash();
    SystemKnockback();
    SystemMovement();
    SystemCombat();
    SystemAnimation();
    SystemHurtFlash();
    SystemLoot();
    SystemPortal();
    SystemDeath();
    UpdateCamera();
}
void ShowGame() {

    window.mem_dc = CreateCompatibleDC(window.hdc);
    HBITMAP hMemBmp = CreateCompatibleBitmap(window.hdc, window.width, window.height);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(window.mem_dc, hMemBmp);

    SystemRender();
    BitBlt(window.hdc, 0, 0, window.width, window.height, window.mem_dc, 0, 0, SRCCOPY);

    SelectObject(window.mem_dc, hOldBmp);
    DeleteObject(hMemBmp);
    DeleteDC(window.mem_dc);
}
void ClearGame() {
    ClearMapTextures();
    ClearHeroTextures();
    ClearEnemyTextures();
    ClearItemTextures();
}
void RestartLevel() {
    // 1. Очистить все сущности
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        active[id] = false;
        hasPosition[id] = false;
        hasVelocity[id] = false;
        hasRenderable[id] = false;
        hasCollider[id] = false;
        hasHealth[id] = false;
        hasAI[id] = false;
        hasWeapon[id] = false;
        hasDirection[id] = false;
        hasAnimation[id] = false;
        hasDash[id] = false;
        hasHurtFlash[id] = false;
        hasKnockback[id] = false;
    }

    // 2. Очистить комнаты и карту
    rooms.clear();
    gold = 0;

    // 3. Пересоздать
    GenerateMap();
    EntityInit();
}

// support functions - logic
bool IsAttackHit(uint32_t enemyID) {
    if (!hasCollider[HeroID] || !hasCollider[enemyID]) return false;
    if (!hasDirection[HeroID]) return false;

    // Центр хитбокса героя (откуда начинается удар)
    float hx = positions[HeroID].x + colliders[HeroID].offsetX;
    float hy = positions[HeroID].y + colliders[HeroID].offsetY;

    // Прямоугольник удара (вытянут вдоль взгляда)
    float R = HERO_ATTACK_RADIUS;
    float W = HERO_ATTACK_WIDTH;
    float rx1, ry1, rx2, ry2;

    int facing = directions[HeroID].facing;
    if (facing == 3) { // Right
        rx1 = hx;         ry1 = hy - W / 2;
        rx2 = hx + R;     ry2 = hy + W / 2;
    }
    else if (facing == 2) { // Left
        rx1 = hx - R;     ry1 = hy - W / 2;
        rx2 = hx;         ry2 = hy + W / 2;
    }
    else if (facing == 1) { // Up
        rx1 = hx - W / 2; ry1 = hy - R;
        rx2 = hx + W / 2; ry2 = hy;
    }
    else { // Down (0)
        rx1 = hx - W / 2; ry1 = hy;
        rx2 = hx + W / 2; ry2 = hy + R;
    }

    // Хитбокс врага
    float ex = positions[enemyID].x + colliders[enemyID].offsetX;
    float ey = positions[enemyID].y + colliders[enemyID].offsetY;
    float ehw = colliders[enemyID].w / 2.0f;
    float ehh = colliders[enemyID].h / 2.0f;

    float ex1 = ex - ehw, ey1 = ey - ehh;
    float ex2 = ex + ehw, ey2 = ey + ehh;

    // Проверка через универсальную функцию
    return IsRectOverlap(rx1, ry1, rx2, ry2, ex1, ey1, ex2, ey2);
}
void UpdateHeroVelocity() {
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
}
void UpdateHeroDirection(float dx, float dy) {
    if (!hasDirection[HeroID]) return;
    if (dx == 0 && dy == 0) return;

    if (abs(dx) > abs(dy)) {
        directions[HeroID].facing = (dx < 0) ? 2 : 3; // Left : Right
    }
    else {
        directions[HeroID].facing = (dy < 0) ? 1 : 0; // Up : Down
    }
}
void SetHeroAnimationFrames(int targetState) {
    int facing = directions[HeroID].facing;
    bool isWalk = (targetState == 1);

    if (facing == 0) { // Down
        animations[HeroID].frameCount = isWalk ? WALK_FRAME_COUNT : 1;
        for (int i = 0; i < animations[HeroID].frameCount; i++) {
            animations[HeroID].frames[i] = isWalk ? hTexHeroDownWalk[i] : hTexHeroDownIdle;
        }
    }
    else if (facing == 1) { // Up
        animations[HeroID].frameCount = isWalk ? WALK_FRAME_COUNT : 1;
        for (int i = 0; i < animations[HeroID].frameCount; i++) {
            animations[HeroID].frames[i] = isWalk ? hTexHeroUpWalk[i] : hTexHeroUpIdle;
        }
    }
    else if (facing == 2) { // Left
        animations[HeroID].frameCount = isWalk ? WALK_FRAME_COUNT : 1;
        for (int i = 0; i < animations[HeroID].frameCount; i++) {
            animations[HeroID].frames[i] = isWalk ? hTexHeroLeftWalk[i] : hTexHeroLeftIdle;
        }
    }
    else if (facing == 3) { // Right
        animations[HeroID].frameCount = isWalk ? WALK_FRAME_COUNT : 1;
        for (int i = 0; i < animations[HeroID].frameCount; i++) {
            animations[HeroID].frames[i] = isWalk ? hTexHeroRightWalk[i] : hTexHeroRightIdle;
        }
    }
}
void UpdateHeroAnimation(float dx, float dy) {
    if (!hasAnimation[HeroID]) return;
    if (animations[HeroID].state == 2) return; // Атака идёт — не трогаем
    if (animations[HeroID].state == 3) return;

    bool isMoving = (dx != 0 || dy != 0);
    int targetState = isMoving ? 1 : 0;

    int oldFacing = directions[HeroID].facing;

    if (isMoving) {
        UpdateHeroDirection(dx, dy);
    }

    if (animations[HeroID].state != targetState || oldFacing != directions[HeroID].facing) {
        animations[HeroID].state = targetState;
        animations[HeroID].currentFrame = 0;
        animations[HeroID].frameTimer = 0.0f;

        SetHeroAnimationFrames(targetState); // ← выносим загрузку кадров
    }
}
void SetHeroAttackFrames() {
    int facing = directions[HeroID].facing;
    int frameCount = ATTACK_FRAME_COUNT;

    HBITMAP* frames = nullptr;
    if (facing == 0) frames = hTexHeroDownAttack;
    else if (facing == 1) frames = hTexHeroUpAttack;
    else if (facing == 2) frames = hTexHeroLeftAttack;
    else if (facing == 3) frames = hTexHeroRightAttack;

    animations[HeroID].frameCount = frameCount;
    for (int i = 0; i < frameCount; i++) {
        animations[HeroID].frames[i] = frames[i];
    }
}
void ApplyHeroDamage() {
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasHealth[id] || !hasPosition[id]) continue;
        if (id == HeroID) continue;

        if (IsAttackHit(id)) {
            healths[id].hp -= weapons[HeroID].damage;
            hasHurtFlash[id] = true;
            hurtFlashes[id].timer = 12.0f;

            // Отталкивание
            float dx = positions[id].x - positions[HeroID].x;
            float dy = positions[id].y - positions[HeroID].y;
            float len = sqrt(dx * dx + dy * dy);
            if (len > 0.001f) {
                hasKnockback[id] = true;
                knockbacks[id].timer = KNOCKBACK_DURATION;
                knockbacks[id].dx = (dx / len) * KNOCKBACK_FORCE;
                knockbacks[id].dy = (dy / len) * KNOCKBACK_FORCE;
            }
        }
    }
}
void HeroAttack() {
    if (!hasWeapon[HeroID] || !hasPosition[HeroID] || !hasAnimation[HeroID]) return;

    // 1. Всегда уменьшаем таймер
    weapons[HeroID].timer -= 1.0f;

    // 2. Если атака идёт — не начинаем новую
    if (animations[HeroID].state == 2) return;

    // 3. Проверяем клик и кулдаун
    if (!mouseLeftDown || weapons[HeroID].timer > 0) return;

    // 4. Запуск атаки
    weapons[HeroID].timer = weapons[HeroID].cooldown;
    mouseLeftDown = false;
    PlaySoundW(L"whip.wav", NULL, SND_FILENAME | SND_ASYNC);

    animations[HeroID].state = 2;
    animations[HeroID].currentFrame = 0;
    animations[HeroID].frameTimer = 0.0f;
    animations[HeroID].frameDuration = 6.0f;

    SetHeroAttackFrames();
    ApplyHeroDamage();
}
void EnemyAttackHero() {
    float dmgMultiplier = 1.0f + (floorLevel - 1) * 0.2f; // +20% урона за этаж

    if (!hasHealth[HeroID] || !hasPosition[HeroID]) return;

    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasAI[id] || !hasHealth[id] || !hasPosition[id]) continue;
        if (id == HeroID) continue;

        ais[id].attackTimer -= 1.0f;

        if (IsHitboxOverlap(id, HeroID) && ais[id].attackTimer <= 0) {
            // Неуязвимость во время дэша
            if (hasDash[HeroID] && dashes[HeroID].timer > 0) continue;

            float spiderDamage = SPIDER_DAMAGE * dmgMultiplier;
            healths[HeroID].hp -= spiderDamage;
            ais[id].attackTimer = SPIDER_ATTACK_COOLDOWN;
        }
    }
}
void EndAttack(uint32_t id) {
    Animation& anim = animations[id];
    anim.state = 0;
    anim.currentFrame = 0;
    anim.frameCount = 1;
    anim.frameTimer = 0.0f;

    int facing = directions[id].facing;
    if (facing == 0) anim.frames[0] = hTexHeroDownIdle;
    else if (facing == 1) anim.frames[0] = hTexHeroUpIdle;
    else if (facing == 2) anim.frames[0] = hTexHeroLeftIdle;
    else if (facing == 3) anim.frames[0] = hTexHeroRightIdle;
}
void UpdateAnimationFrame(uint32_t id) {
    Animation& anim = animations[id];
    if (anim.frameCount <= 1) return;
    if (anim.state == 3) return;

    anim.frameTimer += 1.0f;
    if (anim.frameTimer < anim.frameDuration) return;

    anim.frameTimer = 0.0f;
    anim.currentFrame++;

    if (anim.state == 2 && anim.currentFrame >= anim.frameCount) {
        EndAttack(id);
    }
    else if (anim.state != 2) {
        anim.currentFrame = anim.currentFrame % anim.frameCount;
    }
}
bool EnemyCanSeeHero(uint32_t id) {
    float dx = positions[HeroID].x - positions[id].x;
    float dy = positions[HeroID].y - positions[id].y;
    float length = sqrt(dx * dx + dy * dy);
    return (length < ais[id].aggroRadius)
        && !IsWallBetween(positions[id].x, positions[id].y, positions[HeroID].x, positions[HeroID].y);
}
void GhostHitHero() {

    float dmgMultiplier = 1.0f + (floorLevel - 1) * 0.2f; // +20% урона за этаж


    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasAI[id]) continue;
        if (ais[id].type != 4) continue; // Призрак

        if (IsHitboxOverlap(id, HeroID)) {
            // Неуязвимость во время дэша
            if (hasDash[HeroID] && dashes[HeroID].timer > 0) continue;

            float ghostDamage = GHOST_DAMAGE * dmgMultiplier;
            healths[HeroID].hp -= ghostDamage;

            // Удаляем призрака
            active[id] = false;
            hasAI[id] = false;
            hasPosition[id] = false;
            hasVelocity[id] = false;
            hasRenderable[id] = false;
            hasCollider[id] = false;
            hasHealth[id] = false;
        }
    }
}
void SpawnGhost(uint32_t shooterID) {
    // Направление к герою
    float dx = positions[HeroID].x - positions[shooterID].x;
    float dy = positions[HeroID].y - positions[shooterID].y;
    float len = sqrt(dx * dx + dy * dy);
    if (len < 0.001f) return;

    dx /= len;
    dy /= len;

    // Спавн призрака
    uint32_t ghostID = SpawnEntity(
        positions[shooterID].x,
        positions[shooterID].y,
        32, 32, hTexGhost,
        GHOST_HITBOX_W, GHOST_HITBOX_H, 0, 0
    );

    hasVelocity[ghostID] = true;
    velocities[ghostID].dx = dx * GHOST_SPEED * 0.15f;
    velocities[ghostID].dy = dy * GHOST_SPEED * 0.15f;

    hasAI[ghostID] = true;
    ais[ghostID] = { 4, 0, 0, 0, 0, 0 };

    hasHealth[ghostID] = true;
    healths[ghostID] = { 1.0f, 1.0f };

}

int ScreenX(float worldX) { return (int)((worldX - camera.x) * ZOOM); }
int ScreenY(float worldY) { return (int)((worldY - camera.y) * ZOOM); }

// support functions - collision
bool IsRectOverlap(float rx1, float ry1, float rx2, float ry2,
    float ex1, float ey1, float ex2, float ey2) {
    return (rx1 < ex2 && rx2 > ex1 && ry1 < ey2 && ry2 > ey1);
}
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
float DistanceBetweenSprites(uint32_t a, uint32_t b) {
    float dx = positions[a].x - positions[b].x;
    float dy = positions[a].y - positions[b].y;
    return sqrt(dx * dx + dy * dy);
}
float DistanceBetweenHitboxes(uint32_t a, uint32_t b) {
    float ax = positions[a].x + (hasCollider[a] ? colliders[a].offsetX : 0.0f);
    float ay = positions[a].y + (hasCollider[a] ? colliders[a].offsetY : 0.0f);
    float bx = positions[b].x + (hasCollider[b] ? colliders[b].offsetX : 0.0f);
    float by = positions[b].y + (hasCollider[b] ? colliders[b].offsetY : 0.0f);
    float dx = ax - bx;
    float dy = ay - by;
    return sqrt(dx * dx + dy * dy);
}
bool IsHitboxOverlap(uint32_t a, uint32_t b) {
    if (!hasCollider[a] || !hasCollider[b]) return false;

    float ax = positions[a].x + colliders[a].offsetX;
    float ay = positions[a].y + colliders[a].offsetY;
    float ahw = colliders[a].w / 2.0f;
    float ahh = colliders[a].h / 2.0f;
    float ax1 = ax - ahw, ay1 = ay - ahh;
    float ax2 = ax + ahw, ay2 = ay + ahh;

    float bx = positions[b].x + colliders[b].offsetX;
    float by = positions[b].y + colliders[b].offsetY;
    float bhw = colliders[b].w / 2.0f;
    float bhh = colliders[b].h / 2.0f;
    float bx1 = bx - bhw, by1 = by - bhh;
    float bx2 = bx + bhw, by2 = by + bhh;

    return IsRectOverlap(ax1, ay1, ax2, ay2, bx1, by1, bx2, by2);
}

// support functions - cleaning
void ClearMapTextures() {
    DeleteObject(hTexWall);
    DeleteObject(hTexGrass);
    DeleteObject(hTexPortal);
}
void ClearItemTextures() {
    DeleteObject(hTexCoin);


}
void ClearHeroTextures() {
    DeleteObject(hTexHero);
    DeleteObject(hTexHeroRightIdle);
    DeleteObject(hTexHeroLeftIdle);
    DeleteObject(hTexHeroUpIdle);
    DeleteObject(hTexHeroDownIdle);
    DeleteObject(hTexHeroDownDash);
    DeleteObject(hTexHeroUpDash);
    DeleteObject(hTexHeroLeftDash);
    DeleteObject(hTexHeroRightDash);

    for (int i = 0; i < WALK_FRAME_COUNT; i++) {
        if (hTexHeroRightWalk[i]) DeleteObject(hTexHeroRightWalk[i]);
        if (hTexHeroLeftWalk[i]) DeleteObject(hTexHeroLeftWalk[i]);
        if (hTexHeroUpWalk[i]) DeleteObject(hTexHeroUpWalk[i]);
        if (hTexHeroDownWalk[i]) DeleteObject(hTexHeroDownWalk[i]);
    }

    for (int i = 0; i < ATTACK_FRAME_COUNT; i++) {
        if (hTexHeroDownAttack[i]) DeleteObject(hTexHeroDownAttack[i]);
        if (hTexHeroUpAttack[i]) DeleteObject(hTexHeroUpAttack[i]);
        if (hTexHeroLeftAttack[i]) DeleteObject(hTexHeroLeftAttack[i]);
        if (hTexHeroRightAttack[i]) DeleteObject(hTexHeroRightAttack[i]);
    }


}
void ClearEnemyTextures() {
    DeleteObject(hTexSpider);
    DeleteObject(hTexSpiderHurt);

    DeleteObject(hTexBat);
    DeleteObject(hTexBatHurt);

    DeleteObject(hTexNecromancer);
    DeleteObject(hTexNecromancerHurt);
    DeleteObject(hTexGhost);

}