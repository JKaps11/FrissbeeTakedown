#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "frisbee.h"
#include "player.h"
#include "enemy.h"

typedef enum {
    STATE_TITLE,
    STATE_LEVEL_SELECT,
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_VICTORY
} GameState;

typedef struct {
    GameState state;
    int selectedLevel;
    int enemiesRemaining;
    Camera camera;
    Frisbee frisbee;
    Player player;
    EnemyManager enemies;
    // Audio
    Music backgroundMusic;
    Sound throwSound;
    Sound damageSounds[2];
    Sound deathSounds[2];
    Sound walkingSound;
} Game;

Game InitGame(void);
void UpdateGame(Game *game);
void DrawGame(Game *game);
void UnloadGameAudio(Game *game);

#endif
