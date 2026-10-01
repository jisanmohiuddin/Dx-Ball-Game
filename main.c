#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define WIDTH 800
#define HEIGHT 600

#define LINES_OF_BRICKS 5
#define BRICKS_PER_LINE 10

#define MAX_BULLETS 20
#define MAX_EXTRA_BALLS 2
#define MAX_SCORE_RECORDS 5
#define MAX_NAME_LENGTH 15
#define SCORE_FILE "scoreboard.txt"

#define COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))

typedef enum
{
  TITLE,
  MENU,
  HELP,
  NAME_INPUT,
  SCOREBOARD,
  ABOUT_US,
  PLAYING,
  PAUSED,
  LEVEL_SELECT,
  GAME_OVER,
  LEVEL_COMPLETE,
  STAGE_SELECT,
} GameState;

typedef enum
{
  POWER_NONE,
  POWER_MULTIBALL,
  POWER_GUN,
  POWER_BIG_PADDLE
} PowerType;

typedef struct Brick
{
  Rectangle rect;
  bool active;
  Color color;
  PowerType powerType;
  bool unbreakable;
} Brick;

typedef struct PowerUp
{
  Vector2 position;
  PowerType type;
  bool active;
  float speed;
} PowerUp;

typedef struct ExtraBall
{
  Vector2 position;
  Vector2 speed;
  bool active;
} ExtraBall;

typedef struct Bullet
{
  Vector2 position;
  bool active;
} Bullet;

typedef struct ScoreRecord
{
  char name[MAX_NAME_LENGTH + 1];
  int score;
} ScoreRecord;

void sortScores(ScoreRecord scores[MAX_SCORE_RECORDS], int count)
{
  for (int i = 0; i < count - 1; i++)
    for (int j = i + 1; j < count; j++)
      if (scores[j].score > scores[i].score)
      {
        ScoreRecord temp = scores[i];
        scores[i] = scores[j];
        scores[j] = temp;
      }
}

void loadScores(ScoreRecord scores[MAX_SCORE_RECORDS], int *scoreCount)
{
  *scoreCount = 0;
  FILE *file = fopen(SCORE_FILE, "r");
  if (!file)
    return;

  char line[128];
  while (*scoreCount < MAX_SCORE_RECORDS && fgets(line, sizeof(line), file))
  {
    char name[MAX_NAME_LENGTH + 1];
    int value;
    if (sscanf(line, "%15[^|]|%d", name, &value) != 2)
      continue;
    strncpy(scores[*scoreCount].name, name, MAX_NAME_LENGTH);
    scores[*scoreCount].name[MAX_NAME_LENGTH] = '\0';
    scores[*scoreCount].score = value;
    (*scoreCount)++;
  }
  fclose(file);
  sortScores(scores, *scoreCount);
}

void saveScores(ScoreRecord scores[MAX_SCORE_RECORDS], int scoreCount)
{
  FILE *file = fopen(SCORE_FILE, "w");
  if (!file)
    return;
  for (int i = 0; i < scoreCount && i < MAX_SCORE_RECORDS; i++)
    fprintf(file, "%s|%d\n", scores[i].name, scores[i].score);
  fclose(file);
}

void addScore(ScoreRecord scores[MAX_SCORE_RECORDS], int *scoreCount, const char *name, int score)
{
  if (!name || name[0] == '\0')
    return;
  if (*scoreCount < MAX_SCORE_RECORDS)
  {
    strncpy(scores[*scoreCount].name, name, MAX_NAME_LENGTH);
    scores[*scoreCount].name[MAX_NAME_LENGTH] = '\0';
    scores[*scoreCount].score = score;
    (*scoreCount)++;
  }
  else
  {
    sortScores(scores, *scoreCount);
    if (score <= scores[MAX_SCORE_RECORDS - 1].score)
      return;
    strncpy(scores[MAX_SCORE_RECORDS - 1].name, name, MAX_NAME_LENGTH);
    scores[MAX_SCORE_RECORDS - 1].name[MAX_NAME_LENGTH] = '\0';
    scores[MAX_SCORE_RECORDS - 1].score = score;
  }
  sortScores(scores, *scoreCount);
  saveScores(scores, *scoreCount);
}

// POWER BRICK CONFIGURATION
void setupPowerBricks(
    Brick bricks[LINES_OF_BRICKS][BRICKS_PER_LINE],
    int currentLevel)
{
  for (int i = 0; i < LINES_OF_BRICKS; i++)
  {
    for (int j = 0; j < BRICKS_PER_LINE; j++)
    {
      bricks[i][j].powerType = POWER_NONE;
    }
  }

  if (currentLevel == 1 || currentLevel == 2)
  {
    bricks[0][1].powerType = POWER_MULTIBALL;
    bricks[2][8].powerType = POWER_MULTIBALL;

    bricks[1][4].powerType = POWER_GUN;
    bricks[4][2].powerType = POWER_GUN;

    bricks[2][3].powerType = POWER_BIG_PADDLE;
    bricks[3][9].powerType = POWER_BIG_PADDLE;
  }
}

// RESET BRICKS
void resetBricks(
    Brick bricks[LINES_OF_BRICKS][BRICKS_PER_LINE],
    int currentLevel, int gameMode)
{
  for (int i = 0; i < LINES_OF_BRICKS; i++)
  {
    for (int j = 0; j < BRICKS_PER_LINE; j++)
    {
      bricks[i][j].active = true;
      bricks[i][j].powerType = POWER_NONE;
      bricks[i][j].unbreakable = false;
    }
  }
  if (currentLevel == 2)
  {
    // easy level-2
    bricks[0][4].unbreakable = true;
    bricks[0][5].unbreakable = true;

    bricks[2][2].unbreakable = true;
    bricks[2][7].unbreakable = true;

    bricks[4][4].unbreakable = true;
    bricks[4][5].unbreakable = true;
  }
  if (currentLevel == 3)
  { // easy level-3
    bricks[0][3].unbreakable = true;
    bricks[0][4].unbreakable = true;
    bricks[0][5].unbreakable = true;
    bricks[0][6].unbreakable = true;
    bricks[4][3].unbreakable = true;
    bricks[4][4].unbreakable = true;
    bricks[4][5].unbreakable = true;
    bricks[4][6].unbreakable = true;
  }

  if (gameMode == 1 && currentLevel == 1)
  {
    // hard level -1
    bricks[0][4].unbreakable = true;
    bricks[0][5].unbreakable = true;
    bricks[1][4].unbreakable = true;
    bricks[2][3].unbreakable = true;
    bricks[2][6].unbreakable = true;
    bricks[3][2].unbreakable = true;
    bricks[3][7].unbreakable = true;
    bricks[4][4].unbreakable = true;
  }

  if (gameMode == 1 && currentLevel == 2)
  {
    // hard level-2
    bricks[0][4].unbreakable = true;
    bricks[0][5].unbreakable = true;
    bricks[1][2].unbreakable = true;
    bricks[1][7].unbreakable = true;
    bricks[2][1].unbreakable = true;
    bricks[2][4].unbreakable = true;
    bricks[2][7].unbreakable = true;
    bricks[2][8].unbreakable = true;
    bricks[3][2].unbreakable = true;
    bricks[3][7].unbreakable = true;
    bricks[4][4].unbreakable = true;
    bricks[4][5].unbreakable = true;
  }

  if (gameMode == 1 && currentLevel == 3)
  {
    // hard level-3
    bricks[0][1].unbreakable = true;
    bricks[0][3].unbreakable = true;
    bricks[0][6].unbreakable = true;
    bricks[0][8].unbreakable = true;
    bricks[1][2].unbreakable = true;
    bricks[1][7].unbreakable = true;
    bricks[2][1].unbreakable = true;
    bricks[2][4].unbreakable = true;
    bricks[2][5].unbreakable = true;
    bricks[2][8].unbreakable = true;
    bricks[3][2].unbreakable = true;
    bricks[3][7].unbreakable = true;
    bricks[4][1].unbreakable = true;
    bricks[4][3].unbreakable = true;
    bricks[4][6].unbreakable = true;
    bricks[4][8].unbreakable = true;
  }
  setupPowerBricks(bricks, currentLevel);
}

