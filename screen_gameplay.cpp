/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Gameplay Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2022 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"
#include "screens.h"

const int MAX_ENVIRONMENT_ELEMENTS = 3;
const int gravity = 800.0f;
const int playerJumpSpeed = 650.0f;
const int playerRunSpeed = 500.0f;
const int platformWidth = 100;
const int screenWidth = 800;
const int screenHeight = 600;

// Load slime png
static Texture2D slime;
// Add this flag to keep track of whether the congratulations message should be displayed
static bool showCongratulations = false;

struct Player {
    Vector2 position;
    float speed;
    bool canJump;
};

struct EnvElement {
    Rectangle rect;
    int blocking;
    Color color;
    bool movingPlatform;
    float moveSpeed;
};

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
static int framesCounter = 0;
static int finishScreen = 0;

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Game variables
Player player = {0};
EnvElement envElements[MAX_ENVIRONMENT_ELEMENTS];
static Camera2D camera = { 0 };

// Gameplay Screen Initialization logic
void InitGameplayScreen(void) {
//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

    // Load the player sprite
    slime = LoadTexture("resources/slime.png");

    // Initialize player
    player.position = (Vector2){400, 280};
    player.speed = 0;
    player.canJump = false;

    // Generate platforms!
    for (int i = 0; i < MAX_ENVIRONMENT_ELEMENTS; i++) {
        envElements[i].rect.width = platformWidth; // Platform width
        envElements[i].rect.height = 25; // Platform height
        envElements[i].rect.x = GetRandomValue(0, GetScreenWidth() - envElements[i].rect.width); // Random horizontal placement of platform
        envElements[i].rect.y = GetScreenHeight() - i * 200; // Adjust height between each platform
        envElements[i].blocking = 1;
        envElements[i].color = RAYWHITE; // Adjust color
        envElements[i].movingPlatform = (i % 3 == 2); // Every third platform is a moving platform
        envElements[i].moveSpeed = 50.0f; // Adjust moving platform speed
        if(i == 0){
            envElements[i].rect.x = 0;
            envElements[i].rect.width = screenWidth; // Floor
        }
        if(i >= 15) {
            envElements[i].moveSpeed = 100.0f; // Adjust speed here
        }
        if(i >= 30) {
            envElements[i].moveSpeed = 150.0f; // Adjust speed here
        }
        if(i >= 45) {
            envElements[i].moveSpeed = 200.0f; // Adjust speed here
        }
        if(i >= 60) {
            envElements[i].moveSpeed = 300.0f; // Adjust speed here
        }
        if(i == MAX_ENVIRONMENT_ELEMENTS - 1) {
            envElements[i].color = GREEN; // Winning platform!
            envElements[i].rect.x = 0;
            envElements[i].rect.width = screenWidth;
        }
    }

    // Initialize the camera
    camera.target = (Vector2){ player.position.x, player.position.y };
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void) {
    if (!showCongratulations) {
        // Move left
        if ((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) && player.position.x - playerRunSpeed * GetFrameTime() > 0)
            player.position.x -= playerRunSpeed * GetFrameTime();

        // Move right
        if ((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) && player.position.x + playerRunSpeed * GetFrameTime() < GetScreenWidth())
            player.position.x += playerRunSpeed * GetFrameTime();

        // Check if the player is standing on a platform
        int standingOnPlatform = 0;
        for (int i = 0; i < MAX_ENVIRONMENT_ELEMENTS; i++) {
            EnvElement* element = &envElements[i];
            Vector2* p = &(player.position);
            if (element->blocking && element->rect.x <= p->x && element->rect.x + element->rect.width >= p->x &&
                element->rect.y == p->y + player.speed * GetFrameTime()) {
                standingOnPlatform = 1;
                break;
            }
        }

        // If standing on a platform, make the player continuously jump
        if (standingOnPlatform) {
            player.speed = -playerJumpSpeed;
            player.canJump = false;
        }

        // Iterate through every platform and check for collision
        int hitObstacle = 0;
        for (int i = 0; i < MAX_ENVIRONMENT_ELEMENTS; i++) {
            EnvElement* element = &envElements[i];
            Vector2* p = &(player.position);
            // Collision detected
            if (element->blocking && element->rect.x <= p->x && element->rect.x + element->rect.width >= p->x &&
                element->rect.y >= p->y && element->rect.y <= p->y + player.speed * GetFrameTime()) {
                hitObstacle = 1;
                player.speed = 0.0f;
                p->y = element->rect.y;
            }
        }

        // If there's no platform under the player, fall
        if (!hitObstacle) {
            player.position.y += player.speed * GetFrameTime();
            player.speed += gravity * GetFrameTime();
            player.canJump = false;
        }
        // If on a platform, jump!
        else {
            player.canJump = true;
            PlaySound(fxCoin); // Play sound
        }

        // Iterate through all the platforms for moving platforms
        for (int i = 0; i < MAX_ENVIRONMENT_ELEMENTS; i++) {
            EnvElement* element = &envElements[i];
            if (element->movingPlatform) {
                // Update movement speed
                element->rect.x += element->moveSpeed * GetFrameTime();
                // If it touches the left of right side of the screen, change directions
                if (element->rect.x <= 0 || element->rect.x + element->rect.width >= GetScreenWidth()) {
                    element->moveSpeed = -element->moveSpeed;
                }
            }
        }
        
        // Set camera target to player position only vertically
        camera.target.y = player.position.y;

        framesCounter++;
    }
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
    ClearBackground(SKYBLUE);

    BeginMode2D(camera);

    // Draw environment elements
    for (int i = 0; i < MAX_ENVIRONMENT_ELEMENTS; i++) {
        DrawRectangleRec(envElements[i].rect, envElements[i].color);
    }

    // Draw the player's sprite
    DrawTexture(slime, player.position.x - slime.width / 2, player.position.y - slime.height / 2, WHITE);

    EndMode2D(); // End the 2D drawing mode

    // Check if the player touched the winning platform
    EnvElement* winningPlatform = &envElements[MAX_ENVIRONMENT_ELEMENTS - 1];
    Vector2* playerPos = &(player.position);

    if (winningPlatform->rect.x <= playerPos->x + slime.width / 2 &&
        winningPlatform->rect.x + winningPlatform->rect.width >= playerPos->x - slime.width / 2 &&
        winningPlatform->rect.y <= playerPos->y + slime.height / 2 &&
        winningPlatform->rect.y + winningPlatform->rect.height >= playerPos->y - slime.height / 2) {
        showCongratulations = true;
        }

        if (showCongratulations) {
            // Player touched the winning platform, display a window with the time taken
            Rectangle windowRect = { (float)(GetScreenWidth() / 4), (float)(GetScreenHeight() / 4), (float)(GetScreenWidth() / 2), (float)(GetScreenHeight() / 2) };
            DrawRectangleRec(windowRect, RAYWHITE);
            DrawRectangleLinesEx(windowRect, 3, BLACK);

            // Calculate and display the time taken
            int timerValue = framesCounter / 60; // 60 FPS
            DrawText(TextFormat("Congratulations!\nTime Taken: %02d:%02d\nClick anywhere to return to the\ntitle screen.", timerValue / 60, timerValue % 60),
                windowRect.x + 20, windowRect.y + 20, 20, BLACK);

            // Check if the left mouse button is pressed to return to the title screen
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                finishScreen = 1; // Player clicked, go to title screen
            }
        }
        
    else {
        // Draw timer at the top right corner
        int timerValue = framesCounter / 60; // 60 FPS
        DrawText(TextFormat("Time: %02d:%02d", timerValue / 60, timerValue % 60),
            GetScreenWidth() - MeasureText("Time: 00:00", 20) - 10, 10, 20, BLACK);
    }
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    UnloadTexture(slime);
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}
