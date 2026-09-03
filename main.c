#include "raylib.h"
#include <stdbool.h>

#define screenWidth 800
#define screenHeight 600

#define LINES_OF_BRICKS 5
#define BRICKS_PER_LINE 10

typedef struct Brick {
    Rectangle rect;
    bool active;
    Color color;
} Brick;

int main(void) {
    bool ballLaunched = false;
    Vector2 ballSpeed = { 0, 0 };
    float speedValue = 5.0f;
    InitWindow(screenWidth, screenHeight, "DX Ball");
    SetTargetFPS(60);

    Color darkBg      = (Color){ 18, 16, 26, 255 };
    Color topBarColor = (Color){ 24, 22, 36, 255 };
    Color neonPink    = (Color){ 255, 20, 147, 255 };
    Color neonOrange  = (Color){ 255, 102, 0, 255 };
    Color neonYellow  = (Color){ 255, 215, 0, 255 };
    Color neonGreen   = (Color){ 57, 255, 20, 255 };
    Color neonCyan    = (Color){ 0, 229, 255, 255 };
    
    Rectangle paddle = { screenWidth/2.0f - 60, screenHeight - 35, 120, 15 };
    float ballRadius = 9.0;
    Vector2 ballPos = { paddle.x + (paddle.width / 2.0), paddle.y - ballRadius };
    int score = 0;
    int lives = 3;

    Brick bricks[LINES_OF_BRICKS][BRICKS_PER_LINE];
    float brickWidth = (float)(screenWidth-40)/BRICKS_PER_LINE;
    float brickHeight = 22.0;
    Color rowColors[5]={ neonPink, neonOrange, neonYellow, neonGreen, neonCyan };

    for(int i=0;i<LINES_OF_BRICKS;i++){
        for(int j=0;j<BRICKS_PER_LINE;j++){
                bricks[i][j].rect =(Rectangle){
                    20+j*brickWidth,80+i*(brickHeight+6),brickWidth-5,brickHeight
                };
                bricks[i][j].active = true;
                bricks[i][j].color = rowColors[i];
        }
    }

    while(!WindowShouldClose()){
        if(IsKeyDown(KEY_RIGHT)&&(paddle.x+paddle.width)<screenWidth-5){
            paddle.x+=7.0;
        }
        if (IsKeyDown(KEY_LEFT) && paddle.x > 5){
            paddle.x+=-7.0;
        }

        if (!ballLaunched) {
            ballPos.x = paddle.x + (paddle.width / 2.0f);
            ballPos.y = paddle.y - ballRadius;

            if (IsKeyPressed(KEY_ENTER)) {
                ballLaunched = true;
                ballSpeed = (Vector2){ speedValue, -speedValue };
            }
        }
        
        if(ballLaunched){
            ballPos.x += ballSpeed.x;
            ballPos.y += ballSpeed.y;
            if (ballPos.x <= (5 + ballRadius) || ballPos.x >= (screenWidth - 5 - ballRadius)) {
                ballSpeed.x *= -1.0f; 
            }
            if (ballPos.y <= (50 + ballRadius)) {
                ballSpeed.y *= -1.0f;
            }

            if (CheckCollisionCircleRec(ballPos, ballRadius, paddle)) {
                if (ballSpeed.y > 0) { 
                    ballSpeed.y *= -1.0f; 
                    float hitPos = (ballPos.x - (paddle.x + paddle.width / 2.0f)) / (paddle.width / 2.0f);
                    ballSpeed.x = hitPos * speedValue;
                }
            }
            for (int i = 0; i < LINES_OF_BRICKS; i++) {
                for (int j = 0; j < BRICKS_PER_LINE; j++) {
                    if (bricks[i][j].active) {
                        if (CheckCollisionCircleRec(ballPos, ballRadius, bricks[i][j].rect)) {
                            bricks[i][j].active = false; 
                            ballSpeed.y *= -1.0f;        
                            score += 10;                  
                        }
                    }
                }
            }
            if (ballPos.y >= screenHeight) {
                lives--;
                ballLaunched = false;
                paddle.x = screenWidth / 2.0f - 60;
                ballPos.x = paddle.x + (paddle.width / 2.0f);
                ballPos.y = paddle.y - ballRadius;
            }

        }

        BeginDrawing();
            ClearBackground(darkBg);
            DrawRectangle(0,0,screenWidth,50,topBarColor);
            DrawLine(0,50,screenWidth,50,neonCyan);

            DrawText(TextFormat("SCORE: %05d", score), 25, 15, 20, neonGreen);
            DrawText("DX BALL", screenWidth/2 - 45, 15, 22, RAYWHITE);
            DrawText(TextFormat("LIVES: %d", lives), screenWidth - 140, 15, 20, neonPink);

            DrawRectangle(0,50,5,screenHeight,neonPink);
            DrawRectangle(screenWidth-5,50,5,screenHeight,neonPink);

            for(int i=0;i<LINES_OF_BRICKS;i++){
                for(int j=0;j<BRICKS_PER_LINE;j++){
                    if(bricks[i][j].active){
                        DrawRectangleRec(bricks[i][j].rect, bricks[i][j].color);
                        DrawRectangleLinesEx(bricks[i][j].rect, 1.5, RAYWHITE);
                
                    }
                }
            }

            DrawRectangleRounded(paddle, 0.4f, 4, neonCyan);
            DrawCircleV(ballPos, ballRadius, neonYellow);

            EndDrawing();

    }
    CloseWindow();
    return 0;
}