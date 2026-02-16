#include "game.h"
#include "camera.h"
#include "map.h"
#include "player.h"
#include "enemy.h"
#include <stdio.h>
#include <stdlib.h>

static const int LEVEL_ENEMY_COUNTS[] = {5, 10, 15};
static const float MAX_CHARGE_TIME = 1.0f;  // 1 second to fully charge

static void UpdateTitleScreen(Game *game);
static void UpdateLevelSelect(Game *game);
static void UpdatePlaying(Game *game);
static void UpdateGameOver(Game *game);
static void UpdateVictory(Game *game);
static void DrawTitleScreen(void);
static void DrawLevelSelect(Game *game);
static void DrawHUD(Game *game);
static void DrawGameOver(void);
static void DrawVictory(Game *game);

Game InitGame(void) {
    Game game = {0};
    game.state = STATE_TITLE;
    game.selectedLevel = 1;
    game.enemiesRemaining = 0;
    game.camera = InitCamera();
    game.frisbee = InitFrisbee();
    game.player = InitPlayer();
    // Load audio
    game.backgroundMusic = LoadMusicStream("media/background.mp3");
    game.throwSound = LoadSound("media/frisbeeThrow.mp3");
    game.damageSounds[0] = LoadSound("media/damage1.mp3");
    game.damageSounds[1] = LoadSound("media/damage2.mp3");
    game.deathSounds[0] = LoadSound("media/death1.mp3");
    game.deathSounds[1] = LoadSound("media/death2.mp3");
    game.walkingSound = LoadSound("media/walkingGrass.mp3");
    PlayMusicStream(game.backgroundMusic);
    return game;
}

void UnloadGameAudio(Game *game) {
    UnloadMusicStream(game->backgroundMusic);
    UnloadSound(game->throwSound);
    UnloadSound(game->damageSounds[0]);
    UnloadSound(game->damageSounds[1]);
    UnloadSound(game->deathSounds[0]);
    UnloadSound(game->deathSounds[1]);
    UnloadSound(game->walkingSound);
}

void UpdateGame(Game *game) {
    switch (game->state) {
        case STATE_TITLE:
            UpdateTitleScreen(game);
            break;
        case STATE_LEVEL_SELECT:
            UpdateLevelSelect(game);
            break;
        case STATE_PLAYING:
            UpdatePlaying(game);
            break;
        case STATE_GAME_OVER:
            UpdateGameOver(game);
            break;
        case STATE_VICTORY:
            UpdateVictory(game);
            break;
    }
}

void DrawGame(Game *game) {
    BeginDrawing();

    switch (game->state) {
        case STATE_TITLE:
            ClearBackground(DARKBLUE);
            DrawTitleScreen();
            break;
        case STATE_LEVEL_SELECT:
            ClearBackground(DARKBLUE);
            DrawLevelSelect(game);
            break;
        case STATE_PLAYING:
            ClearBackground(SKYBLUE);
            BeginMode3D(game->camera);
            DrawMap();
            DrawEnemies(&game->enemies, game->player.position);
            float throwProgress = game->player.isThrowing ?
                (1.0f - game->player.throwTimer / 0.3f) : 0.0f;
            float chargeProgress = game->player.isCharging ?
                (game->player.chargeTime / MAX_CHARGE_TIME) : 0.0f;
            DrawPlayerHand(game->camera, throwProgress, chargeProgress);
            DrawFrisbee(game->frisbee, game->camera);
            EndMode3D();

            // Damage flash overlay
            if (game->player.damageFlash > 0.0f) {
                unsigned char alpha = (unsigned char)(game->player.damageFlash * 255.0f);
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){255, 0, 0, alpha});
            }

            DrawFPS(10, 10);
            DrawHUD(game);
            break;
        case STATE_GAME_OVER:
            ClearBackground(DARKGRAY);
            DrawGameOver();
            break;
        case STATE_VICTORY:
            ClearBackground((Color){0, 80, 0, 255});
            DrawVictory(game);
            break;
    }

    EndDrawing();
}

static void UpdateTitleScreen(Game *game) {
    if (GetKeyPressed() != 0) {
        game->state = STATE_LEVEL_SELECT;
    }
}

static void UpdateLevelSelect(Game *game) {
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_LEFT)) {
        game->selectedLevel--;
        if (game->selectedLevel < 1) game->selectedLevel = 3;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_RIGHT)) {
        game->selectedLevel++;
        if (game->selectedLevel > 3) game->selectedLevel = 1;
    }

    if (IsKeyPressed(KEY_ONE)) game->selectedLevel = 1;
    if (IsKeyPressed(KEY_TWO)) game->selectedLevel = 2;
    if (IsKeyPressed(KEY_THREE)) game->selectedLevel = 3;

    if (IsKeyPressed(KEY_ENTER)) {
        int enemyCount = LEVEL_ENEMY_COUNTS[game->selectedLevel - 1];
        game->enemiesRemaining = enemyCount;
        game->camera = InitCamera();
        game->frisbee = InitFrisbee();
        game->player = InitPlayer();
        game->enemies = InitEnemyManager(enemyCount);
        StopMusicStream(game->backgroundMusic);
        PlayMusicStream(game->backgroundMusic);
        DisableCursor();
        game->state = STATE_PLAYING;
    }
}

