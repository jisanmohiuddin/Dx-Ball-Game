#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdbool.h>

#define WIDTH 800
#define HEIGHT 600

#define LINES_OF_BRICKS 5
#define BRICKS_PER_LINE 10

typedef enum
{
  TITLE,
  MENU,
  HELP,
  PLAYING,
  LEVEL_SELECT,
  GAME_OVER,
  LEVEL_COMPLETE,
} GameState;

typedef struct Brick
{
  Rectangle rect;
  bool active;
  Color color;
} Brick;

void resetBricks(Brick bricks[LINES_OF_BRICKS][BRICKS_PER_LINE])
{
  for (int i = 0; i < LINES_OF_BRICKS; i++)
  {
    for (int j = 0; j < BRICKS_PER_LINE; j++)
    {
      bricks[i][j].active = true;
    }
  }
}

void resetGame(
    Brick bricks[LINES_OF_BRICKS][BRICKS_PER_LINE],
    Rectangle *paddle,
    Vector2 *ballPos,
    Vector2 *ballSpeed,
    bool *ballLaunched,
    int *score,
    int *lives,
    float ballRadius)
{
  resetBricks(bricks);

  *paddle = (Rectangle){
      WIDTH / 2.0f - 60,
      HEIGHT - 35,
      120,
      15};

  *ballPos = (Vector2){
      paddle->x + paddle->width / 2.0f,
      paddle->y - ballRadius};

  *ballSpeed = (Vector2){0, 0};

  *ballLaunched = false;

  *score = 0;
  *lives = 3;
}

