#include "player.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>

#define PLAYER_HEIGHT 2.0f
#define WALK_SPEED 5.0f
#define SPRINT_SPEED 10.0f
#define JUMP_FORCE 8.0f
#define GRAVITY 20.0f
#define MOUSE_SENSITIVITY 0.003f
#define GROUND_LEVEL 0.0f

Player InitPlayer(void) {
    Player player = {0};
    player.position = (Vector3){0.0f, PLAYER_HEIGHT, 4.0f};
    player.velocityY = 0.0f;
    player.isGrounded = true;
    player.yaw = -90.0f * DEG2RAD;
    player.pitch = 0.0f;
    player.throwTimer = 0.0f;
    player.isThrowing = false;
    player.chargeTime = 0.0f;
    player.isCharging = false;
    player.health = PLAYER_MAX_HEALTH;
    player.maxHealth = PLAYER_MAX_HEALTH;
    player.damageFlash = 0.0f;
    return player;
}

void UpdatePlayer(Player *player, Camera *camera) {
    float dt = GetFrameTime();

    // Mouse look
    Vector2 mouseDelta = GetMouseDelta();
    player->yaw += mouseDelta.x * MOUSE_SENSITIVITY;
    player->pitch -= mouseDelta.y * MOUSE_SENSITIVITY;

    // Clamp pitch to prevent flipping
    if (player->pitch > 89.0f * DEG2RAD) player->pitch = 89.0f * DEG2RAD;
    if (player->pitch < -89.0f * DEG2RAD) player->pitch = -89.0f * DEG2RAD;

    // Calculate forward and right vectors (ignoring pitch for movement)
    Vector3 forward = {
        cosf(player->yaw),
        0.0f,
        sinf(player->yaw)
    };
    Vector3 right = {
        cosf(player->yaw + PI / 2.0f),
        0.0f,
        sinf(player->yaw + PI / 2.0f)
    };

    // Movement input
    Vector3 moveDir = {0};
    if (IsKeyDown(KEY_W)) {
        moveDir.x += forward.x;
        moveDir.z += forward.z;
    }
    if (IsKeyDown(KEY_S)) {
        moveDir.x -= forward.x;
        moveDir.z -= forward.z;
    }
    if (IsKeyDown(KEY_D)) {
        moveDir.x += right.x;
        moveDir.z += right.z;
    }
    if (IsKeyDown(KEY_A)) {
        moveDir.x -= right.x;
        moveDir.z -= right.z;
    }

    // Normalize movement direction
    float length = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
    if (length > 0.0f) {
        moveDir.x /= length;
        moveDir.z /= length;
    }

    // Sprint
    float speed = IsKeyDown(KEY_LEFT_SHIFT) ? SPRINT_SPEED : WALK_SPEED;

    // Apply horizontal movement
    player->position.x += moveDir.x * speed * dt;
    player->position.z += moveDir.z * speed * dt;

    // Jump
    if (IsKeyPressed(KEY_SPACE) && player->isGrounded) {
        player->velocityY = JUMP_FORCE;
        player->isGrounded = false;
    }

    // Apply gravity
    player->velocityY -= GRAVITY * dt;
    player->position.y += player->velocityY * dt;

    // Ground collision
    if (player->position.y <= PLAYER_HEIGHT + GROUND_LEVEL) {
        player->position.y = PLAYER_HEIGHT + GROUND_LEVEL;
        player->velocityY = 0.0f;
        player->isGrounded = true;
    }

    // Update camera position and target
    camera->position = player->position;

    Vector3 lookDir = {
        cosf(player->yaw) * cosf(player->pitch),
        sinf(player->pitch),
        sinf(player->yaw) * cosf(player->pitch)
    };

    camera->target = (Vector3){
        camera->position.x + lookDir.x,
        camera->position.y + lookDir.y,
        camera->position.z + lookDir.z
    };
}

void DrawPlayerHand(Camera camera, float throwProgress, float chargeProgress) {
    // Get camera's forward and right vectors
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3CrossProduct(forward, camera.up);

    // Arm position: offset right, down, and forward from camera
    Vector3 armPos = camera.position;
    armPos = Vector3Add(armPos, Vector3Scale(right, 0.4f));       // right
    armPos = Vector3Add(armPos, Vector3Scale(camera.up, -0.3f));  // down
    armPos = Vector3Add(armPos, Vector3Scale(forward, 0.6f));     // forward

    // Calculate yaw angle from camera forward direction
    float yawAngle = atan2f(forward.z, forward.x) * RAD2DEG;

    // Arm rotation for backhand throw:
    // - Idle: 0 degrees (forward)
    // - Charging: winds back up to 90 degrees (across body to the right)
    // - Throwing: swings from 90 degrees back to 0 (forward release)
    float armRotation = 0.0f;
    if (throwProgress > 0.0f) {
        // During throw: swing from wound-back position to forward
        armRotation = 90.0f * (1.0f - throwProgress);
    } else if (chargeProgress > 0.0f) {
        // During charge: wind back proportional to charge
        armRotation = 90.0f * chargeProgress;
    }

    // Draw arm rotated to follow camera direction (Minecraft-style horizontal arm)
    rlPushMatrix();
    rlTranslatef(armPos.x, armPos.y, armPos.z);
    rlRotatef(-yawAngle + 90.0f + armRotation, 0, 1, 0);  // Base rotation + animation
    // Arm dimensions: width (left-right), height (up-down), length (extends forward)
    DrawCube((Vector3){0, 0, 0}, 0.35f, 0.1f, 0.12f, BEIGE);
    rlPopMatrix();
}
