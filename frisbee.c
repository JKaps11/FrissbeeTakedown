#include "frisbee.h"
#include "raymath.h"

#define FRISBEE_GRAVITY 9.8f
#define FRISBEE_DRAG 0.5f
#define FRISBEE_RADIUS 0.15f
#define MIN_THROW_SPEED 10.0f
#define MAX_THROW_SPEED 35.0f
#define THROW_DURATION 0.3f

Frisbee InitFrisbee(void) {
    Frisbee frisbee = {0};
    frisbee.position = (Vector3){0.0f, 1.0f, 0.0f};
    frisbee.velocity = (Vector3){0.0f, 0.0f, 0.0f};
    frisbee.rotation = 0.0f;
    frisbee.inFlight = false;
    return frisbee;
}

void ThrowFrisbee(Frisbee *frisbee, Player *player, Camera camera, float chargePercent) {
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

    // Scale throw speed based on charge (0.0 to 1.0)
    float throwSpeed = MIN_THROW_SPEED + (MAX_THROW_SPEED - MIN_THROW_SPEED) * chargePercent;

    frisbee->inFlight = true;
    frisbee->position = camera.position;
    frisbee->velocity = Vector3Scale(forward, throwSpeed);
    frisbee->velocity.y += 2.0f * chargePercent;  // Upward arc scales with power

    player->isThrowing = true;
    player->throwTimer = THROW_DURATION;
}

void ResetFrisbee(Frisbee *frisbee) {
    frisbee->inFlight = false;
    frisbee->velocity = (Vector3){0, 0, 0};
    frisbee->rotation = 0.0f;
}

static bool CheckTreeCollision(Frisbee *frisbee) {
    for (int i = 0; i < 20; i++) {
        float x = (i % 5) * 15.0f - 30.0f;
        float z = (i / 5) * 15.0f - 30.0f;
        float height = 2.0f + (i % 3);

        // Trunk collision (1x1xheight cube centered at x, height/2, z)
        BoundingBox trunk = {
            .min = {x - 0.5f, 0.0f, z - 0.5f},
            .max = {x + 0.5f, height, z + 0.5f}
        };
        if (CheckCollisionBoxSphere(trunk, frisbee->position, FRISBEE_RADIUS)) {
            return true;
        }

        // Foliage collision (3x3x3 cube at top of trunk)
        BoundingBox foliage = {
            .min = {x - 1.5f, height, z - 1.5f},
            .max = {x + 1.5f, height + 3.0f, z + 1.5f}
        };
        if (CheckCollisionBoxSphere(foliage, frisbee->position, FRISBEE_RADIUS)) {
            return true;
        }
    }
    return false;
}

static bool CheckWallCollision(Frisbee *frisbee) {
    // North wall: z = -50, x = -50 to 50
    BoundingBox northWall = {
        .min = {-50.0f, 0.0f, -51.0f},
        .max = {50.0f, 1.0f, -49.0f}
    };
    // South wall: z = 50
    BoundingBox southWall = {
        .min = {-50.0f, 0.0f, 49.0f},
        .max = {50.0f, 1.0f, 51.0f}
    };
    // East wall: x = 50
    BoundingBox eastWall = {
        .min = {49.0f, 0.0f, -50.0f},
        .max = {51.0f, 1.0f, 50.0f}
    };
    // West wall: x = -50
    BoundingBox westWall = {
        .min = {-51.0f, 0.0f, -50.0f},
        .max = {-49.0f, 1.0f, 50.0f}
    };

    return CheckCollisionBoxSphere(northWall, frisbee->position, FRISBEE_RADIUS) ||
           CheckCollisionBoxSphere(southWall, frisbee->position, FRISBEE_RADIUS) ||
           CheckCollisionBoxSphere(eastWall, frisbee->position, FRISBEE_RADIUS) ||
           CheckCollisionBoxSphere(westWall, frisbee->position, FRISBEE_RADIUS);
}

void UpdateFrisbee(Frisbee *frisbee, float dt) {
    if (!frisbee->inFlight) return;

    // Apply gravity
    frisbee->velocity.y -= FRISBEE_GRAVITY * dt;

    // Apply drag (air resistance)
    frisbee->velocity = Vector3Scale(frisbee->velocity, 1.0f - FRISBEE_DRAG * dt);

    // Update position
    frisbee->position = Vector3Add(frisbee->position, Vector3Scale(frisbee->velocity, dt));

    // Spin rotation (visual)
    frisbee->rotation += 720.0f * dt;

    // Ground collision
    if (frisbee->position.y <= 0.1f) {
        ResetFrisbee(frisbee);
        return;
    }

    // Tree collision
    if (CheckTreeCollision(frisbee)) {
        ResetFrisbee(frisbee);
        return;
    }

    // Wall collision
    if (CheckWallCollision(frisbee)) {
        ResetFrisbee(frisbee);
        return;
    }
}

void DrawFrisbee(Frisbee frisbee, Camera camera) {
    Vector3 drawPos;

    if (frisbee.inFlight) {
        // In flight: draw at frisbee's world position
        drawPos = frisbee.position;
    } else {
        // Not in flight: draw at end of arm
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3CrossProduct(forward, camera.up);

        // Calculate arm position (same as DrawPlayerHand)
        Vector3 armPos = camera.position;
        armPos = Vector3Add(armPos, Vector3Scale(right, 0.3f));       // right
        armPos = Vector3Add(armPos, Vector3Scale(camera.up, -0.25f)); // down
        armPos = Vector3Add(armPos, Vector3Scale(forward, 0.5f));     // forward

        // Frisbee at end of arm (extend further forward)
        drawPos = Vector3Add(armPos, Vector3Scale(forward, 0.25f));
    }

    DrawCylinder(drawPos, 0.15f, 0.15f, 0.03f, 16, RED);
    DrawCylinderWires(drawPos, 0.15f, 0.15f, 0.03f, 16, MAROON);
}
