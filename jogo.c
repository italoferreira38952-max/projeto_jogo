#include "raylib.h"
int main(void){

    const int largu = 800;
    const int altu = 450;

    InitWindow(largu , altu , "teste-jogo");
Rectangle botao={400 ,225 ,200 ,200 };
int cont = 0;

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        if(CheckCollisionPointRec(GetMousePosition(),botao) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            cont++;
        }
        if(IsKeyPressed(KEY_F11)){
            ToggleFullscreen();
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawRectangleRec(botao, RED);
        DrawText("CLIQUE", 310, 300, 20, BLUE);
        DrawText(TextFormat("%d", cont), 50, 15, 40, DARKBLUE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