// RESET POWER SYSTEM
void resetPowerUps(
    PowerUp *powerUp,
    ExtraBall extraBalls[MAX_EXTRA_BALLS],
    Bullet bullets[MAX_BULLETS],
    bool *gunActive,
    float *gunTimer,
    float *bulletTimer,
    bool *bigPaddleActive,
    float *bigPaddleTimer,
    int *activeBalls)
{
  powerUp->active = false;
  powerUp->type = POWER_NONE;

  for (int i = 0; i < MAX_EXTRA_BALLS; i++)
  {
    extraBalls[i].active = false;
    extraBalls[i].position = (Vector2){0, 0};
    extraBalls[i].speed = (Vector2){0, 0};
  }

  for (int i = 0; i < MAX_BULLETS; i++)
  {
    bullets[i].active = false;
    bullets[i].position = (Vector2){0, 0};
  }

  *gunActive = false;
  *gunTimer = 0.0f;
  *bulletTimer = 0.0f;

  *bigPaddleActive = false;
  *bigPaddleTimer = 0.0f;

  *activeBalls = 1;
}

// RESET GAME
void resetGame(
    Brick bricks[LINES_OF_BRICKS][BRICKS_PER_LINE],
    Rectangle *paddle,
    Vector2 *ballPos,
    Vector2 *ballSpeed,
    bool *ballLaunched,
    int *score,
    int *lives,
    float ballRadius,
    int currentLevel,
    int gameMode)
{
  resetBricks(bricks, currentLevel, gameMode);

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
  *lives = 3;
}

void DrawTextureFit(Texture2D texture, Rectangle box)
{
  float imageWidth = (float)texture.width;
  float imageHeight = (float)texture.height;

  float boxWidth = box.width;
  float boxHeight = box.height;

  float scaleX = boxWidth / imageWidth;
  float scaleY = boxHeight / imageHeight;

  float scale = (scaleX < scaleY) ? scaleX : scaleY;

  float newWidth = imageWidth * scale;
  float newHeight = imageHeight * scale;

  float x = box.x + (boxWidth - newWidth) / 2.0f;
  float y = box.y + (boxHeight - newHeight) / 2.0f;

  DrawTextureEx(
      texture,
      (Vector2){x, y},
      0.0f,
      scale,
      WHITE);
}

   //MAIN PART

