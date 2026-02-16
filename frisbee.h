#ifndef FRISBEE_H
#define FRISBEE_H

#include "raylib.h"
#include "player.h"

typedef struct {
    Vector3 position;
    Vector3 velocity;
    float rotation;
    bool inFlight;
} Frisbee;

Frisbee InitFrisbee(void);
void DrawFrisbee(Frisbee frisbee, Camera camera);
void ThrowFrisbee(Frisbee *frisbee, Player *player, Camera camera, float chargePercent);
void UpdateFrisbee(Frisbee *frisbee, float dt);
void ResetFrisbee(Frisbee *frisbee);

#endif
