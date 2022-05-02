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
#include <stdio.h>

#define PHYSAC_IMPLEMENTATION
#include "raylibextras/physac.h"
#include "raymath.h"

#undef PHYSAC_IMPLEMENTATION
#include "vendor/flecs/flecs.h"

#define MAIN
#include "ecs.h"

#define MAX_BUILDINGS   10000
#define FORCE          4.0f
#define MAX_FORCE     10.0f
#define MAX_VEL     0.1f
#define TURN          0.1f

#define SHIP_WIDTH 20
#define SHIP_LENGTH 40

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera");

    InitPhysics();



    Rectangle player = { 400, 280, 40, 40 };
    Rectangle buildings[MAX_BUILDINGS] = { 0 };
    Color buildColors[MAX_BUILDINGS] = { 0 };

    PhysicsBody playerBody = CreatePhysicsBodyRectangle((Vector2){screenWidth / 2.0f, screenHeight / 2.0f}, 54, 40, 1.0f);
    int spacing = 0;
    playerBody->useGravity = false;

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
    camera.zoom = 1.0f;

    init_ecs(&camera);

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        UpdatePhysics();

        // Player movement
        if (IsKeyDown(KEY_RIGHT)) player.x += 2;
        else if (IsKeyDown(KEY_LEFT)) player.x -= 2;

        // Get physics body position
        Vector2 p = playerBody->position;
        player.x = p.x;
        player.y = p.y;
        float orientation = playerBody->orient;
        float lineStartX = player.x + 40;
        float lineStartY = player.y + 40;
        float lineEndX = (float)cos(orientation) * 50.0f;
        float lineEndY = (float)sin(orientation) * 50.0f;

        char buf[255];
        snprintf(buf, sizeof(buf), "SPD %.2f ROT %.2f", Vector2Length(playerBody->velocity), orientation);


        if (IsKeyDown(KEY_W))
        {
          float orientation = playerBody->orient;
          Vector2 force = { cos(orientation) * FORCE, sin(orientation) * FORCE };
          PhysicsAddForce(playerBody, force);

        }
        else if (IsKeyDown(KEY_S))
        {
          float orientation = playerBody->orient;
          Vector2 force = { cos(orientation) * -FORCE, sin(orientation) * -FORCE };
          PhysicsAddForce(playerBody, force);
        }

        if (Vector2Length(playerBody->velocity) > MAX_VEL)
        {
          Vector2 maxVel = Vector2Scale(Vector2Normalize(playerBody->velocity),MAX_VEL);
          playerBody->velocity = maxVel;
        }
        if (Vector2Length(playerBody->force) > MAX_FORCE)
        {
          printf("SCALE");
          Vector2 force = Vector2Normalize(playerBody->force);
          force = Vector2Scale(force, MAX_FORCE);
          PhysicsAddForce(playerBody, force);
        }

        if (IsKeyDown(KEY_A))
        {
          playerBody->orient -= TURN;
          //PhysicsAddTorque(playerBody, -5000.0f);
        }
        else if (IsKeyDown(KEY_D))
        {
          playerBody->orient += TURN;
          //PhysicsAddTorque(playerBody, 5000.0f);
        }
        
        //if (IsKeyDown(KEY_S)) player.y -= 2;
        // Camera target follows player
        //camera.target = (Vector2){ player.x + 20, player.y + 20 };

        // Camera rotation controls
        //if (IsKeyDown(KEY_A)) camera.rotation--;
        //else if (IsKeyDown(KEY_S)) camera.rotation++;


        //camera.rotation = RAD2DEG*playerBody->orient;

        // Limit camera rotation to 80 degrees (-40 to 40)
        //if (camera.rotation > 40) camera.rotation = 40;
        //else if (camera.rotation < -40) camera.rotation = -40;

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
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode2D(camera);

                DrawRectangle(-6000, 320, 13000, 8000, DARKGRAY);

                for (int i = 0; i < MAX_BUILDINGS; i++) DrawRectangleRec(buildings[i], buildColors[i]);



                //DrawRectangleRec(player, RED);

                DrawLine((int)camera.target.x, -screenHeight*10, (int)camera.target.x, screenHeight*10, GREEN);
                DrawLine(-screenWidth*10, (int)camera.target.y, screenWidth*10, (int)camera.target.y, GREEN);
                run_ecs();

            EndMode2D();


            DrawText("SCREEN AREA", 640, 10, 20, RED);
            DrawText(buf, 640, 40, 20, RED);
            DrawLine((int)lineStartX, (int)lineStartY, (int)(lineStartY + lineEndX), (int)(lineStartY + lineEndY), GREEN);

            DrawRectangle(0, 0, screenWidth, 5, RED);
            DrawRectangle(0, 5, 5, screenHeight - 10, RED);
            DrawRectangle(screenWidth - 5, 5, 5, screenHeight - 10, RED);
            DrawRectangle(0, screenHeight - 5, screenWidth, 5, RED);

            DrawRectangle( 10, 10, 250, 113, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines( 10, 10, 250, 113, BLUE);

            DrawText("Free 2d camera controls:", 20, 20, 10, BLACK);
            DrawText("- Right/Left to move Offset", 40, 40, 10, DARKGRAY);
            DrawText("- Mouse Wheel to Zoom in-out", 40, 60, 10, DARKGRAY);
            DrawText("- A / S to Rotate", 40, 80, 10, DARKGRAY);
            DrawText("- R to reset Zoom and Rotation", 40, 100, 10, DARKGRAY);

            BeginMode2D(camera);
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
            EndMode2D();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
