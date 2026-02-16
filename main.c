#include "game.h"
#include "raylib.h"

int main() {
  const int screenWidth = 1920;
  const int screenHeight = 1080;

  InitWindow(screenWidth, screenHeight, "Frisbee Thrower");
  InitAudioDevice();
  SetTargetFPS(60);

  Game game = InitGame();

  while (!WindowShouldClose()) {
    UpdateMusicStream(game.backgroundMusic);
    UpdateGame(&game);
    DrawGame(&game);
  }

  UnloadGameAudio(&game);
  CloseAudioDevice();
  CloseWindow();

  return 0;
}
