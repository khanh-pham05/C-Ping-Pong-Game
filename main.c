#include "raylib.h"
#include <stdlib.h> // able to use bool
#include <time.h> // able to use timer

// Parent class for positions
typedef struct {
    Vector2 position; //vector2 has x and y float values like coordinates to determine the position
} Coordinates;

// child classes that all inherit from Coordinates 
//typedef used so when calling, it doesn't need to have 'struct' every time calling the class
typedef struct {
    Coordinates base; //attributes encapsulated inside
    Vector2 speed;
    int radius;
} Ball; //name of class

typedef struct {
    Coordinates base;
    int width;
    int height;
    int speed; //int because paddles only goes up and down
    bool poweredUp;
    int powerTimer;
} Paddle;

typedef struct {
    Coordinates base;
    bool active;
    float radius;
} PowerUp;

//all subroutines above main to avoid needing to declare it
//Subroutine to reset the game - can call it anytime when i need to reset
void ResetGame(Ball *ball, Paddle *left, Paddle *right, PowerUp *powerUp, int screenWidth, int screenHeight) {
    ball->base.position = (Vector2){ screenWidth / 2, screenHeight / 2 };
    ball->speed = (Vector2){ 5, 5 };

    //2 objects made for each paddle that has different attribute values
    *left = (Paddle){ { { 50, screenHeight / 2 - 60 } }, 20, 120, 6, false, 0 }; //position, width, height, speed(per frame), power
    *right = (Paddle){ { { screenWidth - 70, screenHeight / 2 - 60 } }, 20, 120, 6, false, 0 };

    float margin = 20;
    float minY = margin;
    float maxY = screenHeight - margin - left->height;

    *powerUp = (PowerUp){
        { { rand() % (screenWidth - 100) + 50, rand() % ((int)(maxY - minY)) + minY } },
        true, 12
    };
}

//power up - paddle gets longer
void UpdatePowerUp(PowerUp *powerUp, Paddle *left, Paddle *right, Ball *ball) {
    if (!powerUp->active) return;

    //paddles turned into 2d rectangles so it can detect the collision of the ball
    Rectangle leftRect = { left->base.position.x, left->base.position.y, left->width, left->height };
    Rectangle rightRect = { right->base.position.x, right->base.position.y, right->width, right->height };

    if (CheckCollisionCircleRec(powerUp->base.position, powerUp->radius, leftRect)) {
        left->poweredUp = true; //uses bool values to see if paddle has power up
        left->powerTimer = 600; //my fps is 60 so this power up will last 10s
        left->height = 180; //increase length of paddle
        powerUp->active = false;
    } else if (CheckCollisionCircleRec(powerUp->base.position, powerUp->radius, rightRect)) {
        right->poweredUp = true;
        right->powerTimer = 600;
        right->height = 180;
        powerUp->active = false;
    }
}

// main game subroutine
void Game(Ball *ball, Paddle *left, Paddle *right, int *p1Score, int *p2Score,
                int screenWidth, int screenHeight, int mode, int *gameStart, PowerUp *powerUp) {
    
    //ball logic
    ball->base.position.x += ball->speed.x; //It adds the speed of the ball to the current position to get the next position
    ball->base.position.y += ball->speed.y;

    // Ball collision with the top and bottom of the window
    if (ball->base.position.y <= 0 || ball->base.position.y >= screenHeight){
        ball->speed.y *= -1; 
    }//The maths reverses the speed of the ball so it goes backwards (bounces) but only on the y axis as that points to the top and bottom 

    //paddles turned into 2d rectangles so it can detect the collision of the ball
    Rectangle leftRect = { left->base.position.x, left->base.position.y, left->width, left->height };
    Rectangle rightRect = { right->base.position.x, right->base.position.y, right->width, right->height };

    // Ball collision with paddles
    if (CheckCollisionCircleRec(ball->base.position, ball->radius, leftRect)) {
        ball->speed.x *= -1;
        ball->base.position.x = leftRect.x + leftRect.width + ball->radius;
    }
    if (CheckCollisionCircleRec(ball->base.position, ball->radius, rightRect)) {
        ball->speed.x *= -1;
        ball->base.position.x = rightRect.x - ball->radius;
    }

    //scoring logic when the ball goes off the screen
    if (ball->base.position.x < 0) {
        (*p2Score)++;
        ResetGame(ball, left, right, powerUp, screenWidth, screenHeight);
    } else if (ball->base.position.x > screenWidth) {
        (*p1Score)++;
        ResetGame(ball, left, right, powerUp, screenWidth, screenHeight);
    }

    //keys for player one on multiplayer or the user in single player
    if (IsKeyDown(KEY_W) && left->base.position.y > 0)
        left->base.position.y -= left->speed;
    if (IsKeyDown(KEY_S) && left->base.position.y < screenHeight - left->height)
        left->base.position.y += left->speed;

    //keys for player 2 in multiplayer
    if (mode == 0) {
        if (IsKeyDown(KEY_UP) && right->base.position.y > 0)
            right->base.position.y -= right->speed;
        if (IsKeyDown(KEY_DOWN) && right->base.position.y < screenHeight - right->height)
            right->base.position.y += right->speed;
    } 
    
    //Computer as player 2 on single player
    //uses the centre of the paddle and follows the ball so if ball is above the center of paddle, it moves up
    else {
        if (ball->base.position.y < right->base.position.y + right->height / 2)
            right->base.position.y -= right->speed;
        else
            right->base.position.y += right->speed;

        if (right->base.position.y < 0) right->base.position.y = 0;
        if (right->base.position.y > screenHeight - right->height)
            right->base.position.y = screenHeight - right->height;
    }

    //when the power up runs out
    UpdatePowerUp(powerUp, left, right, ball);

    if (left->poweredUp && --left->powerTimer <= 0) {
        left->height = 120;
        left->poweredUp = false;
    }
    if (right->poweredUp && --right->powerTimer <= 0) {
        right->height = 120;
        right->poweredUp = false;
    }

    if (IsKeyPressed(KEY_R)) *gameStart = 3;
    if (IsKeyPressed(KEY_P)) *gameStart = 2;
}

