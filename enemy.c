#include "enemy.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdlib.h>
#include <math.h>

// Tree positions from map.c (20 trees in a 5x4 grid)
static Vector3 GetTreePosition(int index) {
    float x = (index % 5) * 15.0f - 30.0f;
    float z = (index / 5) * 15.0f - 30.0f;
    return (Vector3){x, 0.0f, z};
}

static bool IsPositionValid(Vector3 pos, Vector3 playerStart) {
    // Check minimum distance from player start (15 units)
    float distToPlayer = sqrtf((pos.x - playerStart.x) * (pos.x - playerStart.x) +
                               (pos.z - playerStart.z) * (pos.z - playerStart.z));
    if (distToPlayer < 15.0f) return false;

    // Check wall margins (5 units from edges at +-50)
    if (pos.x < -45.0f || pos.x > 45.0f || pos.z < -45.0f || pos.z > 45.0f) return false;

    // Check tree avoidance (4 unit radius from each tree)
    for (int i = 0; i < 20; i++) {
        Vector3 treePos = GetTreePosition(i);
        float distToTree = sqrtf((pos.x - treePos.x) * (pos.x - treePos.x) +
                                 (pos.z - treePos.z) * (pos.z - treePos.z));
        if (distToTree < 4.0f) return false;
    }

    return true;
}

EnemyManager InitEnemyManager(int enemyCount) {
    EnemyManager manager = {0};
    manager.count = enemyCount;
    manager.aliveCount = enemyCount;

    Vector3 playerStart = {0.0f, 0.0f, 4.0f};  // Player starts at (0, 2, 4), use XZ

    for (int i = 0; i < enemyCount; i++) {
        manager.enemies[i].health = ENEMY_MAX_HEALTH;
        manager.enemies[i].alive = true;
        manager.enemies[i].attackCooldown = 0.0f;
        manager.enemies[i].walkPhase = (float)(rand() % 100) / 100.0f * 6.28f;

        // Find valid spawn position
        Vector3 spawnPos;
        int attempts = 0;
        do {
            spawnPos.x = (float)(rand() % 80) - 40.0f;  // -40 to 40
            spawnPos.y = 0.0f;
            spawnPos.z = (float)(rand() % 80) - 40.0f;
            attempts++;
        } while (!IsPositionValid(spawnPos, playerStart) && attempts < 100);

        manager.enemies[i].position = spawnPos;
    }

    return manager;
}

void UpdateEnemies(EnemyManager *manager, Vector3 playerPosition, float dt) {
    for (int i = 0; i < manager->count; i++) {
        Enemy *enemy = &manager->enemies[i];
        if (!enemy->alive) continue;

        // Calculate direction to player (XZ plane only)
        Vector3 toPlayer = {
            playerPosition.x - enemy->position.x,
            0.0f,
            playerPosition.z - enemy->position.z
        };

        float dist = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
        if (dist > 0.1f) {
            // Normalize direction
            toPlayer.x /= dist;
            toPlayer.z /= dist;

            // Move toward player
            enemy->position.x += toPlayer.x * ENEMY_SPEED * dt;
            enemy->position.z += toPlayer.z * ENEMY_SPEED * dt;
        }

        // Simple tree avoidance: push away if within 2.5 units
        for (int t = 0; t < 20; t++) {
            Vector3 treePos = GetTreePosition(t);
            float distToTree = sqrtf((enemy->position.x - treePos.x) * (enemy->position.x - treePos.x) +
                                     (enemy->position.z - treePos.z) * (enemy->position.z - treePos.z));
            if (distToTree < 2.5f && distToTree > 0.01f) {
                // Push away from tree
                float pushX = (enemy->position.x - treePos.x) / distToTree;
                float pushZ = (enemy->position.z - treePos.z) / distToTree;
                float pushStrength = (2.5f - distToTree) * 2.0f;
                enemy->position.x += pushX * pushStrength * dt;
                enemy->position.z += pushZ * pushStrength * dt;
            }
        }

        // Clamp to map bounds
        if (enemy->position.x < -48.0f) enemy->position.x = -48.0f;
        if (enemy->position.x > 48.0f) enemy->position.x = 48.0f;
        if (enemy->position.z < -48.0f) enemy->position.z = -48.0f;
        if (enemy->position.z > 48.0f) enemy->position.z = 48.0f;

        // Decrement attack cooldown
        if (enemy->attackCooldown > 0.0f) {
            enemy->attackCooldown -= dt;
        }

        // Update walk animation phase
        enemy->walkPhase += dt * 10.0f;
        if (enemy->walkPhase > 6.28f) enemy->walkPhase -= 6.28f;
    }
}

int CheckFrisbeeEnemyCollision(EnemyManager *manager, Vector3 frisbeePos, float frisbeeRadius) {
    for (int i = 0; i < manager->count; i++) {
        Enemy *enemy = &manager->enemies[i];
        if (!enemy->alive) continue;

        // Enemy center is at y+1.0 (middle of body)
        Vector3 enemyCenter = {
            enemy->position.x,
            enemy->position.y + 1.0f,
            enemy->position.z
        };

        float dist = Vector3Distance(frisbeePos, enemyCenter);
        if (dist < frisbeeRadius + ENEMY_COLLISION_RADIUS) {
            enemy->health--;
            if (enemy->health <= 0) {
                enemy->alive = false;
                manager->aliveCount--;
                return 2;  // Enemy killed
            }
            return 1;  // Enemy damaged
        }
    }
    return 0;  // No hit
}

