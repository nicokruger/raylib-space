/*******************************************************************************************
*
*   raylib [core] example - 2d camera
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <iostream>
#include <stdio.h>

#define PHYSAC_IMPLEMENTATION
#include "raylibextras/physac.h"
#include "raymath.h"

#undef PHYSAC_IMPLEMENTATION
#include "vendor/flecs/flecs.h"

#define MAIN
#include "ecs.h"
#include "button.h"

#define MAX_BUILDINGS   1
#define FORCE          4.0f
#define MAX_FORCE     10.0f
#define MAX_VEL     0.1f
#define TURN          0.1f

#define SHIP_WIDTH 20
#define SHIP_LENGTH 40

#define PLATFORM_DESKTOP 1
#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera");

    InitPhysics();

    // Load texture texture to apply shaders
    Texture2D texture = LoadTexture("resources/space.png");

    // Load shader and setup location points and values
    Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/wave.fs", GLSL_VERSION));
    int secondsLoc = GetShaderLocation(shader, "secondes");
    int freqXLoc = GetShaderLocation(shader, "freqX");
    int freqYLoc = GetShaderLocation(shader, "freqY");
    int ampXLoc = GetShaderLocation(shader, "ampX");
    int ampYLoc = GetShaderLocation(shader, "ampY");
    int speedXLoc = GetShaderLocation(shader, "speedX");
    int speedYLoc = GetShaderLocation(shader, "speedY");

    // Shader uniform values that can be updated at any time
    float freqX = 25.0f;
    float freqY = 25.0f;
    float ampX = 5.0f;
    float ampY = 5.0f;
    float speedX = 8.0f;
    float speedY = 8.0f;

    float screenSize[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
    SetShaderValue(shader, GetShaderLocation(shader, "size"), &screenSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, freqXLoc, &freqX, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, freqYLoc, &freqY, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, ampXLoc, &ampX, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, ampYLoc, &ampY, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, speedXLoc, &speedX, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, speedYLoc, &speedY, SHADER_UNIFORM_FLOAT);


    Rectangle player = { 400, 280, 40, 40 };
    Rectangle buildings[MAX_BUILDINGS] = { 0 };
    Color buildColors[MAX_BUILDINGS] = { 0 };

    int spacing = 0;

    for (int i = 0; i < MAX_BUILDINGS; i++)
    {
        buildings[i].width = (float)GetRandomValue(50, 200);
        buildings[i].height = (float)GetRandomValue(100, 800);
        buildings[i].y = screenHeight - 130.0f - buildings[i].height;
        buildings[i].x = -6000.0f + spacing;

        spacing += (int)buildings[i].width;

        buildColors[i] = (Color){ GetRandomValue(200, 240), GetRandomValue(200, 240), GetRandomValue(200, 250), 255 };
    }

    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 0.8f;

    HudInfo *hudInfo = init_ecs(&camera);

    float seconds = 0.0f;
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // Camera zoom controls
        camera.zoom += ((float)GetMouseWheelMove()*0.05f);

        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.1f) camera.zoom = 0.1f;

        // Camera reset (zoom and rotation)
        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 1.0f;
            camera.rotation = 0.0f;
        }

        if (hudInfo->state == GUI_STATE_PLAYING) {
          UpdatePhysics();
          //----------------------------------------------------------------------------------

          // Draw
          //----------------------------------------------------------------------------------
          BeginDrawing();
              BeginShaderMode(shader);
                //DrawTexture(texture, 0, 0, WHITE);
                //DrawTexture(texture, texture.width, 0, WHITE);
            float scrollingBack = -camera.target.x / 6.0f;
            DrawTextureEx(texture, (Vector2){ scrollingBack, 20 }, 0.0f, 2.0f, WHITE);
            DrawTextureEx(texture, (Vector2){ texture.width*2 + scrollingBack, 20 }, 0.0f, 2.0f, WHITE);
              EndShaderMode();


              ClearBackground(RAYWHITE);
              seconds += GetFrameTime();

              SetShaderValue(shader, secondsLoc, &seconds, SHADER_UNIFORM_FLOAT);

              BeginMode2D(camera);

                  //DrawRectangle(-6000, 320, 13000, 8000, DARKGRAY);

                  for (int i = 0; i < MAX_BUILDINGS; i++) DrawRectangleRec(buildings[i], buildColors[i]);



                  //DrawRectangleRec(player, RED);

                  //DrawLine((int)camera.target.x, -screenHeight*10, (int)camera.target.x, screenHeight*10, GREEN);
                  //DrawLine(-screenWidth*10, (int)camera.target.y, screenWidth*10, (int)camera.target.y, GREEN);
                  run_ecs();

              EndMode2D();


              DrawText("SCREEN AREA", 640, 10, 20, RED);

              DrawRectangle(0, 0, screenWidth, 5, RED);
              DrawRectangle(0, 5, 5, screenHeight - 10, RED);
              DrawRectangle(screenWidth - 5, 5, 5, screenHeight - 10, RED);
              DrawRectangle(0, screenHeight - 5, screenWidth, 5, RED);

              DrawRectangle( 10, 10, 250, 113, Fade(SKYBLUE, 0.5f));
              DrawRectangleLines( 10, 10, 250, 113, BLUE);

              char buf2[255];
              snprintf(buf2, sizeof(buf2), "Health %.2f/%.2f\nProgress: %.2f/%.2f\nLevel: %i",
                  hudInfo->health,
                  hudInfo->maxHealth,
                  hudInfo->score,
                  hudInfo->maxScore,
                  hudInfo->level
                  );


              DrawText(buf2, 20, 20, 30, BLACK);
              //DrawText("- Right/Left to move Offset", 40, 40, 10, DARKGRAY);
              //DrawText("- Mouse Wheel to Zoom in-out", 40, 60, 10, DARKGRAY);
              //DrawText("- A / S to Rotate", 40, 80, 10, DARKGRAY);
              //DrawText("- R to reset Zoom and Rotation", 40, 100, 10, DARKGRAY);

              BeginMode2D(camera);
              /*
                  Vector2 v1, v2, v3;
                  float playerAngle = playerBody->orient;
                  v1 = Vector2Rotate((Vector2){
                    40.0f,
                    0.0f
                  }, playerAngle);
                  v2 = Vector2Rotate((Vector2){
                    - 40.0f,
                    - 40.0f,
                    //0,0
                  }, playerAngle);
                  v3 = Vector2Rotate((Vector2){
                    -40.0f,
                    40.0f
                    //500,500
                  }, playerAngle);
                  DrawTriangle(
                      (Vector2){
                      v1.x + player.x,
                      v1.y + player.y},
                      (Vector2){v2.x + player.x,
                      v2.y + player.y},
                      (Vector2){v3.x + player.x,
                      v3.y + player.y},
                      RED);
                  DrawTriangle(v1, v2, v3, GREEN);
                  */
              EndMode2D();

          EndDrawing();
          //----------------------------------------------------------------------------------
        } else if (hudInfo->state == GUI_STATE_GAMEOVER) {
          BeginDrawing();
          ClearBackground(RAYWHITE);
          DrawText("GAME OVER", screenWidth/2 - 100, screenHeight/2 - 50, 50, RED);
          EndDrawing();
          
          if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsGestureDetected(GESTURE_TAP)) {
            hudInfo->state = GUI_STATE_PLAYING;
            hudInfo->health = hudInfo->maxHealth;
            InitPhysics();
            reset_ecs();
            setup_scene();
          }

        } else if (hudInfo->state == GUI_STATE_UPGRADE) {
          int numButtons = 4;
          Button buttons[] = {
            {screenWidth-200,200,145,45,30, GRAY, LIGHTGRAY,DARKGRAY,hudInfo->upgrades[0].name.c_str(),1,true},
            {screenWidth-200,300,145,45,30, GRAY, LIGHTGRAY,DARKGRAY,hudInfo->upgrades[1].name.c_str(),1,true},

            {screenWidth-400,200,145,45,30, GRAY, LIGHTGRAY,DARKGRAY,hudInfo->upgrades[2].name.c_str(),1,true},
            {screenWidth-400,300,145,45,30, GRAY, LIGHTGRAY,DARKGRAY,hudInfo->upgrades[3].name.c_str(),1,true},
          };
          BeginDrawing();
          ClearBackground(RAYWHITE);
          // TODO: Draw TITLE screen here!
          DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
          DrawText("UPGRADE!!", 20, 20, 40, DARKGREEN);
          DrawText("CHOOSE YOUR UPGRADE", 120, 220, 20, DARKGREEN);

          for (int i = 0; i < numButtons; i++) {
            buttons[i].draw();
            if (buttons[i].onButtonPress()) {
              std::cout << "clicked it" << std::endl;
              hudInfo->toUpgrade = hudInfo->upgrades[i].execute;
              hudInfo->doUpgrade = true;
              hudInfo->state = GUI_STATE_PLAYING;
            }
          }
          EndDrawing();

        }
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    //
    UnloadShader(shader);         // Unload shader
    UnloadTexture(texture);       // Unload texture
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
