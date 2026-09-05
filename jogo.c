#include "raylib.h"
int main(void){

    const int largu = 800;
    const int altu = 450;

    InitWindow(largu , altu , "teste-jogo");
Vector2 cbotao={400, 225};
float raio=100.0f;
int cont = 0;

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        if(CheckCollisionPointCircle(GetMousePosition(),cbotao,raio) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            cont++;
        }if(IsKeyPressed(KEY_SPACE)){
            cont++;
        }
        if(IsKeyPressed(KEY_F11)){
            ToggleFullscreen();
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawCircleV(cbotao, raio, RED);
        DrawText("CLIQUE", 365, 215, 20, WHITE);
        DrawText(TextFormat("%d", cont), 50, 15, 40, DARKBLUE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