static void UpdatePlaying(Game *game) {
    float dt = GetFrameTime();

    UpdatePlayer(&game->player, &game->camera);

    // Walking sound - play when moving on ground
    bool isMoving = (IsKeyDown(KEY_W) || IsKeyDown(KEY_S) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D));
    if (isMoving && game->player.isGrounded) {
        if (!IsSoundPlaying(game->walkingSound)) {
            PlaySound(game->walkingSound);
        }
    } else {
        StopSound(game->walkingSound);
    }

    // Update enemies
    UpdateEnemies(&game->enemies, game->player.position, dt);

    // Handle charge and throw input
    if (!game->frisbee.inFlight && !game->player.isThrowing) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Start charging
            game->player.isCharging = true;
            game->player.chargeTime = 0.0f;
        }

        if (game->player.isCharging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            // Accumulate charge
            game->player.chargeTime += dt;
            if (game->player.chargeTime > MAX_CHARGE_TIME) {
                game->player.chargeTime = MAX_CHARGE_TIME;
            }
        }

        if (game->player.isCharging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            // Release throw
            float chargePercent = game->player.chargeTime / MAX_CHARGE_TIME;
            ThrowFrisbee(&game->frisbee, &game->player, game->camera, chargePercent);
            SetSoundVolume(game->throwSound, 0.3f + 0.7f * chargePercent);
            PlaySound(game->throwSound);
            game->player.isCharging = false;
            game->player.chargeTime = 0.0f;
        }
    }

    // Update frisbee physics
    UpdateFrisbee(&game->frisbee, dt);

    // Frisbee-enemy collision
    if (game->frisbee.inFlight) {
        int hitResult = CheckFrisbeeEnemyCollision(&game->enemies, game->frisbee.position, 0.15f);
        if (hitResult > 0) {
            ResetFrisbee(&game->frisbee);
            game->enemiesRemaining = game->enemies.aliveCount;
            if (hitResult == 2) {
                PlaySound(game->deathSounds[rand() % 2]);
            } else {
                PlaySound(game->damageSounds[rand() % 2]);
            }
        }
    }

    // Enemy-player collision
    int damage = CheckEnemyPlayerCollision(&game->enemies, game->player.position, PLAYER_COLLISION_RADIUS, dt);
    if (damage > 0) {
        game->player.health -= damage;
        game->player.damageFlash = 0.3f;
        PlaySound(game->damageSounds[rand() % 2]);
    }

    // Update damage flash
    if (game->player.damageFlash > 0.0f) {
        game->player.damageFlash -= dt;
    }

    // Win/lose conditions
    if (game->player.health <= 0) {
        EnableCursor();
        StopMusicStream(game->backgroundMusic);
        PlaySound(game->deathSounds[rand() % 2]);
        game->state = STATE_GAME_OVER;
    }
    if (game->enemies.aliveCount <= 0) {
        EnableCursor();
        StopMusicStream(game->backgroundMusic);
        game->state = STATE_VICTORY;
    }

    // Update throw animation timer
    if (game->player.isThrowing) {
        game->player.throwTimer -= dt;
        if (game->player.throwTimer <= 0) {
            game->player.isThrowing = false;
        }
    }
}

static void DrawTitleScreen(void) {
    const char *title = "FRISBEE TAKEDOWN";
    const char *prompt = "Press any key to continue";

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    int titleFontSize = 60;
    int promptFontSize = 20;

    int titleWidth = MeasureText(title, titleFontSize);
    int promptWidth = MeasureText(prompt, promptFontSize);

    DrawText(title, (screenWidth - titleWidth) / 2, screenHeight / 2 - 50, titleFontSize, WHITE);
    DrawText(prompt, (screenWidth - promptWidth) / 2, screenHeight / 2 + 30, promptFontSize, LIGHTGRAY);
}

static void DrawLevelSelect(Game *game) {
    const char *header = "SELECT LEVEL";

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    int headerFontSize = 40;
    int levelFontSize = 30;

    int headerWidth = MeasureText(header, headerFontSize);
    DrawText(header, (screenWidth - headerWidth) / 2, screenHeight / 2 - 120, headerFontSize, WHITE);

    for (int i = 1; i <= 3; i++) {
        char levelText[64];
        snprintf(levelText, sizeof(levelText), "Level %d - %d Enemies", i, LEVEL_ENEMY_COUNTS[i - 1]);

        int levelWidth = MeasureText(levelText, levelFontSize);
        int y = screenHeight / 2 - 40 + (i - 1) * 50;

        Color color = (i == game->selectedLevel) ? YELLOW : LIGHTGRAY;
        DrawText(levelText, (screenWidth - levelWidth) / 2, y, levelFontSize, color);
    }

    const char *instructions = "Use Arrow Keys or 1-3, Enter to Start";
    int instrFontSize = 18;
    int instrWidth = MeasureText(instructions, instrFontSize);
    DrawText(instructions, (screenWidth - instrWidth) / 2, screenHeight / 2 + 130, instrFontSize, GRAY);
}