int main(void)
{
  GameState state = TITLE;
  int selectedOption = 0;
  int selectedLevel = 0;
  int currentLevel = 1;
  bool ballLaunched = false;
  Vector2 ballSpeed = {0, 0};
  float speedValue = 5.0f;

  Rectangle paddle = {
      WIDTH / 2.0f - 60,
      HEIGHT - 35,
      120,
      15};

  float ballRadius = 9.0f;

  Vector2 ballPos = {
      paddle.x + paddle.width / 2.0f,
      paddle.y - ballRadius};

  int score = 0;
  int lives = 3;

  float brickWidth = (float)(WIDTH - 40) / BRICKS_PER_LINE;
  float brickHeight = 22.0f;

  Brick bricks[LINES_OF_BRICKS][BRICKS_PER_LINE];

  Color darkBg      = (Color){ 18, 16, 26, 255 };
  Color topBarColor = (Color){ 24, 22, 36, 255 };
  Color neonPink    = (Color){ 255, 20, 147, 255 };
  Color neonOrange  = (Color){ 255, 102, 0, 255 };
  Color neonYellow  = (Color){ 255, 215, 0, 255 };
  Color neonGreen   = (Color){ 57, 255, 20, 255 };
  Color neonCyan    = (Color){ 0, 229, 255, 255 };

  Color rowColors[5] = { neonPink, neonOrange, neonYellow, neonGreen, neonCyan };

  for (int i = 0; i < LINES_OF_BRICKS; i++)
  {
    for (int j = 0; j < BRICKS_PER_LINE; j++)
    {
      bricks[i][j].rect = (Rectangle){
          20 + j * brickWidth,
          80 + i * (brickHeight + 6),
          brickWidth - 5,
          brickHeight};

      bricks[i][j].active = true;
      bricks[i][j].color = rowColors[i];
    }
  }

  InitWindow(WIDTH, HEIGHT, "DX Ball");

  InitAudioDevice();
  Sound hitSound = { 0 };
  bool hasHitSound = false;
  if (FileExists("assets/audio/hitSound.mp3"))
  {
    hitSound = LoadSound("assets/audio/hitSound.mp3");
    hasHitSound = true;
  }
  Music menuBgMusic = { 0 };
  bool hasBgMusic = false;
  if (FileExists("assets/audio/retroBackground.mp3"))
  {
      menuBgMusic = LoadMusicStream("assets/audio/retroBackground.mp3");
      PlayMusicStream(menuBgMusic);
      hasBgMusic = true;
  }

  resetGame(
      bricks,
      &paddle,
      &ballPos,
      &ballSpeed,
      &ballLaunched,
      &score,
      &lives,
      ballRadius);

  SetTargetFPS(60);

  while (!WindowShouldClose())
  {
    if (hasBgMusic) UpdateMusicStream(menuBgMusic);
    if (state == TITLE)
    {
      if (IsKeyPressed(KEY_ENTER))
      {
        state = MENU;
      }
    }

    else if (state == MENU)
    {
      if (IsKeyPressed(KEY_DOWN))
      {
        selectedOption++;
      }
      if (IsKeyPressed(KEY_UP))
      {
        selectedOption--;
      }
      if (selectedOption < 0)
        selectedOption = 1;
      if (selectedOption > 1)
        selectedOption = 0;

      if (IsKeyPressed(KEY_ENTER))
      {
        if (selectedOption == 0)
        {
          state = LEVEL_SELECT;
        }
        else if (selectedOption == 1)
        {
          state = HELP;
        }
      }
    }

    else if (state == HELP)
    {
      if (IsKeyPressed(KEY_ENTER))
      {
        state = MENU;
      }
    }

    else if (state == LEVEL_SELECT)
    {
      if (IsKeyPressed(KEY_DOWN))
      {
        selectedLevel++;
      }

      if (IsKeyPressed(KEY_UP))
      {
        selectedLevel--;
      }

      if (selectedLevel < 0)
        selectedLevel = 1;

      if (selectedLevel > 1)
        selectedLevel = 0;

      if (IsKeyPressed(KEY_ENTER))
      {
        if (selectedLevel == 0)
        {
          currentLevel = 1;
          speedValue = 5.0f;
          resetGame(
              bricks,
              &paddle,
              &ballPos,
              &ballSpeed,
              &ballLaunched,
              &score,
              &lives,
              ballRadius);

          state = PLAYING;
          if (hasBgMusic) StopMusicStream(menuBgMusic);
        }
        if (selectedLevel == 1)
        {
          currentLevel = 2;
          speedValue = 8.0f;

          resetGame(
              bricks,
              &paddle,
              &ballPos,
              &ballSpeed,
              &ballLaunched,
              &score,
              &lives,
              ballRadius);

          state = PLAYING;
          if (hasBgMusic) StopMusicStream(menuBgMusic);
        }
      }
    }

    else if (state == PLAYING)
    {
      if (IsKeyDown(KEY_RIGHT) && paddle.x + paddle.width < WIDTH - 5)
      {
        paddle.x += 7.0f;
      }

      if (IsKeyDown(KEY_LEFT) && paddle.x > 5)
      {
        paddle.x -= 7.0f;
      }

      if (!ballLaunched)
      {
        ballPos.x = paddle.x + paddle.width / 2.0f;
        ballPos.y = paddle.y - ballRadius;

        if (IsKeyPressed(KEY_ENTER))
        {
          ballLaunched = true;
          ballSpeed = (Vector2){speedValue, -speedValue};
        }
      }

      if (ballLaunched)
      {
        ballPos.x += ballSpeed.x;
        ballPos.y += ballSpeed.y;
      }
      if (ballPos.x <= (5 + ballRadius) || ballPos.x >= (WIDTH - 5 - ballRadius))
      {
        ballSpeed.x *= -1;
      }

      if (ballPos.y <= (50 + ballRadius))
      {
        ballSpeed.y *= -1;
      }

      if (CheckCollisionCircleRec(ballPos, ballRadius, paddle))
      {
        if (ballSpeed.y > 0)
        {
          float hitPos =
              (ballPos.x - (paddle.x + paddle.width / 2.0f)) / (paddle.width / 2.0f);

          ballSpeed.x = hitPos * speedValue;
          ballSpeed.y *= -1;
        }
      }

      for (int i = 0; i < LINES_OF_BRICKS; i++)
      {
        for (int j = 0; j < BRICKS_PER_LINE; j++)
        {
          if (bricks[i][j].active)
          {
            if (CheckCollisionCircleRec(
                    ballPos,
                    ballRadius,
                    bricks[i][j].rect))
            {
              bricks[i][j].active = false;
              score += 10;
              ballSpeed.y *= -1;

              if (hasHitSound) PlaySound(hitSound);
            }
          }
        }
      }

      int remainingBricks = 0;

      for (int i = 0; i < LINES_OF_BRICKS; i++)
      {
        for (int j = 0; j < BRICKS_PER_LINE; j++)
        {
          if (bricks[i][j].active)
          {
            remainingBricks++;
          }
        }
      }

      if (remainingBricks == 0)
      {
        state = LEVEL_COMPLETE;
      }

      if (ballPos.y >= HEIGHT)
      {
        lives--;

        if (lives <= 0)
        {
          state = GAME_OVER;
        }
        else
        {
          ballLaunched = false;

          paddle.x = WIDTH / 2.0f - 60;

          ballPos.x = paddle.x + paddle.width / 2.0f;
          ballPos.y = paddle.y - ballRadius;

          ballSpeed = (Vector2){0, 0};
        }
      }
    }

    else if (state == GAME_OVER)
    {
      if (IsKeyPressed(KEY_ENTER))
      {
        state = MENU;
        lives = 3;
        score = 0;
        if (hasBgMusic) PlayMusicStream(menuBgMusic);
      }
    }

    else if (state == LEVEL_COMPLETE)
    {
      if (IsKeyPressed(KEY_ENTER))
      {
        state = MENU;
        score = 0;
        lives = 3;
        if (hasBgMusic) PlayMusicStream(menuBgMusic);
      }
    }

    BeginDrawing();

    ClearBackground(darkBg);
    if (state == TITLE)
    {
      int titlesize = 60;
      int titlewidth = MeasureText("DX BALL", titlesize);

      DrawText(
          "DX BALL",
          (WIDTH - titlewidth) / 2,
          200,
          titlesize,
          neonPink);

      int subtitlesize = 30;
      int subtitlewidth =
          MeasureText("CLASSIC BRICK BREAKER", subtitlesize);

      DrawText(
          "CLASSIC BRICK BREAKER",
          (WIDTH - subtitlewidth) / 2,
          290,
          subtitlesize,
          GRAY);

      int startsize = 20;
      int startwidth = MeasureText("PRESS ENTER TO START", startsize);
      DrawText("PRESS ENTER TO START", (WIDTH - startwidth) / 2, 400, startsize, neonCyan);
    }
    else if (state == MENU)
    {
      int titlesize = 60;
      int titlewidth = MeasureText("DX BALL", titlesize);

      DrawText(
          "DX BALL",
          (WIDTH - titlewidth) / 2,
          150,
          titlesize,
          WHITE);

      int playsize = 30;
      int playWidth = MeasureText("PLAY", playsize);

      int helpSize = 30;
      int helpWidth = MeasureText("HELP", helpSize);

      DrawText(
          "PLAY",
          (WIDTH - playWidth) / 2,
          250,
          playsize,
          selectedOption == 0 ? neonPink : WHITE);

      DrawText(
          "HELP",
          (WIDTH - helpWidth) / 2,
          300,
          helpSize,
          selectedOption == 1 ? neonPink : WHITE);
    }

    else if (state == HELP)
    {
      int titlesize = 50;
      int titlewidth = MeasureText("HOW TO PLAY", titlesize);

      DrawText("HOW TO PLAY", (WIDTH - titlewidth) / 2, 100, titlesize, WHITE);

      int line1w = MeasureText("USE LEFT AND RIGHT KEYS TO MOVE THE PADDLE", 20);
      DrawText("USE LEFT AND RIGHT KEYS TO MOVE THE PADDLE", (WIDTH - line1w) / 2, 220, 20, GRAY);

      int line2w = MeasureText("BREAK ALL THE BRICKS TO WIN", 20);
      DrawText("BREAK ALL THE BRICKS TO WIN", (WIDTH - line2w) / 2, 270, 20, GRAY);

      int line3w = MeasureText("PRESS ENTER TO GO BACK", 20);
      DrawText("PRESS ENTER TO GO BACK", (WIDTH - line3w) / 2, 400, 20, neonYellow);
    }

    else if (state == LEVEL_SELECT)
    {
      int titleSize = 50;
      int titleWidth = MeasureText("SELECT LEVEL", titleSize);

      DrawText(
          "SELECT LEVEL",
          (WIDTH - titleWidth) / 2,
          120,
          titleSize,
          WHITE);

      int easySize = 30;
      int easyWidth = MeasureText("EASY", easySize);

      DrawText(
          "EASY",
          (WIDTH - easyWidth) / 2,
          250,
          easySize,
          selectedLevel == 0 ? neonGreen : WHITE);

      int hardSize = 30;
      int hardWidth = MeasureText("HARD", hardSize);

      DrawText(
          "HARD",
          (WIDTH - hardWidth) / 2,
          310,
          hardSize,
          selectedLevel == 1 ? neonPink : WHITE);
    }

    else if (state == PLAYING)
    {
      DrawRectangle(0, 0, WIDTH, 50, topBarColor);
      DrawLine(0, 50, WIDTH, 50, neonCyan);

      DrawText(TextFormat("SCORE: %05d", score), 25, 15, 20, neonGreen);
      DrawText("DX BALL", WIDTH / 2 - 45, 15, 22, RAYWHITE);
      DrawText(TextFormat("LIVES: %d", lives), WIDTH - 140, 15, 20, neonPink);

      DrawRectangle(0, 50, 5, HEIGHT, neonPink);
      DrawRectangle(WIDTH - 5, 50, 5, HEIGHT, neonPink);

      for (int i = 0; i < LINES_OF_BRICKS; i++)
      {
        for (int j = 0; j < BRICKS_PER_LINE; j++)
        {
          if (bricks[i][j].active)
          {
            DrawRectangleRec(bricks[i][j].rect, bricks[i][j].color);
            DrawRectangleLinesEx(bricks[i][j].rect, 1.5f, RAYWHITE);
          }
        }
      }

      DrawRectangleRounded(paddle, 0.4f, 4, neonCyan);
      DrawCircleV(ballPos, ballRadius, neonYellow);
    }

    else if (state == GAME_OVER)
    {
      int goSize = 50;
      int goWidth = MeasureText("GAME OVER", goSize);
      DrawText("GAME OVER", (WIDTH - goWidth) / 2, 220, goSize, neonPink);

      int retSize = 20;
      int retWidth = MeasureText("PRESS ENTER TO RETURN TO MENU", retSize);
      DrawText("PRESS ENTER TO RETURN TO MENU", (WIDTH - retWidth) / 2, 300, retSize, WHITE);
    }

    else if (state == LEVEL_COMPLETE)
    {
      int titleSize = 50;
      int titleWidth = MeasureText("LEVEL COMPLETE!", titleSize);

      DrawText(
          "LEVEL COMPLETE!",
          (WIDTH - titleWidth) / 2,
          200,
          titleSize,
          neonGreen);

      int lcRetSize = 20;
      int lcRetWidth = MeasureText("PRESS ENTER TO RETURN TO MENU", lcRetSize);
      DrawText(
          "PRESS ENTER TO RETURN TO MENU",
          (WIDTH - lcRetWidth) / 2,
          300,
          lcRetSize,
          WHITE);
    }
    EndDrawing();
  }
  if (hasHitSound) UnloadSound(hitSound);
  if (hasBgMusic) UnloadMusicStream(menuBgMusic);
  CloseAudioDevice();

  CloseWindow();

  return 0;
}