int CheckEnemyPlayerCollision(EnemyManager *manager, Vector3 playerPos, float playerRadius, float dt) {
    (void)dt;  // Currently unused but kept for future use
    int totalDamage = 0;

    for (int i = 0; i < manager->count; i++) {
        Enemy *enemy = &manager->enemies[i];
        if (!enemy->alive) continue;

        // Distance check on XZ plane
        float dist = sqrtf((playerPos.x - enemy->position.x) * (playerPos.x - enemy->position.x) +
                           (playerPos.z - enemy->position.z) * (playerPos.z - enemy->position.z));

        float attackRange = playerRadius + ENEMY_COLLISION_RADIUS;
        if (dist < attackRange && enemy->attackCooldown <= 0.0f) {
            totalDamage++;
            enemy->attackCooldown = ENEMY_ATTACK_COOLDOWN;
        }
    }

    return totalDamage;
}

void DrawEnemies(EnemyManager *manager, Vector3 playerPosition) {
    for (int i = 0; i < manager->count; i++) {
        Enemy *enemy = &manager->enemies[i];
        if (!enemy->alive) continue;

        Vector3 pos = enemy->position;

        // Calculate angle to face player
        float dx = playerPosition.x - pos.x;
        float dz = playerPosition.z - pos.z;
        float angle = atan2f(dx, dz) * RAD2DEG;

        // Walking animation - leg swing
        float legSwing = sinf(enemy->walkPhase) * 25.0f;

        // Colors
        Color skinColor = (Color){255, 200, 150, 255};
        Color jerseyColor = RED;
        Color shortsColor = DARKBLUE;
        Color eyeColor = BLACK;

        rlPushMatrix();
        rlTranslatef(pos.x, pos.y, pos.z);
        rlRotatef(angle, 0, 1, 0);

        // Left leg with animation
        rlPushMatrix();
        rlTranslatef(-0.15f, 0.7f, 0.0f);
        rlRotatef(legSwing, 1, 0, 0);
        rlTranslatef(0.0f, -0.35f, 0.0f);
        DrawCube((Vector3){0, -0.15f, 0}, 0.15f, 0.4f, 0.15f, skinColor);  // Lower leg
        DrawCube((Vector3){0, 0.15f, 0}, 0.2f, 0.3f, 0.2f, shortsColor);   // Upper leg
        rlPopMatrix();

        // Right leg with opposite animation
        rlPushMatrix();
        rlTranslatef(0.15f, 0.7f, 0.0f);
        rlRotatef(-legSwing, 1, 0, 0);
        rlTranslatef(0.0f, -0.35f, 0.0f);
        DrawCube((Vector3){0, -0.15f, 0}, 0.15f, 0.4f, 0.15f, skinColor);  // Lower leg
        DrawCube((Vector3){0, 0.15f, 0}, 0.2f, 0.3f, 0.2f, shortsColor);   // Upper leg
        rlPopMatrix();

        // Torso/Jersey
        DrawCube((Vector3){0, 1.05f, 0}, 0.5f, 0.7f, 0.25f, jerseyColor);

        // Left arm with swing
        rlPushMatrix();
        rlTranslatef(-0.35f, 1.1f, 0.0f);
        rlRotatef(-legSwing * 0.5f, 1, 0, 0);
        DrawCube((Vector3){0, 0, 0}, 0.15f, 0.35f, 0.15f, jerseyColor);    // Sleeve
        DrawCube((Vector3){0, -0.3f, 0}, 0.12f, 0.3f, 0.12f, skinColor);   // Forearm
        rlPopMatrix();

        // Right arm with opposite swing
        rlPushMatrix();
        rlTranslatef(0.35f, 1.1f, 0.0f);
        rlRotatef(legSwing * 0.5f, 1, 0, 0);
        DrawCube((Vector3){0, 0, 0}, 0.15f, 0.35f, 0.15f, jerseyColor);    // Sleeve
        DrawCube((Vector3){0, -0.3f, 0}, 0.12f, 0.3f, 0.12f, skinColor);   // Forearm
        rlPopMatrix();

        // Head
        DrawCube((Vector3){0, 1.575f, 0}, 0.35f, 0.35f, 0.35f, skinColor);

        // Eyes (facing forward in local space)
        DrawCube((Vector3){-0.08f, 1.6f, 0.15f}, 0.06f, 0.06f, 0.06f, eyeColor);
        DrawCube((Vector3){0.08f, 1.6f, 0.15f}, 0.06f, 0.06f, 0.06f, eyeColor);

        rlPopMatrix();

        // Health bar above head (drawn in world space, not rotated)
        if (enemy->health < ENEMY_MAX_HEALTH) {
            float barWidth = 0.6f;
            float barHeight = 0.1f;
            float healthPercent = (float)enemy->health / ENEMY_MAX_HEALTH;

            DrawCube((Vector3){pos.x, pos.y + 2.0f, pos.z}, barWidth, barHeight, 0.05f, RED);
            float fillWidth = barWidth * healthPercent;
            float fillOffset = (barWidth - fillWidth) / 2.0f;
            DrawCube((Vector3){pos.x - fillOffset, pos.y + 2.0f, pos.z + 0.01f}, fillWidth, barHeight, 0.05f, GREEN);
        }
    }
}
