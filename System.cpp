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

            uint32_t enemyID = SpawnEntity(spawnX, spawnY, SPRITE_SIZE, SPRITE_SIZE, hTexSpider,
                SPIDER_HITBOX_W, SPIDER_HITBOX_H, 0, SPIDER_HITBOX_OFFSET_Y);
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

    SpawnEntityInRooms();
}

// download texures 
void LoadMapTextures() {
    hTexWall = (HBITMAP)LoadImageW(NULL, L"wall_tile64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hTexGrass = (HBITMAP)LoadImageW(NULL, L"grass_tile64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

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
void LoadHeroTextures() {
    hTexHero = (HBITMAP)LoadImageW(NULL, L"fiol_girl64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    LoadHeroIdle();
    LoadHeroWalk();
    LoadHeroAttack();
}
void LoadSpiderTextures() {
    hTexSpider = (HBITMAP)LoadImageW(NULL, L"spider64px.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

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


}
void CheckLoadSpiderTextures() {
    if (!hTexSpider) {
        MessageBoxW(NULL, L"The textures didn’t load! --> spider.bmp", L"Error:LoadBMP", MB_ICONERROR);
    }


}
void CheckLoadTextures() {
    CheckLoadMapTextures();
    CheckLoadHeroIdleTextures();
    CheckLoadHeroWalkTextures();
    CheckLoadAttackTextures();

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

    UpdateHeroVelocity();
    UpdateHeroAnimation(velocities[HeroID].dx, velocities[HeroID].dy);
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
}
void SystemAnimation() {
    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasAnimation[id]) continue;
        UpdateAnimationFrame(id);
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

    // HUD
    wchar_t buf[64];
    swprintf_s(buf, L"HP: %.0f", healths[HeroID].hp);
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

    HBITMAP bmp = NULL;
    if (hasAnimation[id] && animations[id].frameCount > 0) {
        bmp = animations[id].frames[animations[id].currentFrame];
    }
    else if (hasRenderable[id]) {
        bmp = renderables[id].bmp;
    }

    if (!bmp) return;

    int w = SPRITE_SIZE, h = SPRITE_SIZE; // Размер спрайта (у нас 64x64)
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
    LoadSpiderTextures();

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
    SystemMovement();
    SystemCombat();
    SystemAnimation();
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
    DeleteObject(hOldBmp);
    DeleteDC(window.mem_dc);
}
void ClearGame() {
    ClearMapTextures();
    ClearHeroTextures();
    ClearSpiderTextures();
}

// support functions - logic
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

        float dist = DistanceBetweenHitboxes(id, HeroID);
        if (dist < HERO_ATTACK_RADIUS) {
            healths[id].hp -= weapons[HeroID].damage;
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
    if (!hasHealth[HeroID] || !hasPosition[HeroID]) return;

    for (uint32_t id = 0; id < MAX_ENTITIES; id++) {
        if (!active[id] || !hasAI[id] || !hasHealth[id] || !hasPosition[id]) continue;
        if (id == HeroID) continue;

        ais[id].attackTimer -= 1.0f;

        if (IsHitboxOverlap(id, HeroID) && ais[id].attackTimer <= 0) {
            healths[HeroID].hp -= SPIDER_DAMAGE;
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

int ScreenX(float worldX) { return (int)((worldX - camera.x) * ZOOM); }
int ScreenY(float worldY) { return (int)((worldY - camera.y) * ZOOM); }

// support functions - collision
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

    float halfW_a = colliders[a].w / 2.0f;
    float halfH_a = colliders[a].h / 2.0f;
    float halfW_b = colliders[b].w / 2.0f;
    float halfH_b = colliders[b].h / 2.0f;

    float cx_a = positions[a].x + colliders[a].offsetX;
    float cy_a = positions[a].y + colliders[a].offsetY;
    float cx_b = positions[b].x + colliders[b].offsetX;
    float cy_b = positions[b].y + colliders[b].offsetY;

    return (abs(cx_a - cx_b) < (halfW_a + halfW_b)) &&
        (abs(cy_a - cy_b) < (halfH_a + halfH_b));
}

// support functions - cleaning
void ClearMapTextures() {
    DeleteObject(hTexWall);
    DeleteObject(hTexGrass);
}
void ClearHeroTextures() {
    DeleteObject(hTexHero);
    DeleteObject(hTexHeroRightIdle);
    DeleteObject(hTexHeroLeftIdle);
    DeleteObject(hTexHeroUpIdle);
    DeleteObject(hTexHeroDownIdle);

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
void ClearSpiderTextures() {
    DeleteObject(hTexSpider);
}