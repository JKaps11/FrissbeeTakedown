#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_ENEMIES 15
#define ENEMY_SPEED 3.0f
#define ENEMY_COLLISION_RADIUS 0.8f
#define ENEMY_MAX_HEALTH 2
#define ENEMY_ATTACK_COOLDOWN 1.0f

typedef struct {
    Vector3 position;
    int health;
    bool alive;
    float attackCooldown;
    float walkPhase;
} Enemy;

typedef struct {
    Enemy enemies[MAX_ENEMIES];
    int count;
    int aliveCount;
} EnemyManager;

EnemyManager InitEnemyManager(int enemyCount);
void UpdateEnemies(EnemyManager *manager, Vector3 playerPosition, float dt);
// Returns: 0 = no hit, 1 = hit (damaged), 2 = hit (killed)
int CheckFrisbeeEnemyCollision(EnemyManager *manager, Vector3 frisbeePos, float frisbeeRadius);
int CheckEnemyPlayerCollision(EnemyManager *manager, Vector3 playerPos, float playerRadius, float dt);
void DrawEnemies(EnemyManager *manager, Vector3 playerPosition);

#endif
