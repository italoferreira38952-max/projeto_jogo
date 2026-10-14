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
void desenharUpgrades(Jogador *j, Font fonte, int mouseX, int mouseY, int cliqueMouse);

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

    // texturas (mantém o pixel art nítido)
    Texture2D texFundo = LoadTexture("assets/fundo.png");
    Texture2D texBotao = LoadTexture("assets/botao.png");
    SetTextureFilter(texFundo, TEXTURE_FILTER_POINT);
    SetTextureFilter(texBotao, TEXTURE_FILTER_POINT);

    // avisos no terminal se algum arquivo não for encontrado
    if (texFundo.id == 0) TraceLog(LOG_WARNING, "NAO ACHOU assets/fundo.png (confira pasta e nome)");
    if (texBotao.id == 0) TraceLog(LOG_WARNING, "NAO ACHOU assets/botao.png (confira pasta e nome)");

    // fonte Matcha Style: ASCII + acentos do português
    int codepoints[256];
    int nCodepoints = 0;
    for (int i = 32; i < 127; i++) codepoints[nCodepoints++] = i;

    const char *extras = "áàâãéêíóôõúçÁÀÂÃÉÊÍÓÔÕÚÇ";
    for (int i = 0; extras[i] != '\0'; ) {
        int bytes;
        codepoints[nCodepoints++] = GetCodepoint(&extras[i], &bytes);
        i += bytes;
    }

    // carrega em tamanho grande (48) e desenha menor; filtro suave (não é fonte pixel)
    Font fonte = LoadFontEx("assets/Matcha_Style.ttf", 48, codepoints, nCodepoints);
    SetTextureFilter(fonte.texture, TEXTURE_FILTER_BILINEAR);
    if (fonte.texture.id == GetFontDefault().texture.id)
        TraceLog(LOG_WARNING, "NAO ACHOU assets/Matcha_Style.ttf (usando fonte padrao)");

    // o PNG do botão tem 4 frames lado a lado
    const int NUM_FRAMES = 4;
    float larguraFrame = (float)texBotao.width / NUM_FRAMES;

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
        if (jogador.stats.totalCliques >= 50)  conquistas[0][1] = 1;
        if (jogador.stats.totalCliques >= 200) conquistas[1][0] = 1;
        if (jogador.stats.totalCliques >= 500) conquistas[1][1] = 1;

        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
        if (IsKeyPressed(KEY_ESCAPE)) break;

        BeginDrawing();
        ClearBackground(WHITE);

        // fundo
        DrawTexturePro(texFundo,
            (Rectangle){0, 0, (float)texFundo.width, (float)texFundo.height},
            (Rectangle){0, 0, (float)x, (float)y},
            (Vector2){0, 0}, 0.0f, WHITE);

        // botão: frame 2 (apertado) enquanto segura, frame 0 (solto) no resto
        int segurando = (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
                         CheckCollisionPointCircle(GetMousePosition(), cbotao, raio))
                        || IsKeyDown(KEY_SPACE);
        int frame = segurando ? 2 : 0;

        float tam = raio * 2.1f;  // o desenho ocupa quase toda a célula
        DrawTexturePro(texBotao,
            (Rectangle){frame * larguraFrame, 0, larguraFrame, (float)texBotao.height},
            (Rectangle){cbotao.x, cbotao.y, tam, tam},
            (Vector2){tam / 2, tam / 2}, 0.0f, WHITE);

        // textos com a fonte Matcha Style
        DrawTextEx(fonte, TextFormat("%s - Moedas: %ld", jogador.nome, jogador.moedas),
                   (Vector2){20, 20}, 30, 1, GOLD);
        DrawTextEx(fonte, TextFormat("Cliques totais: %ld", jogador.stats.totalCliques),
                   (Vector2){20, 58}, 22, 1, RAYWHITE);
        DrawTextEx(fonte, TextFormat("Cliques manuais: %ld", jogador.stats.cliquesManuais),
                   (Vector2){20, 84}, 22, 1, RAYWHITE);

        desenharUpgrades(&jogador, fonte, GetMouseX(), GetMouseY(), cliqueMouse);

        DrawTextEx(fonte, "Conquistas:", (Vector2){600, 0}, 22, 1, RAYWHITE);
        for (int l = 0; l < 2; l++)
            for (int c = 0; c < 2; c++)
                DrawRectangle(600 + c * 30, 26 + l * 30, 25, 25, conquistas[l][c] ? GOLD : LIGHTGRAY);

        DrawTextEx(fonte, "ESPAÇO faz ganhar cliques | ESC sai",
                   (Vector2){20, (float)(y - 34)}, 22, 1, RAYWHITE);

        EndDrawing();
    }

    free(jogador.upgrades);
    UnloadTexture(texFundo);
    UnloadTexture(texBotao);
    UnloadFont(fonte);
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

void desenharUpgrades(Jogador *j, Font fonte, int mouseX, int mouseY, int cliqueMouse) {
    for (int i = 0; i < NUM_UPGRADES; i++) {
        Upgrade *u = &j->upgrades[i];
        // caixa mais larga (240) para caber "Poder de Clique Nv10" na Matcha Style
        Rectangle caixa = {550, (float)(110 + i * 64), 240, 56};
        Color cor = LIGHTGRAY;

        if (CheckCollisionPointRec((Vector2){(float)mouseX, (float)mouseY}, caixa)) {
            cor = SKYBLUE;
            if (cliqueMouse) comprarUpgrade(j, i);
        }

        DrawRectangleRec(caixa, cor);
        DrawTextEx(fonte, TextFormat("%s Nv%d", u->nome, u->nivel),
                   (Vector2){caixa.x + 6, caixa.y + 6}, 18, 1, BLACK);
        DrawTextEx(fonte, TextFormat("Custo: %d", u->custo),
                   (Vector2){caixa.x + 6, caixa.y + 30}, 18, 1, DARKGRAY);
    }
}