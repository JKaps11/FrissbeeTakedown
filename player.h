#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

#define PLAYER_MAX_HEALTH 5
#define PLAYER_COLLISION_RADIUS 0.5f

typedef struct {
    Vector3 position;
    float velocityY;
    bool isGrounded;
    float yaw;
    float pitch;
    float throwTimer;
    bool isThrowing;
    float chargeTime;
    bool isCharging;
    int health;
    int maxHealth;
    float damageFlash;
} Player;

Player InitPlayer(void);
void UpdatePlayer(Player *player, Camera *camera);
void DrawPlayerHand(Camera camera, float throwProgress, float chargeProgress);

#endif