//main subroutine
int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Ping Pong");
    SetTargetFPS(60); //frame rate per second of the computer rendering. Usually the typical num when googling
    srand(time(NULL));

    //objects are made from the classes outside the main subroutine
    Ball ball = { { { screenWidth / 2, screenHeight / 2 } }, { 5, 5 }, 10 }; //position, speed x and y, radius
    Paddle left, right;
    PowerUp powerUp;
    ResetGame(&ball, &left, &right, &powerUp, screenWidth, screenHeight);

    int player1score = 0, player2score = 0;
    int gameStart = 0; // 0 = Menu screen, 1 = Game, 2 = Pause screen, 3 = Reset screen
    int mode = 0;      // 0 = Multiplayer, 1 = Single-Player

    while (!WindowShouldClose()) {
        //starting menu with 2 options
        if (gameStart == 0) {
            if (IsKeyPressed(KEY_ONE)) { mode = 1; gameStart = 1; }
            if (IsKeyPressed(KEY_TWO)) { mode = 0; gameStart = 1; }

        } else if (gameStart == 1) {//starts the game by calling the game subroutine
            Game(&ball, &left, &right, &player1score, &player2score,
                       screenWidth, screenHeight, mode, &gameStart, &powerUp);

        } else if (gameStart == 2) { //reset button doesn't pause it exactly to where the game stops
            if (IsKeyPressed(KEY_P)) gameStart = 1; // pause button

        } else if (gameStart == 3) { //reset screen logic
            if (IsKeyPressed(KEY_Y)) {
                player1score = 0;
                player2score = 0;
                ResetGame(&ball, &left, &right, &powerUp, screenWidth, screenHeight);
                gameStart = 0; // back to starting menu
            }
            if (IsKeyPressed(KEY_N)) gameStart = 1; //resumes game
        }

        // render
        BeginDrawing();
        ClearBackground(BLACK);

        if (gameStart == 0) {
            DrawText("PING PONG", screenWidth / 2 - MeasureText("PING PONG", 40) / 2, 150, 40, WHITE);
            DrawText("Press 1 for Single Player", screenWidth / 2 - 120, 250, 20, WHITE);
            DrawText("Press 2 for Multiplayer", screenWidth / 2 - 120, 280, 20, WHITE);
        } else if (gameStart == 1 || gameStart == 2) {
            DrawCircleV(ball.base.position, ball.radius, WHITE);
            DrawRectangleV(left.base.position, (Vector2){ left.width, left.height }, WHITE);
            DrawRectangleV(right.base.position, (Vector2){ right.width, right.height }, WHITE);
            DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);
            DrawText(TextFormat("Player 1: %d", player1score), 20, 20, 20, WHITE);
            DrawText(TextFormat("Player 2: %d", player2score), screenWidth - 150, 20, 20, WHITE);

            if (powerUp.active)
                DrawCircleV(powerUp.base.position, powerUp.radius, GOLD);

            if (gameStart == 2)
                DrawText("Game Paused - Press P to Resume", screenWidth / 2 - 140, screenHeight / 2, 20, RED);
        } else if (gameStart == 3) {
            DrawText("Reset the game?", screenWidth / 2 - 140, screenHeight / 2 - 100, 40, WHITE);
            DrawText("Press Y to confirm, N to cancel", screenWidth / 2 - 140, screenHeight / 2, 20, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