int main(void)
{
  GameState state = TITLE;

  int selectedOption = 0;
  int selectedLevel = 0;
  int pauseOption = 0;
  int currentLevel = 1;
  int gameMode = 0;

  int selectedStage = 0;

  int unlockedEasy = 1;
  int unlockedHard = 1;

  ScoreRecord scoreRecords[MAX_SCORE_RECORDS];
  int scoreCount = 0;
  loadScores(scoreRecords, &scoreCount);

  char playerName[MAX_NAME_LENGTH + 1] = "";
  int playerNameLength = 0;
  bool scoreRecorded = false;

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

     //POWER UP

  PowerUp powerUp = {
      {0, 0},
      POWER_NONE,
      false,
      2.5f};

     //EXTRA BALL

  ExtraBall extraBalls[MAX_EXTRA_BALLS];

     //GUN SYSTEM

  Bullet bullets[MAX_BULLETS];

  bool gunActive = false;
  float gunTimer = 0.0f;
  float bulletTimer = 0.0f;

     //BIG PADDLE SYSTEM

  bool bigPaddleActive = false;
  float bigPaddleTimer = 0.0f;

  /* Number of balls currently alive */
  int activeBalls = 1;

  for (int i = 0; i < MAX_EXTRA_BALLS; i++)
  {
    extraBalls[i].active = false;
  }

  for (int i = 0; i < MAX_BULLETS; i++)
  {
    bullets[i].active = false;
  }

     //BRICKS

  float brickWidth =
      (float)(WIDTH - 40) / BRICKS_PER_LINE;

  float brickHeight = 22.0f;

  Brick bricks[LINES_OF_BRICKS][BRICKS_PER_LINE];

     //COLORS

  Color darkBg =
      (Color){18, 16, 26, 255};

  Color topBarColor =
      (Color){24, 22, 36, 255};

  Color neonPink =
      (Color){255, 20, 147, 255};

  Color neonOrange =
      (Color){255, 102, 0, 255};

  Color neonYellow =
      (Color){255, 215, 0, 255};

  Color neonGreen =
      (Color){57, 255, 20, 255};

  Color neonCyan =
      (Color){0, 229, 255, 255};

  Color rowColors[5] =
      {
          neonPink,
          neonOrange,
          neonYellow,
          neonGreen,
          neonCyan};

     //CREATE BRICKS

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
      bricks[i][j].powerType = POWER_NONE;
    }
  }

  InitWindow(WIDTH, HEIGHT, "DX Ball");

  InitAudioDevice();

  Sound hitSound = {0};

  bool hasHitSound = false;

  if (FileExists("assets/audio/hitSound.mp3"))
  {
    hitSound =
        LoadSound("assets/audio/hitSound.mp3");

    hasHitSound = true;
  }

  Music menuBgMusic = {0};

  bool hasBgMusic = false;
  bool soundMuted = false;
  if (FileExists("assets/audio/retroBackground.mp3"))
  {
    menuBgMusic =
        LoadMusicStream(
            "assets/audio/retroBackground.mp3");

    PlayMusicStream(menuBgMusic);

    hasBgMusic = true;
  }

  Texture2D heartTexture = {0};

  bool hasHeartTexture = false;

  Texture2D jisanTexture = {0};
  Texture2D fardinTexture = {0};

  bool hasJisanTexture = false;
  bool hasFardinTexture = false;
  if (FileExists("assets/heart.png"))
  {
    heartTexture =
        LoadTexture("assets/heart.png");
    hasHeartTexture = true;
  }

  if (FileExists("assets/Jisan.png"))
  {
    jisanTexture = LoadTexture("assets/Jisan.png");
    hasJisanTexture = true;
  }

  if (FileExists("assets/Fardin.png"))
  {
    fardinTexture = LoadTexture("assets/Fardin.png");
    hasFardinTexture = true;
  }
 
  resetPowerUps(
      &powerUp,
      extraBalls,
      bullets,
      &gunActive,
      &gunTimer,
      &bulletTimer,
      &bigPaddleActive,
      &bigPaddleTimer,
      &activeBalls);

  resetGame(
      bricks,
      &paddle,
      &ballPos,
      &ballSpeed,
      &ballLaunched,
      &score,
      &lives,
      ballRadius,
      currentLevel,
      gameMode);

  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  
  while (!WindowShouldClose())
  {
    if (IsKeyPressed(KEY_M))
    {
      soundMuted = !soundMuted;

      if (soundMuted)
      {
        if (hasBgMusic)
          PauseMusicStream(menuBgMusic);

        if (hasHitSound)
          StopSound(hitSound);
      }
      else
      {
        if (hasBgMusic)
          ResumeMusicStream(menuBgMusic);
      }
    }
    if (hasBgMusic)
    {
      UpdateMusicStream(menuBgMusic);
    }

    //TITLE

    if (state == TITLE)
    {
      if (IsKeyPressed(KEY_ENTER))
      {
        state = MENU;
      }
    }

 // MENU

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
      {
        selectedOption = 3;
      }

      if (selectedOption > 3)
      {
        selectedOption = 0;
      }

      if (IsKeyPressed(KEY_ENTER))
      {
        if (selectedOption == 0)
        {
          playerName[0] = '\0';
          playerNameLength = 0;

          score = 0;
          scoreRecorded = false;

          state = NAME_INPUT;
        }
        else if (selectedOption == 1)
        {
          state = HELP;
        }
        else if (selectedOption == 2)
        {
          state = SCOREBOARD;
        }
        else if (selectedOption == 3)
        {
          state = ABOUT_US;
        }
      }
    }

       //HELP
    else if (state == HELP)
    {
      if (IsKeyPressed(KEY_ENTER) ||
          IsKeyPressed(KEY_ESCAPE))
      {
        state = MENU;
      }
    }

       //LEVEL SELECT

    else if (state == NAME_INPUT)
    {
      int key = GetCharPressed();
      while (key > 0)
      {
        if (key >= 32 && key <= 126 && playerNameLength < MAX_NAME_LENGTH)
        {
          playerName[playerNameLength++] = (char)key;
          playerName[playerNameLength] = '\0';
        }
        key = GetCharPressed();
      }

      if (IsKeyPressed(KEY_BACKSPACE) && playerNameLength > 0)
      {
        playerNameLength--;
        playerName[playerNameLength] = '\0';
      }

      if (IsKeyPressed(KEY_ENTER) && playerNameLength > 0)
      {
        selectedLevel = 0;
        state = LEVEL_SELECT;
      }
      if (IsKeyPressed(KEY_ESCAPE))
        state = MENU;
    }

    else if (state == SCOREBOARD)
    {
      if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE))
        state = MENU;
    }
    else if (state == ABOUT_US)
    {
      if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE))
        state = MENU;
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
      {
        selectedLevel = 1;
      }

      if (selectedLevel > 1)
      {
        selectedLevel = 0;
      }

      if (IsKeyPressed(KEY_ENTER))
      {
        
        if (selectedLevel == 0)
        {
          gameMode = 0;      // EASY
          selectedStage = 0; // Level 1
          state = STAGE_SELECT;
        }
        
        if (selectedLevel == 1)
        {
          gameMode = 1;      // HARD
          selectedStage = 0; // Level 1
          state = STAGE_SELECT;
        }
      }
    }
   
    else if (state == STAGE_SELECT)
    {
      if (IsKeyPressed(KEY_DOWN))
      {
        selectedStage++;
      }

      if (IsKeyPressed(KEY_UP))
      {
        selectedStage--;
      }

      int maxUnlocked;

      if (gameMode == 0)
        maxUnlocked = unlockedEasy;
      else
        maxUnlocked = unlockedHard;

      if (selectedStage < 0)
        selectedStage = 0;

      if (selectedStage >= maxUnlocked)
        selectedStage = maxUnlocked - 1;

      if (IsKeyPressed(KEY_ENTER))
      {
        currentLevel = selectedStage + 1;

        //EASY
        if (gameMode == 0)
        {
          speedValue =
              5.0f + (currentLevel - 1) * 0.5f;
        }
       //HARD
        else
        {
          speedValue =
              8.0f + (currentLevel - 1) * 0.5f;
        }

        scoreRecorded = false;

        resetPowerUps(
            &powerUp,
            extraBalls,
            bullets,
            &gunActive,
            &gunTimer,
            &bulletTimer,
            &bigPaddleActive,
            &bigPaddleTimer,
            &activeBalls);

        resetGame(
            bricks,
            &paddle,
            &ballPos,
            &ballSpeed,
            &ballLaunched,
            &score,
            &lives,
            ballRadius,
            currentLevel,
            gameMode);

        state = PLAYING;

        if (hasBgMusic)
        {
          StopMusicStream(menuBgMusic);
        }
      }

      if (IsKeyPressed(KEY_ESCAPE))
      {
        state = LEVEL_SELECT;
      }
    }

       //PLAYING

    else if (state == PLAYING)
    {
      if (IsKeyPressed(KEY_P))
      {
        pauseOption = 0;
        state = PAUSED;
      }

      if (IsKeyDown(KEY_RIGHT) &&
          paddle.x + paddle.width < WIDTH - 5)
      {
        paddle.x += 7.0f;
      }

      if (IsKeyDown(KEY_LEFT) &&
          paddle.x > 5)
      {
        paddle.x -= 7.0f;
      }

      if (!ballLaunched)
      {
        ballPos.x =
            paddle.x +
            paddle.width / 2.0f;

        ballPos.y =
            paddle.y -
            ballRadius;

        if (IsKeyPressed(KEY_ENTER))
        {
          ballLaunched = true;

          ballSpeed =
              (Vector2){
                  speedValue,
                  -speedValue};
        }
      }

      if (ballLaunched)
      {
        ballPos.x += ballSpeed.x;
        ballPos.y += ballSpeed.y;
      }

      if (ballPos.x <= (5 + ballRadius) ||
          ballPos.x >=
              (WIDTH - 5 - ballRadius))
      {
        ballSpeed.x *= -1;
      }

      if (ballPos.y <= (50 + ballRadius))
      {
        ballSpeed.y *= -1;
      }

      if (CheckCollisionCircleRec(
              ballPos,
              ballRadius,
              paddle))
      {
        if (ballSpeed.y > 0)
        {
          float hitPos =
              (ballPos.x -
               (paddle.x +
                paddle.width / 2.0f)) /
              (paddle.width / 2.0f);

          ballSpeed.x =
              hitPos * speedValue;

          ballSpeed.y =
              -fabsf(ballSpeed.y);
        }
      }

      for (int b = 0;
           b < MAX_EXTRA_BALLS;
           b++)
      {
        if (!extraBalls[b].active)
          continue;

        extraBalls[b].position.x +=
            extraBalls[b].speed.x;

        extraBalls[b].position.y +=
            extraBalls[b].speed.y;


        if (extraBalls[b].position.x <=
                (5 + ballRadius) ||
            extraBalls[b].position.x >=
                (WIDTH - 5 - ballRadius))
        {
          extraBalls[b].speed.x *= -1;
        }


        if (extraBalls[b].position.y <=
            (50 + ballRadius))
        {
          extraBalls[b].speed.y *= -1;
        }

        if (CheckCollisionCircleRec(
                extraBalls[b].position,
                ballRadius,
                paddle))
        {
          if (extraBalls[b].speed.y > 0)
          {
            float hitPos =
                (extraBalls[b].position.x -
                 (paddle.x +
                  paddle.width / 2.0f)) /
                (paddle.width / 2.0f);

            extraBalls[b].speed.x =
                hitPos * speedValue;

            extraBalls[b].speed.y =
                -fabsf(
                    extraBalls[b].speed.y);
          }
        }
      }

      if (powerUp.active)
      {
        powerUp.position.y +=
            powerUp.speed;

        if (powerUp.position.y > HEIGHT)
        {
          powerUp.active = false;
          powerUp.type = POWER_NONE;
        }

        if (powerUp.active &&
            CheckCollisionPointRec(
                powerUp.position,
                paddle))
        {

          if (powerUp.type ==
              POWER_MULTIBALL)
          {
            extraBalls[0].position =
                ballPos;

            extraBalls[0].speed =
                (Vector2){
                    ballSpeed.x + 2.0f,
                    ballSpeed.y};

            extraBalls[1].position =
                ballPos;

            extraBalls[1].speed =
                (Vector2){
                    ballSpeed.x - 2.0f,
                    ballSpeed.y};

            extraBalls[0].active = true;
            extraBalls[1].active = true;

            activeBalls = 3;
          }

          else if (powerUp.type ==
                   POWER_GUN)
          {
            gunActive = true;

            gunTimer = 5.0f;

            bulletTimer = 0.0f;
          }

          else if (powerUp.type ==
                   POWER_BIG_PADDLE)
          {
            bigPaddleActive = true;

            bigPaddleTimer = 6.0f;

            paddle.width = 180;
          }

          powerUp.active = false;

          powerUp.type =
              POWER_NONE;
        }
      }

      if (bigPaddleActive)
      {
        bigPaddleTimer -=
            GetFrameTime();

        if (bigPaddleTimer <= 0)
        {
          bigPaddleActive = false;

          bigPaddleTimer = 0;

          paddle.width = 120;

          if (paddle.x +
                  paddle.width >
              WIDTH - 5)
          {
            paddle.x =
                WIDTH -
                5 -
                paddle.width;
          }
        }
      }

      if (gunActive)
      {
        gunTimer -=
            GetFrameTime();

        if (gunTimer <= 0)
        {
          gunActive = false;

          gunTimer = 0;

          bulletTimer = 0;
        }
    }

      if (gunActive)
      {
        bulletTimer -=
            GetFrameTime();

        if (bulletTimer <= 0)
        {
          for (int i = 0;
               i < MAX_BULLETS;
               i++)
          {
            if (!bullets[i].active)
            {
              bullets[i].position =
                  (Vector2){
                      paddle.x +
                          paddle.width / 2.0f,
                      paddle.y};

              bullets[i].active = true;

              break;
            }
          }
          bulletTimer = 0.20f;
        }
      }

      for (int i = 0;
           i < MAX_BULLETS;
           i++)
      {
        if (!bullets[i].active)
          continue;

        bullets[i].position.y -=
            9.0f;

        if (bullets[i].position.y < 50)
        {
          bullets[i].active = false;

          continue;
        }

        for (int r = 0;
             r < LINES_OF_BRICKS;
             r++)
        {
          for (int c = 0;
               c < BRICKS_PER_LINE;
               c++)
          {
            if (!bricks[r][c].active)
              continue;

            Rectangle bulletRect =
                {
                    bullets[i].position.x - 2,
                    bullets[i].position.y - 6,
                    4,
                    12};

            if (CheckCollisionRecs(
                    bulletRect,
                    bricks[r][c].rect))
            {
              if (!bricks[r][c].unbreakable)
              {
                bricks[r][c].active = false;

                if (gameMode == 0)
                  score += 10;
                else
                  score += 15;
              }
              bullets[i].active =
                  false;

              if (hasHitSound && !soundMuted)
              {
                PlaySound(hitSound);
              }
              break;
            }
          }

          if (!bullets[i].active)
            break;
        }
      }

      for (int i = 0;
           i < LINES_OF_BRICKS;
           i++)
      {
        for (int j = 0;
             j < BRICKS_PER_LINE;
             j++)
        {
          if (bricks[i][j].active)
          {
            if (CheckCollisionCircleRec(
                    ballPos,
                    ballRadius,
                    bricks[i][j].rect))
            {
              if (!bricks[i][j].unbreakable)
              {
                bricks[i][j].active = false;

                if (gameMode == 0)
                  score += 10;
                else
                  score += 15;
              }

              ballSpeed.y *= -1;

              if (hasHitSound && !soundMuted)
              {
                PlaySound(hitSound);
              }

              if (bricks[i][j].powerType !=
                      POWER_NONE &&
                  !powerUp.active)
              {
                powerUp.position =
                    (Vector2){
                        bricks[i][j].rect.x +
                            bricks[i][j].rect.width / 2.0f,

                        bricks[i][j].rect.y +
                            bricks[i][j].rect.height};

                powerUp.type =
                    bricks[i][j].powerType;

                powerUp.active =
                    true;

                powerUp.speed =
                    2.5f;
              }
            }
          }
        }
      }

      for (int b = 0;
           b < MAX_EXTRA_BALLS;
           b++)
      {
        if (!extraBalls[b].active)
          continue;

        for (int i = 0;
             i < LINES_OF_BRICKS;
             i++)
        {
          for (int j = 0;
               j < BRICKS_PER_LINE;
               j++)
          {
            if (!bricks[i][j].active)
              continue;

            if (CheckCollisionCircleRec(
                    extraBalls[b].position,
                    ballRadius,
                    bricks[i][j].rect))
            {
              if (!bricks[i][j].unbreakable)
              {
                bricks[i][j].active = false;

                if (gameMode == 0)
                  score += 10;
                else
                  score += 15;
              }
              extraBalls[b].speed.y *=
                  -1;

              if (hasHitSound && !soundMuted)
              {
                PlaySound(hitSound);
              }

              if (bricks[i][j].powerType !=
                      POWER_NONE &&
                  !powerUp.active)
              {
                powerUp.position =
                    (Vector2){
                        bricks[i][j].rect.x +
                            bricks[i][j].rect.width / 2.0f,

                        bricks[i][j].rect.y +
                            bricks[i][j].rect.height};

                powerUp.type =
                    bricks[i][j].powerType;

                powerUp.active =
                    true;

                powerUp.speed =
                    2.5f;
              }

              break;
            }
          }
        }
      }

      //Remaining Brick Count
      int remainingBricks = 0;

      for (int i = 0;
           i < LINES_OF_BRICKS;
           i++)
      {
        for (int j = 0;
             j < BRICKS_PER_LINE;
             j++)
        {
          if (bricks[i][j].active &&
              !bricks[i][j].unbreakable)
          {
            remainingBricks++;
          }
        }
      }

      if (remainingBricks == 0)
      {
        int levelBonus;

        if (gameMode == 0)
        {
          levelBonus = currentLevel * 50;
        }
        else
        {
          levelBonus = 50 + currentLevel * 50;
        }
        score += levelBonus;

        if (gameMode == 0)
        {
          if (currentLevel == unlockedEasy && unlockedEasy < 3)
          {
            unlockedEasy++;
          }
        }
        else
        {
          if (currentLevel == unlockedHard && unlockedHard < 3)
          {
            unlockedHard++;
          }
        }
        state = LEVEL_COMPLETE;
      }

      if (ballLaunched &&
          ballPos.y >= HEIGHT)
      {
        ballLaunched = false;

        ballPos =
            (Vector2){
                paddle.x +
                    paddle.width / 2.0f,

                paddle.y -
                    ballRadius};

        ballSpeed =
            (Vector2){0, 0};

        if (activeBalls > 1)
        {
          activeBalls = 0;

          for (int b = 0;
               b < MAX_EXTRA_BALLS;
               b++)
          {
            if (extraBalls[b].active)
            {
              activeBalls++;
            }
          }

          if (activeBalls > 0)
          {
            for (int b = 0;
                 b < MAX_EXTRA_BALLS;
                 b++)
            {
              if (extraBalls[b].active)
              {
                ballPos =
                    extraBalls[b].position;

                ballSpeed =
                    extraBalls[b].speed;

                extraBalls[b].active =
                    false;

                activeBalls--;

                ballLaunched =
                    true;

                break;
              }
            }
          }
          else
          {
            lives--;

            if (lives <= 0)
            {
              if (!scoreRecorded)
              {
                addScore(scoreRecords, &scoreCount, playerName, score);
                scoreRecorded = true;
              }
              state = GAME_OVER;
            }
            else
            {
              ballLaunched =
                  false;

              paddle.x =
                  WIDTH / 2.0f -
                  paddle.width / 2.0f;

              ballPos =
                  (Vector2){
                      paddle.x +
                          paddle.width / 2.0f,

                      paddle.y -
                          ballRadius};

              ballSpeed =
                  (Vector2){0, 0};
            }
          }
        }
        else
        {
          lives--;

          if (lives <= 0)
          {
            if (!scoreRecorded)
            {
              addScore(scoreRecords, &scoreCount, playerName, score);
              scoreRecorded = true;
            }
            state = GAME_OVER;
          }
          else
          {
            paddle.x =
                WIDTH / 2.0f -
                paddle.width / 2.0f;

            ballPos =
                (Vector2){
                    paddle.x +
                        paddle.width / 2.0f,

                    paddle.y -
                        ballRadius};

            ballSpeed =
                (Vector2){0, 0};
          }
        }
      }

      for (int b = 0;
           b < MAX_EXTRA_BALLS;
           b++)
      {
        if (extraBalls[b].active &&
            extraBalls[b].position.y >= HEIGHT)
        {
          extraBalls[b].active = false;

          activeBalls--;

          if (activeBalls <= 0)
          {
            lives--;

            if (lives <= 0)
            {
              if (!scoreRecorded)
              {
                addScore(scoreRecords, &scoreCount, playerName, score);
                scoreRecorded = true;
              }
              state = GAME_OVER;
            }
            else
            {
              activeBalls = 1;

              ballLaunched =
                  false;

              paddle.x =
                  WIDTH / 2.0f -
                  paddle.width / 2.0f;

              ballPos =
                  (Vector2){
                      paddle.x +
                          paddle.width / 2.0f,

                      paddle.y -
                          ballRadius};

              ballSpeed =
                  (Vector2){0, 0};
            }
          }
        }
      }
    }
    else if (state == PAUSED)
    {
      if (IsKeyPressed(KEY_P))
      {
        state = PLAYING;
      }

      if (IsKeyPressed(KEY_DOWN))
      {
        pauseOption++;

        if (pauseOption > 1)
          pauseOption = 0;
      }

      if (IsKeyPressed(KEY_UP))
      {
        pauseOption--;

        if (pauseOption < 0)
          pauseOption = 1;
      }
      if (IsKeyPressed(KEY_ENTER))
      {
        if (pauseOption == 0)
        {
          state = PLAYING;
        }
        else if (pauseOption == 1)
        {
          state = MENU;
          if (hasBgMusic)
          {
            PlayMusicStream(menuBgMusic);
          }
        }
      }
    }
    
    //Game Over
    else if (state == GAME_OVER)
    {
      if (IsKeyPressed(KEY_ENTER))
      {
        if (!scoreRecorded)
        {
          addScore(scoreRecords, &scoreCount, playerName, score);
          scoreRecorded = true;
        }
        state = MENU;

        lives = 3;

        score = 0;

        if (hasBgMusic)
        {
          PlayMusicStream(menuBgMusic);
        }
      }
    }