static void DrawHUD(Game *game) {
    char hudText[32];
    snprintf(hudText, sizeof(hudText), "Enemies Left: %d", game->enemiesRemaining);
    DrawText(hudText, 10, 35, 20, WHITE);

    // Draw player health bar (top-right)
    int screenWidth = GetScreenWidth();
    int healthBarWidth = 150;
    int healthBarHeight = 20;
    int healthBarX = screenWidth - healthBarWidth - 20;
    int healthBarY = 10;

    float healthPercent = (float)game->player.health / game->player.maxHealth;

    // Determine health bar color based on percentage
    Color healthColor;
    if (healthPercent > 0.6f) {
        healthColor = GREEN;
    } else if (healthPercent > 0.3f) {
        healthColor = YELLOW;
    } else {
        healthColor = RED;
    }

    // Background
    DrawRectangle(healthBarX - 2, healthBarY - 2, healthBarWidth + 4, healthBarHeight + 4, DARKGRAY);
    // Fill
    DrawRectangle(healthBarX, healthBarY, (int)(healthBarWidth * healthPercent), healthBarHeight, healthColor);
    // Border
    DrawRectangleLines(healthBarX - 2, healthBarY - 2, healthBarWidth + 4, healthBarHeight + 4, WHITE);

    // Health label
    char healthText[16];
    snprintf(healthText, sizeof(healthText), "HP: %d/%d", game->player.health, game->player.maxHealth);
    int textWidth = MeasureText(healthText, 16);
    DrawText(healthText, healthBarX + (healthBarWidth - textWidth) / 2, healthBarY + 2, 16, WHITE);

    // Draw charge bar when charging
    if (game->player.isCharging) {
        int screenHeight = GetScreenHeight();
        int barWidth = 200;
        int barHeight = 10;
        int barX = (screenWidth - barWidth) / 2;
        int barY = screenHeight - 50;

        float chargePercent = game->player.chargeTime / MAX_CHARGE_TIME;

        // Background
        DrawRectangle(barX - 2, barY - 2, barWidth + 4, barHeight + 4, DARKGRAY);
        // Fill
        DrawRectangle(barX, barY, (int)(barWidth * chargePercent), barHeight, RED);
        // Border
        DrawRectangleLines(barX - 2, barY - 2, barWidth + 4, barHeight + 4, WHITE);
    }
}

static void UpdateGameOver(Game *game) {
    if (IsKeyPressed(KEY_ENTER)) {
        game->state = STATE_LEVEL_SELECT;
    }
}

static void UpdateVictory(Game *game) {
    if (IsKeyPressed(KEY_ENTER)) {
        // Next level
        if (game->selectedLevel < 3) {
            game->selectedLevel++;
        }
        int enemyCount = LEVEL_ENEMY_COUNTS[game->selectedLevel - 1];
        game->enemiesRemaining = enemyCount;
        game->camera = InitCamera();
        game->frisbee = InitFrisbee();
        game->player = InitPlayer();
        game->enemies = InitEnemyManager(enemyCount);
        StopMusicStream(game->backgroundMusic);
        PlayMusicStream(game->backgroundMusic);
        DisableCursor();
        game->state = STATE_PLAYING;
    }
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) {
        game->state = STATE_LEVEL_SELECT;
    }
}

static void DrawGameOver(void) {
    const char *title = "GAME OVER";
    const char *prompt = "Press ENTER to return to menu";

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    int titleFontSize = 60;
    int promptFontSize = 20;

    int titleWidth = MeasureText(title, titleFontSize);
    int promptWidth = MeasureText(prompt, promptFontSize);

    DrawText(title, (screenWidth - titleWidth) / 2, screenHeight / 2 - 50, titleFontSize, RED);
    DrawText(prompt, (screenWidth - promptWidth) / 2, screenHeight / 2 + 30, promptFontSize, LIGHTGRAY);
}

static void DrawVictory(Game *game) {
    const char *title = "VICTORY!";
    const char *nextPrompt = game->selectedLevel < 3 ?
        "Press ENTER for next level" : "Press ENTER to replay level 3";
    const char *quitPrompt = "Press Q to return to menu";

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    int titleFontSize = 60;
    int promptFontSize = 20;

    int titleWidth = MeasureText(title, titleFontSize);
    int nextWidth = MeasureText(nextPrompt, promptFontSize);
    int quitWidth = MeasureText(quitPrompt, promptFontSize);

    DrawText(title, (screenWidth - titleWidth) / 2, screenHeight / 2 - 50, titleFontSize, YELLOW);
    DrawText(nextPrompt, (screenWidth - nextWidth) / 2, screenHeight / 2 + 30, promptFontSize, LIGHTGRAY);
    DrawText(quitPrompt, (screenWidth - quitWidth) / 2, screenHeight / 2 + 60, promptFontSize, GRAY);
}
