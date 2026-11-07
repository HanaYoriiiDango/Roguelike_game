#pragma once
#include "Components.h"
#include "Config.h"
#include "Textures.h"
//#include <algorithm>
//#include <wingdi.h> 
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "Winmm.lib")

// map generation 
void GenerateMap();
void SpawnEntityInRooms();
void EntityInit();

// download texures 
void LoadMapTextures();
void LoadHeroIdle();
void LoadHeroWalk();
void LoadHeroAttack();
void LoadHeroTextures();
void LoadSpiderTextures();
void CheckLoadHeroWalkTextures();
void CheckLoadHeroIdleTextures();
void CheckLoadAttackTextures();
void CheckLoadMapTextures();
void CheckLoadSpiderTextures();
void CheckLoadTextures();

// entity
uint32_t CreateEntity();
uint32_t SpawnEntity(float x, float y, int w, int h, HBITMAP bmp,
	float hitW, float hitH, float offX = 0, float offY = 0);

// main game system - logic 
void SystemInput();
void SystemAI();
void SystemMovement();
void SystemCombat();
void SystemAnimation(); 
void SystemDeath();
void UpdateCamera();
void SystemHurtFlash();

// render
void SystemRender();
void ShowSprite(int x, int y, int w, int h, HBITMAP hBitmap, bool transparent);
void DrawGrid();
void DrawDebugHitbox(float worldX, float worldY, float w, float h, COLORREF color);
void DrawAttackZone();
void ShowMap();
void DrawEntity(uint32_t id);

// gameloop
void InitWindow();
void LoadBMP();
void InitGame();
void ProcessGame();
void ShowGame();
void ClearGame();

// support functions 
bool IsAttackHit(uint32_t enemyID);
void UpdateHeroVelocity();
void UpdateHeroDirection(float dx, float dy);
void SetHeroAnimationFrames(int targetState);
void UpdateHeroAnimation(float dx, float dy);
void SetHeroAttackFrames();
void ApplyHeroDamage();
void HeroAttack();
void EnemyAttackHero();
void EndAttack(uint32_t id);
void UpdateAnimationFrame(uint32_t id);
bool EnemyCanSeeHero(uint32_t id);

int ScreenX(float worldX);
int ScreenY(float worldY);

// support functions - collision
bool IsRectOverlap(float rx1, float ry1, float rx2, float ry2,
	float ex1, float ey1, float ex2, float ey2);
bool IsWallAt(float worldX, float worldY);
bool IsWallBetween(float x1, float y1, float x2, float y2);
float DistanceBetweenSprites(uint32_t a, uint32_t b);
float DistanceBetweenHitboxes(uint32_t a, uint32_t b);
bool IsHitboxOverlap(uint32_t a, uint32_t b);

// support functions - cleaning
void ClearMapTextures();
void ClearHeroTextures();
void ClearSpiderTextures();