//Level Complete

    else if (state == LEVEL_COMPLETE)
    {
      if (IsKeyPressed(KEY_ESCAPE))
      {
        state = STAGE_SELECT;
        selectedStage = currentLevel - 1;
      }
      if (IsKeyPressed(KEY_ENTER))
      {
        if (currentLevel == 3 && !scoreRecorded)
        {
          addScore(
              scoreRecords,
              &scoreCount,
              playerName,
              score);

          scoreRecorded = true;
        }

        state = STAGE_SELECT;
        selectedStage = currentLevel;

        if (hasBgMusic)
        {
          PlayMusicStream(menuBgMusic);
        }
      }
    }

    BeginDrawing();
    ClearBackground(darkBg);
    if (state == TITLE)
    {
      int titlesize = 60;

      int titlewidth =
          MeasureText(
              "DX BALL",
              titlesize);

      DrawText(
          "DX BALL",
          (WIDTH - titlewidth) / 2,
          200,
          titlesize,
          neonPink);

      int subtitlesize = 30;

      int subtitlewidth =
          MeasureText(
              "CLASSIC BRICK BREAKER",
              subtitlesize);

      DrawText(
          "CLASSIC BRICK BREAKER",
          (WIDTH - subtitlewidth) / 2,
          290,
          subtitlesize,
          GRAY);

      int startsize = 20;

      int startwidth =
          MeasureText(
              "PRESS ENTER TO START",
              startsize);

      DrawText(
          "PRESS ENTER TO START",
          (WIDTH - startwidth) / 2,
          400,
          startsize,
          neonCyan);
    }

    else if (state == MENU)
    {
      int titlesize = 60;

      int titlewidth =
          MeasureText(
              "DX BALL",
              titlesize);

      DrawText(
          "DX BALL",
          (WIDTH - titlewidth) / 2,
          150,
          titlesize,
          WHITE);

      int playsize = 30;

      int playWidth =
          MeasureText(
              "PLAY",
              playsize);

      int helpSize = 30;

      int helpWidth =
          MeasureText(
              "HELP",
              helpSize);

      DrawText(
          "PLAY",
          (WIDTH - playWidth) / 2,
          250,
          playsize,
          selectedOption == 0
              ? neonPink
              : WHITE);

      DrawText(
          "HELP",
          (WIDTH - helpWidth) / 2,
          300,
          helpSize,
          selectedOption == 1
              ? neonPink
              : WHITE);

      int scoreSize = 30;
      int scoreWidth = MeasureText("SCORE", scoreSize);

      DrawText(
          "SCORE",
          (WIDTH - scoreWidth) / 2,
          350,
          scoreSize,
          selectedOption == 2
              ? neonPink
              : WHITE);
      int aboutSize = 30;
      int aboutWidth = MeasureText("ABOUT US", aboutSize);

      DrawText(
          "ABOUT US",
          (WIDTH - aboutWidth) / 2,
          400,
          aboutSize,
          selectedOption == 3
              ? neonPink
              : WHITE);
    }

    else if (state == HELP)
    {
      int titleWidth = MeasureText("HOW TO PLAY", 50);
      DrawText("HOW TO PLAY", (WIDTH - titleWidth) / 2, 60, 50, WHITE);

      const char *lines[] = {
          "LEFT / RIGHT : MOVE THE PADDLE",
          "ENTER : LAUNCH THE BALL",
          "P : PAUSE        ESC : LEAVE THE LEVEL",
          "BREAK ALL BREAKABLE BRICKS TO WIN",
          "GRAY BRICKS CANNOT BE DESTROYED"};

      for (int i = 0; i < COUNT(lines); i++)
      {
        int w = MeasureText(lines[i], 20);
        DrawText(lines[i], (WIDTH - w) / 2, 150 + i * 35, 20, GRAY);
      }

      DrawCircle(230, 355, 10, neonYellow);
      DrawText("3", 225, 347, 16, BLACK);
      DrawText("MULTIBALL", 255, 346, 20, WHITE);

      DrawCircle(230, 395, 10, neonOrange);
      DrawText("G", 225, 387, 16, BLACK);
      DrawText("GUN (5 SEC)", 255, 386, 20, WHITE);

      DrawCircle(230, 435, 10, neonGreen);
      DrawText("+", 225, 427, 16, BLACK);
      DrawText("BIG PADDLE (6 SEC)", 255, 426, 20, WHITE);

      int backW = MeasureText("PRESS ENTER TO GO BACK", 20);
      DrawText("PRESS ENTER TO GO BACK", (WIDTH - backW) / 2, 520, 20, neonYellow);
    }

    else if (state == NAME_INPUT)
    {
      const char *title = "ENTER YOUR NAME";
      int titleSize = 45;
      int titleWidth = MeasureText(title, titleSize);
      DrawText(title, (WIDTH - titleWidth) / 2, 120, titleSize, WHITE);

      Rectangle nameBox = {170, 235, 460, 60};
      DrawRectangleLinesEx(nameBox, 3, neonCyan);

      const char *shownName = playerNameLength > 0 ? playerName : "_";
      int nameWidth = MeasureText(shownName, 30);
      DrawText(shownName, (WIDTH - nameWidth) / 2, 250, 30, neonYellow);

      DrawText("TYPE YOUR NAME  |  BACKSPACE TO EDIT", 205, 335, 18, GRAY);
      DrawText("PRESS ENTER TO CONTINUE", 260, 390, 20, neonGreen);
      DrawText("ESC: BACK TO MENU", 310, 430, 18, GRAY);
    }

    else if (state == SCOREBOARD)
    {
      const char *title = "HIGH SCORES";
      int titleSize = 50;
      int titleWidth = MeasureText(title, titleSize);
      DrawText(title, (WIDTH - titleWidth) / 2, 70, titleSize, neonYellow);

      if (scoreCount == 0)
      {
        const char *emptyText = "NO SCORES YET";
        int emptyWidth = MeasureText(emptyText, 25);
        DrawText(emptyText, (WIDTH - emptyWidth) / 2, 260, 25, GRAY);
      }
      else
      {
        for (int i = 0; i < scoreCount; i++)
        {
          int y = 160 + i * 55;
          Color rowColor = (i == 0) ? neonYellow : WHITE;
          DrawText(TextFormat("%d.", i + 1), 150, y, 25, rowColor);
          DrawText(scoreRecords[i].name, 220, y, 25, rowColor);
          DrawText(TextFormat("%05d", scoreRecords[i].score), 540, y, 25,
                   i == 0 ? neonYellow : neonGreen);
        }
      }

      if (scoreCount > 0)
        DrawText(TextFormat("HIGH SCORE: %05d", scoreRecords[0].score),
                 275, 465, 22, neonPink);

      DrawText("PRESS ENTER TO RETURN", 285, 530, 20, neonCyan);
    }

    else if (state == ABOUT_US)
    {
      const char *title = "ABOUT US";
      int titleSize = 50;
      int titleWidth = MeasureText(title, titleSize);
      DrawText(title, (WIDTH - titleWidth) / 2, 45, titleSize, neonYellow);

      DrawRectangleLinesEx((Rectangle){65, 115, 670, 405}, 3, neonCyan);

      Rectangle jisanBox = {100, 135, 250, 210};
      if (hasJisanTexture)
      {
        Rectangle source = {0, 0, (float)jisanTexture.width, (float)jisanTexture.height};
        DrawTextureFit(jisanTexture, jisanBox);
      }
      else
      {
        DrawRectangleLinesEx(jisanBox, 2, GRAY);
        DrawText("Jisan.png", 155, 225, 20, GRAY);
      }

      const char *jisanName = "Muhammad Mohiuddin Jisan";
      int jisanNameSize = 20;
      int jisanNameWidth = MeasureText(jisanName, jisanNameSize);
      DrawText(jisanName, 225 - jisanNameWidth / 2, 365, jisanNameSize, WHITE);

      const char *jisanBatch = "BUET CSE '25";
      int jisanBatchWidth = MeasureText(jisanBatch, 18);
      DrawText(jisanBatch, 225 - jisanBatchWidth / 2, 392, 18, neonYellow);

      Rectangle fardinBox = {450, 135, 250, 210};
      if (hasFardinTexture)
      {
        Rectangle source = {0, 0, (float)fardinTexture.width, (float)fardinTexture.height};
        DrawTextureFit(fardinTexture, fardinBox);
      }
      else
      {
        DrawRectangleLinesEx(fardinBox, 2, GRAY);
        DrawText("Fardin.png", 505, 225, 20, GRAY);
      }

      const char *fardinName = "Fardin Jobayer Purno";
      int fardinNameSize = 20;
      int fardinNameWidth = MeasureText(fardinName, fardinNameSize);
      DrawText(fardinName, 575 - fardinNameWidth / 2, 365, fardinNameSize, WHITE);

      const char *fardinBatch = "BUET CSE '25";
      int fardinBatchWidth = MeasureText(fardinBatch, 18);
      DrawText(fardinBatch, 575 - fardinBatchWidth / 2, 392, 18, neonYellow);
      DrawText("Advisor : Junaed Younus Khan", 235, 430, 25, WHITE);
      DrawText("PRESS ENTER OR ESC TO RETURN TO MENU", 225, 475, 18, neonCyan);
    }

    else if (state == LEVEL_SELECT)
    {
      int titleSize = 50;

      int titleWidth =
          MeasureText(
              "SELECT LEVEL",
              titleSize);

      DrawText(
          "SELECT LEVEL",
          (WIDTH - titleWidth) / 2,
          120,
          titleSize,
          WHITE);

      int easySize = 30;

      int easyWidth =
          MeasureText(
              "EASY",
              easySize);

      DrawText(
          "EASY",
          (WIDTH - easyWidth) / 2,
          250,
          easySize,
          selectedLevel == 0
              ? neonGreen
              : WHITE);

      int hardSize = 30;

      int hardWidth =
          MeasureText(
              "HARD",
              hardSize);

      DrawText(
          "HARD",
          (WIDTH - hardWidth) / 2,
          310,
          hardSize,
          selectedLevel == 1
              ? neonPink
              : WHITE);
    }

    else if (state == STAGE_SELECT)
    {
      ClearBackground(darkBg);

      DrawText(
          gameMode == 0 ? "EASY MODE" : "HARD MODE",
          280, 80, 40, WHITE);

      DrawText(
          "SELECT LEVEL",
          300, 140, 30, WHITE);

      int maxUnlocked;

      if (gameMode == 0)
        maxUnlocked = unlockedEasy;
      else
        maxUnlocked = unlockedHard;

      for (int i = 0; i < 3; i++)
      {
        int y = 220 + i * 80;

        if (i < maxUnlocked)
        {
          Color levelColor = (i == selectedStage) ? YELLOW : WHITE;

          DrawText(
              TextFormat("LEVEL %d", i + 1),
              320, y, 30, levelColor);
        }
        else
        {
          DrawText(
              TextFormat("LEVEL %d - LOCKED", i + 1),
              270, y, 30, GRAY);
        }
      }

      DrawText(
          "UP/DOWN: SELECT    ENTER: PLAY    ESC: BACK",
          170, 500, 20, LIGHTGRAY);
    }

    else if (state == PLAYING)
    {
      DrawRectangle(
          0,
          0,
          WIDTH,
          50,
          topBarColor);

      DrawLine(
          0,
          50,
          WIDTH,
          50,
          neonCyan);

      DrawText(
          TextFormat(
              "SCORE: %05d",
              score),
          25,
          15,
          20,
          neonGreen);

      DrawText(
          "DX BALL",
          WIDTH / 2 - 45,
          15,
          22,
          RAYWHITE);

      if (hasHeartTexture)
      {
        for (int i = 0;
             i < lives;
             i++)
        {
          Rectangle source =
              {
                  0,
                  0,
                  (float)heartTexture.width,
                  (float)heartTexture.height};

          Rectangle dest =
              {
                  WIDTH - 105 +
                      i * 30,
                  12,
                  24,
                  24};

          DrawTexturePro(
              heartTexture,
              source,
              dest,
              (Vector2){0, 0},
              0.0f,
              WHITE);
        }
      }
      else
      {
        DrawText(
            TextFormat(
                "LIVES: %d",
                lives),
            WIDTH - 140,
            15,
            20,
            neonPink);
      }

      DrawRectangle(
          0,
          50,
          5,
          HEIGHT,
          neonPink);

      DrawRectangle(
          WIDTH - 5,
          50,
          5,
          HEIGHT,
          neonPink);

      for (int i = 0;
           i < LINES_OF_BRICKS;
           i++)
      {
        for (int j = 0;
             j < BRICKS_PER_LINE;
             j++)
        {
          if (bricks[i][j].active)
          {
            DrawRectangleRec(
                bricks[i][j].rect,
                bricks[i][j].color);

            DrawRectangleLinesEx(
                bricks[i][j].rect,
                1.5f,
                RAYWHITE);
            Color brickColor;

            if (bricks[i][j].unbreakable)
            {
              brickColor = DARKGRAY;
            }
            else
            {
              brickColor = bricks[i][j].color;
            }

            DrawRectangleRec(
                bricks[i][j].rect,
                brickColor);
          }
        }
      }

      if (powerUp.active)
      {
        Color powerColor = WHITE;

        if (powerUp.type ==
            POWER_MULTIBALL)
        {
          powerColor = neonYellow;
        }
        else if (powerUp.type ==
                 POWER_GUN)
        {
          powerColor = neonOrange;
        }
        else if (powerUp.type ==
                 POWER_BIG_PADDLE)
        {
          powerColor = neonGreen;
        }

        DrawCircleV(
            powerUp.position,
            10,
            powerColor);

        if (powerUp.type ==
            POWER_MULTIBALL)
        {
          DrawText(
              "3",
              powerUp.position.x - 5,
              powerUp.position.y - 8,
              16,
              BLACK);
        }
        else if (powerUp.type ==
                 POWER_GUN)
        {
          DrawText(
              "G",
              powerUp.position.x - 5,
              powerUp.position.y - 8,
              16,
              BLACK);
        }
        else if (powerUp.type ==
                 POWER_BIG_PADDLE)
        {
          DrawText(
              "+",
              powerUp.position.x - 5,
              powerUp.position.y - 8,
              16,
              BLACK);
        }
      }

      DrawRectangleRounded(
          paddle,
          0.4f,
          4,
          neonCyan);

      if (ballLaunched ||
          !ballLaunched)
      {
        DrawCircleV(
            ballPos,
            ballRadius,
            neonYellow);
      }

      for (int b = 0;
           b < MAX_EXTRA_BALLS;
           b++)
      {
        if (extraBalls[b].active)
        {
          DrawCircleV(
              extraBalls[b].position,
              ballRadius,
              neonYellow);
        }
      }

      for (int i = 0;
           i < MAX_BULLETS;
           i++)
      {
        if (bullets[i].active)
        {
          DrawRectangle(
              bullets[i].position.x - 2,
              bullets[i].position.y - 7,
              4,
              14,
              neonOrange);
        }
      }

      if (gunActive)
      {
        DrawText(
            TextFormat(
                "GUN %.1f",
                gunTimer),
            25,
            520,
            18,
            neonOrange);
      }

      if (bigPaddleActive)
      {
        DrawText(
            TextFormat(
                "BIG PADDLE %.1f",
                bigPaddleTimer),
            25,
            545,
            18,
            neonGreen);
      }
    }

    else if (state == PAUSED)
    {
      DrawRectangle(
          0,
          0,
          WIDTH,
          HEIGHT,
          (Color){10, 10, 18, 240});

      const char *pauseTitle = "GAME PAUSED";
      int titleSize = 50;
      int titleWidth = MeasureText(pauseTitle, titleSize);

      DrawText(
          pauseTitle,
          (WIDTH - titleWidth) / 2,
          150,
          titleSize,
          neonYellow);

      const char *resumeText = "RESUME";
      int resumeSize = 30;
      int resumeWidth = MeasureText(resumeText, resumeSize);

      DrawText(
          resumeText,
          (WIDTH - resumeWidth) / 2,
          270,
          resumeSize,
          pauseOption == 0
              ? neonGreen
              : WHITE);

      const char *menuText = "MAIN MENU";
      int menuSize = 30;
      int menuWidth = MeasureText(menuText, menuSize);

      DrawText(
          menuText,
          (WIDTH - menuWidth) / 2,
          330,
          menuSize,
          pauseOption == 1
              ? neonPink
              : WHITE);

      DrawText(
          "UP/DOWN: SELECT    ENTER: CONFIRM",
          210,
          430,
          18,
          GRAY);

      DrawText(
          "P: RESUME",
          335,
          470,
          18,
          neonCyan);
    }

    else if (state == GAME_OVER)
    {
      int goSize = 50;

      int goWidth =
          MeasureText(
              "GAME OVER",
              goSize);

      DrawText(
          "GAME OVER",
          (WIDTH - goWidth) / 2,
          220,
          goSize,
          neonPink);

      int scoreSize = 25;

      char scoreText[50];

      sprintf(
          scoreText,
          "YOUR SCORE: %d",
          score);

      int scoreWidth =
          MeasureText(
              scoreText,
              scoreSize);

      DrawText(
          scoreText,
          (WIDTH - scoreWidth) / 2,
          285,
          scoreSize,
          WHITE);

      int retSize = 20;

      int retWidth =
          MeasureText(
              "PRESS ENTER TO RETURN TO MENU",
              retSize);

      DrawText(
          "PRESS ENTER TO RETURN TO MENU",
          (WIDTH - retWidth) / 2,
          340,
          retSize,
          WHITE);
    }

    else if (state == LEVEL_COMPLETE)
    {
      int titleSize = 50;
      char levelText[50];
      sprintf(levelText, "LEVEL %d COMPLETE!", currentLevel);
      int titleWidth = MeasureText(levelText, titleSize);
      DrawText(
          levelText,
          (WIDTH - titleWidth) / 2,
          140,
          titleSize,
          neonGreen);
      char scoreText[50];
      sprintf(scoreText, "SCORE: %05d", score);
      int scoreWidth = MeasureText(scoreText, 30);
      DrawText(
          scoreText,
          (WIDTH - scoreWidth) / 2,
          240,
          30,
          neonYellow);
      int levelBonus;
      if (gameMode == 0)
        levelBonus = currentLevel * 50;
      else
        levelBonus = 50 + currentLevel * 50;
      char bonusText[50];
      sprintf(bonusText, "LEVEL BONUS: +%d", levelBonus);
      int bonusWidth = MeasureText(bonusText, 22);
      DrawText(
          bonusText,
          (WIDTH - bonusWidth) / 2,
          290,
          22,
          neonCyan);
      if (currentLevel < 3)
      {
        const char *nextText =
            "PRESS ENTER FOR NEXT LEVEL";
        int nextWidth =
            MeasureText(nextText, 20);
        DrawText(
            nextText,
            (WIDTH - nextWidth) / 2,
            390,
            20,
            WHITE);
      }
      else
      {
        const char *nextText =
            "ALL LEVELS COMPLETE!";
        int nextWidth =
            MeasureText(nextText, 20);
        DrawText(
            nextText,
            (WIDTH - nextWidth) / 2,
            390,
            20,
            neonPink);
      }
      DrawText(
          "ESC: BACK",
          350,
          450,
          20,
          GRAY);
    }

    EndDrawing();
  }

  if (hasHitSound)
  {
    UnloadSound(hitSound);
  }

  if (hasBgMusic)
  {
    UnloadMusicStream(menuBgMusic);
  }

  if (hasHeartTexture)
  {
    UnloadTexture(heartTexture);
  }
  if (hasJisanTexture)
  {
    UnloadTexture(jisanTexture);
  }

  if (hasFardinTexture)
  {
    UnloadTexture(fardinTexture);
  }

  CloseAudioDevice();

  CloseWindow();

  return 0;
}