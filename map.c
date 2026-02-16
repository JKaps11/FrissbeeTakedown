#include "map.h"

void DrawMap(void) {
    DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){200.0f, 200.0f}, DARKGREEN);
    DrawPlane((Vector3){0.0f, 0.01f, 0.0f}, (Vector2){80.0f, 80.0f}, GREEN);
    DrawPlane((Vector3){0.0f, 0.02f, 0.0f}, (Vector2){30.0f, 30.0f}, LIME);

    for (int i = 0; i < 20; i++) {
        float x = (i % 5) * 15.0f - 30.0f;
        float z = (i / 5) * 15.0f - 30.0f;
        float height = 2.0f + (i % 3);
        DrawCube((Vector3){x, height / 2.0f, z}, 1.0f, height, 1.0f, BROWN);
        DrawCube((Vector3){x, height + 1.5f, z}, 3.0f, 3.0f, 3.0f, DARKGREEN);
    }

    DrawCube((Vector3){0.0f, 0.5f, -50.0f}, 100.0f, 1.0f, 2.0f, DARKGRAY);
    DrawCube((Vector3){50.0f, 0.5f, 0.0f}, 2.0f, 1.0f, 100.0f, DARKGRAY);
    DrawCube((Vector3){-50.0f, 0.5f, 0.0f}, 2.0f, 1.0f, 100.0f, DARKGRAY);
    DrawCube((Vector3){0.0f, 0.5f, 50.0f}, 100.0f, 1.0f, 2.0f, DARKGRAY);
}
