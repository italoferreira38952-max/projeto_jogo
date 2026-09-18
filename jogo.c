#include "raylib.h"
typedef enum { 
    UPG_PODER = 0,
    UPG_BONUS,
    NUM_UPGRADES } TipoUpgrade;

typedef struct {
    char nome[30];
    long moedas;
} Jogador;

int main(void){

    const int largu = 800;
    const int altu = 450;

    InitWindow(largu , altu , "teste-jogo");

float raio=100.0f;
Jogador jogador;
jogador.moedas = 0;
    SetTargetFPS(60);
    while (!WindowShouldClose()) {

        int x = GetRenderWidth();
        int y = GetRenderHeight();
        Vector2 cbotao = {(float)x /2.0f, (float)y / 2.0f};
        if(CheckCollisionPointCircle(GetMousePosition(),cbotao,raio) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            jogador.moedas++;
        }if(IsKeyPressed(KEY_SPACE)){
            jogador.moedas++; 
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
        //botão para colocar upgrades
        DrawRectangle(600,320,170,50,GRAY);
        DrawCircleV(cbotao, raio, RED);
        DrawText("CLIQUE", cbotao.x - 35, cbotao.y - 10, 20, WHITE);
        DrawText(TextFormat("TOTAL DE CLIQUES: %d", jogador.moedas), 50, 15, 40, DARKBLUE);
        //adicionei essa informação
         DrawText("ESPAÇO faz ganhar cliques | ESC sai", 20, y - 25, 26, GRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
