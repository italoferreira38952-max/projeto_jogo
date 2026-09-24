#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { UPG_PODER = 0, UPG_BONUS, NUM_UPGRADES } TipoUpgrade;

typedef struct {
    char nome[30];
    int nivel;
    int custo;
    int efeito;
} Upgrade;

typedef struct {
    long totalCliques;
    long cliquesManuais;
} Estatisticas;

typedef struct {
    char nome[30];
    long moedas;
    Estatisticas stats;
    Upgrade *upgrades;
} Jogador;

void comprarUpgrade(Jogador *j, int i);
int valorClique(Jogador *j);
void desenharUpgrades(Jogador *j, int mouseX, int mouseY, int cliqueMouse);

int main(void) {
    Jogador jogador;

    printf("Digite seu nome: ");
    scanf("%29s", jogador.nome);

    char nomeOriginal[30];
    strcpy(nomeOriginal, jogador.nome);

    char saudacao[60] = "Bem-vindo(a), ";
    strcat(saudacao, jogador.nome);
    printf("%s! Seu nome tem %d letras.\n", saudacao, (int)strlen(jogador.nome));
    printf("Copia de seguranca do nome original: %s\n", nomeOriginal);

    if (strcmp(jogador.nome, "admin") == 0) {
        jogador.moedas = 1000;
    } else {
        jogador.moedas = 0;
    }

    jogador.stats.totalCliques = 0;
    jogador.stats.cliquesManuais = 0;

    jogador.upgrades = malloc(sizeof(Upgrade) * NUM_UPGRADES);
    jogador.upgrades[UPG_PODER] = (Upgrade){"Poder de Clique", 0, 10, 1};
    jogador.upgrades[UPG_BONUS] = (Upgrade){"Bonus Fixo", 0, 20, 5};

    int conquistas[2][2] = {{0, 0}, {0, 0}};

    const int largu = 800;
    const int altu = 450;
    InitWindow(largu, altu, "teste-jogo");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        int x = GetRenderWidth();
        int y = GetRenderHeight();
        Vector2 cbotao = {(float)x / 2.0f, (float)y / 2.0f};
        float raio = 100.0f;

        int cliqueMouse = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        int acertouBotao = cliqueMouse && CheckCollisionPointCircle(GetMousePosition(), cbotao, raio);

        if (acertouBotao || IsKeyPressed(KEY_SPACE)) {
            jogador.moedas += valorClique(&jogador);
            jogador.stats.totalCliques++;
            jogador.stats.cliquesManuais++;
        }

        if (jogador.stats.totalCliques >= 25)  conquistas[0][0] = 1;
        if (jogador.stats.totalCliques >= 50) conquistas[0][1] = 1;
        if (jogador.stats.totalCliques >= 200) conquistas[1][0] = 1;
        if (jogador.stats.totalCliques >= 500) conquistas[1][1] = 1;

        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
        if (IsKeyPressed(KEY_ESCAPE)) break;

        BeginDrawing();
        ClearBackground(WHITE);

        DrawCircleV(cbotao, raio, RED);
        DrawText("CLIQUE", (int)cbotao.x - 35, (int)cbotao.y - 10, 20, WHITE);

        DrawText(TextFormat("%s - Moedas: %ld", jogador.nome, jogador.moedas), 20, 20, 20, DARKBLUE);
        DrawText(TextFormat("Cliques totais: %ld", jogador.stats.totalCliques), 20, 50, 20, GRAY);
        DrawText(TextFormat("Cliques manuais: %ld", jogador.stats.cliquesManuais), 20, 75, 20, GRAY);

        desenharUpgrades(&jogador, GetMouseX(), GetMouseY(), cliqueMouse);

        DrawText("Conquistas:", 600, 0, 16, DARKGRAY);
        for (int l = 0; l < 2; l++)
            for (int c = 0; c < 2; c++)
                DrawRectangle(600 + c * 30, 20 + l * 30, 25, 25, conquistas[l][c] ? GOLD : LIGHTGRAY);

        DrawText("ESPAÇO faz ganhar cliques | ESC sai", 20, y - 25, 26, GRAY);

        EndDrawing();
    }

    free(jogador.upgrades);
    CloseWindow();
    return 0;
}

void comprarUpgrade(Jogador *j, int i) {
    Upgrade *u = &j->upgrades[i];
    if (j->moedas >= u->custo) {
        j->moedas -= u->custo;
        u->nivel++;
        u->custo += 5;
    }
}

int valorClique(Jogador *j) {
    int base = 1 + j->upgrades[UPG_PODER].nivel * j->upgrades[UPG_PODER].efeito;
    int bonus = j->upgrades[UPG_BONUS].nivel * j->upgrades[UPG_BONUS].efeito;
    return base + bonus;
}

void desenharUpgrades(Jogador *j, int mouseX, int mouseY, int cliqueMouse) {
    for (int i = 0; i < NUM_UPGRADES; i++) {
        Upgrade *u = &j->upgrades[i];
        Rectangle caixa = {600, (float)(100 + i * 60), 180, 50};
        Color cor = LIGHTGRAY;

        if (CheckCollisionPointRec((Vector2){(float)mouseX, (float)mouseY}, caixa)) {
            cor = SKYBLUE;
            if (cliqueMouse) comprarUpgrade(j, i);
        }

        DrawRectangleRec(caixa, cor);
        DrawText(TextFormat("%s Nv%d", u->nome, u->nivel), (int)caixa.x + 5, (int)caixa.y + 5, 14, BLACK);
        DrawText(TextFormat("Custo: %d", u->custo), (int)caixa.x + 5, (int)caixa.y + 28, 14, DARKGRAY);
    }
}