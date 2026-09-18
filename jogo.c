#include "raylib.h"
#include <stdlib.h>  
#include <string.h>   
typedef enum { 
    UPG_PODER = 0,
    UPG_BONUS,
    NUM_UPGRADES } TipoUpgrade;

typedef struct {
    long totalCliques;
    long cliquesManuais;
} Estatisticas;

typedef struct {
    char nome[30];
    int nivel;
    int custo;
    int efeito;
} Upgrade;

typedef struct {
    char nome[30];
    long moedas;
    Estatisticas stats;
    Upgrade *upgrades; 
} Jogador;


int main(void){

    const int largu = 800;
    const int altu = 450;

    InitWindow(largu , altu , "teste-jogo");

float raio=100.0f;
Jogador jogador;

jogador.moedas = 0;
jogador.stats.totalCliques = 0;
jogador.stats.cliquesManuais = 0;

jogador.upgrades = malloc(sizeof(Upgrade) * NUM_UPGRADES);

jogador.upgrades[UPG_PODER] = (Upgrade){"Poder de Clique", 0, 10, 1};
jogador.upgrades[UPG_BONUS] = (Upgrade){"Bonus Fixo", 0, 20, 5};

    SetTargetFPS(60);
    while (!WindowShouldClose()) {

        int x = GetRenderWidth();
        int y = GetRenderHeight();
        Vector2 cbotao = {(float)x /2.0f, (float)y / 2.0f};
        if(CheckCollisionPointCircle(GetMousePosition(),cbotao,raio) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            jogador.moedas++;
            jogador.stats.totalCliques++;
            jogador.stats.cliquesManuais++;
        }if(IsKeyPressed(KEY_SPACE)){
            jogador.moedas++; 
            jogador.stats.totalCliques++;
            jogador.stats.cliquesManuais++;
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
        DrawText(TextFormat("Moedas: %ld", jogador.moedas), 20, 20, 20, DARKBLUE);
        DrawText(TextFormat("Cliques: %ld", jogador.stats.totalCliques), 20, 50, 20, GRAY);
        //adicionei essa informação
         DrawText("ESPAÇO faz ganhar cliques | ESC sai", 20, y - 25, 26, GRAY);
        EndDrawing();
    }
    free(jogador.upgrades);
    CloseWindow();
    return 0;
}
