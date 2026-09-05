#include "raylib.h"
int main(void){

    const int largu = 800;
    const int altu = 450;

    InitWindow(largu , altu , "teste-jogo");

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        DrawText("dark sousls", 200, 200, 50, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
