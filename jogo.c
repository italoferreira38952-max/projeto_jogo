#include "raylib.h"
int main(void){

    const int largu = 800;
    const int altu = 450;

    InitWindow(largu , altu , "teste-jogo");
//Vector2 cbotao={400, 225};,troquei
float raio=100.0f;
int cont = 0;
    SetTargetFPS(60);
    while (!WindowShouldClose()) {

        int x = GetRenderWidth();
        int y = GetRenderHeight();
        Vector2 cbotao = {(float)x /2.0f, (float)y / 2.0f};
        if(CheckCollisionPointCircle(GetMousePosition(),cbotao,raio) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            cont++;
        }if(IsKeyPressed(KEY_SPACE)){
            cont++; 
        }
        if(IsKeyPressed(KEY_F11)){
            ToggleFullscreen();
        }
        // Se apertar ESC, sai do jogo
        if (IsKeyPressed(KEY_ESCAPE))
        {
            break;
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawCircleV(cbotao, raio, RED);
        DrawText("CLIQUE", cbotao.x - 35, cbotao.y - 10, 20, WHITE);
        DrawText(TextFormat("TOTAL DE CLIQUES: %d", cont), 50, 15, 40, DARKBLUE);
        //adicionei essa informação
         DrawText("ESPAÇO faz ganhar cliques | ESC sai", 20, altu - 25, 26, GRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
