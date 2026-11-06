#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------- Paleta (inspirada na interface de referência) ----------
#define COR_FUNDO       (Color){202, 215, 241, 255}  // azul-claro do fundo
#define COR_NAVY        (Color){27, 38, 105, 255}    // textos e bordas
#define COR_CAIXA       (Color){68, 80, 124, 255}    // caixa de upgrade
#define COR_CAIXA_HOV   (Color){88, 101, 150, 255}   // caixa com mouse em cima
#define COR_PRECO_OK    (Color){105, 170, 130, 255}    // botão de preço (dá pra comprar)
#define COR_PRECO_NO    (Color){60, 72, 98, 255}     // botão de preço (sem moedas)
#define COR_TEXTO_CLARO (Color){235, 240, 255, 255}
#define COR_TEXTO_SUAVE (Color){190, 200, 230, 255}
#define COR_CONQ_OFF    (Color){150, 165, 205, 255}

typedef enum { ESTADO_DIGITANDO_NOME = 0, ESTADO_JOGANDO } GameState;

// NOVO: UPG_AUTO (terceiro upgrade). NUM_UPGRADES passa a valer 3.
typedef enum { UPG_PODER = 0, UPG_BONUS, UPG_AUTO, NUM_UPGRADES } TipoUpgrade;

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

#define ARQUIVO_SAVE "save.txt"

// Ranking: cada partida encerrada vira uma linha "cliques;nome" em ranking.txt
#define ARQUIVO_RANKING "ranking.txt"
#define MAX_RANKING 100

typedef struct {
    char nome[30];
    long cliques;
} Registro;

int carregarRanking(Registro v[]);
void registrarRanking(Jogador *j);
void ordenarRanking(Registro v[], int n, int porNome);
int buscaLinear(Registro v[], int n, const char *nome, int *comp);
int buscaBinaria(Registro v[], int n, long cliques, int *comp);

int salvarJogo(Jogador *j, int conq[2][2]);
int carregarJogo(Jogador *j, int conq[2][2]);
void comprarUpgrade(Jogador *j, int i);
int valorClique(Jogador *j);
int cliquesPorSegundo(Jogador *j);
int moedasPorSegundo(Jogador *j);
void desenharCaixa(Rectangle r, Color preenchimento);
void desenharUpgrades(Jogador *j, Font fonte, int mouseX, int mouseY, int cliqueMouse);

int main(void) {
    Jogador jogador;
    memset(&jogador, 0, sizeof(Jogador)); // Limpa a struct

    GameState estadoAtual = ESTADO_DIGITANDO_NOME;
    int letraCount = 0;

    const int largu = 800;
    const int altu = 450;
    InitWindow(largu, altu, "teste-jogo");
    InitAudioDevice();
    SetTargetFPS(60);

    // textura do botão (mantém o pixel art nítido)
    // OBS: o fundo agora é uma cor lisa, então não precisa mais do wall.png
    Texture2D texBotao = LoadTexture("assets/botao.png");
    SetTextureFilter(texBotao, TEXTURE_FILTER_POINT);

    Sound som = LoadSound("assets/clique.wav");
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

    // o PNG do botão tem 4 frames lado a lado: 0 normal, 1 hover, 2 apertado, 3 desativado
    const int NUM_FRAMES = 4;
    float larguraFrame = (float)texBotao.width / NUM_FRAMES;

    int conquistas[2][2] = {{0, 0}, {0, 0}};

    float tempoApertado = 0.0f; // mantém o botão afundado um instante depois do clique
    float acumAuto = 0.0f;      // conta o tempo para o Auto Clique (1 tick por segundo)
    float tempoMsg = 0.0f;      // tempo restante da mensagem de save/load na tela
    const char *textoMsg = "";

    Registro ranking[MAX_RANKING];
    int nRanking = 0;
    int mostrarRanking = 0; // 1 = tela de ranking aberta
    int ordemPorNome = 0;   // 0 = por cliques (maior primeiro), 1 = por nome (A-Z)

    // busca no ranking
    int modoBusca = 0;       // 0 = nenhuma, 1 = linear (nome), 2 = binária (cliques)
    char termo[30] = "";     // texto digitado na busca
    int termoCount = 0;
    int indiceAchado = -1;   // posição achada (-1 = não achou)
    int comparacoes = 0;     // quantas comparações a última busca fez
    int buscou = 0;          // 1 = existe resultado para mostrar
    int tipoUltimaBusca = 0;

    while (!WindowShouldClose()) {
        int x = GetRenderWidth();
        int y = GetRenderHeight();

        // Botão: um pouco à esquerda do centro. Tamanho proporcional à janela.
        // Declarado UMA vez aqui, assim clique e desenho usam sempre o mesmo valor.
        Vector2 cbotao = {(float)x * 0.33f, (float)y * 0.55f};
        float raio = (float)y * 0.22f;

        if (estadoAtual == ESTADO_DIGITANDO_NOME) {
            // Ler caracteres digitados
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && (letraCount < 28)) {
                    jogador.nome[letraCount] = (char)key;
                    jogador.nome[letraCount + 1] = '\0';
                    letraCount++;
                }
                key = GetCharPressed();
            }

            // Apagar com Backspace
            if (IsKeyPressed(KEY_BACKSPACE)) {
                letraCount--;
                if (letraCount < 0) letraCount = 0;
                jogador.nome[letraCount] = '\0';
            }

            // Confirmar com ENTER (só avança se digitar ao menos 1 letra)
            if (IsKeyPressed(KEY_ENTER) && letraCount > 0) {
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
                jogador.upgrades[UPG_AUTO]  = (Upgrade){"Auto Clique", 0, 50, 1}; // efeito = cliques/seg por nível

                // se existir save deste jogador, restaura o progresso
                if (carregarJogo(&jogador, conquistas)) {
                    textoMsg = "Progresso carregado!";
                    tempoMsg = 2.5f;
                }

                estadoAtual = ESTADO_JOGANDO;
            }
        }
        else if (estadoAtual == ESTADO_JOGANDO) {
            int cliqueMouse = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
            int acertouBotao = cliqueMouse && CheckCollisionPointCircle(GetMousePosition(), cbotao, raio);

            if (mostrarRanking && modoBusca != 0) {
                // digitando o termo da busca
                int key = GetCharPressed();
                int limite = (modoBusca == 1) ? 28 : 9;
                while (key > 0) {
                    int ok = (modoBusca == 1) ? (key >= 32 && key <= 125) : (key >= '0' && key <= '9');
                    if (ok && termoCount < limite) {
                        termo[termoCount++] = (char)key;
                        termo[termoCount] = '\0';
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && termoCount > 0) termo[--termoCount] = '\0';

                if (IsKeyPressed(KEY_ENTER) && termoCount > 0) {
                    comparacoes = 0;
                    if (modoBusca == 1) {
                        indiceAchado = buscaLinear(ranking, nRanking, termo, &comparacoes);
                    } else {
                        // a busca binária exige o vetor ordenado por cliques
                        if (ordemPorNome) {
                            ordemPorNome = 0;
                            ordenarRanking(ranking, nRanking, ordemPorNome);
                        }
                        indiceAchado = buscaBinaria(ranking, nRanking, atol(termo), &comparacoes);
                    }
                    tipoUltimaBusca = modoBusca;
                    buscou = 1;
                    modoBusca = 0;
                }
            }
            else {
                // R abre/fecha o ranking: lê o arquivo, soma a partida atual e ordena
                if (IsKeyPressed(KEY_R)) {
                    mostrarRanking = !mostrarRanking;
                    modoBusca = 0; buscou = 0; indiceAchado = -1;
                    if (mostrarRanking) {
                        nRanking = carregarRanking(ranking);
                        if (nRanking < MAX_RANKING) {
                            strcpy(ranking[nRanking].nome, jogador.nome);
                            ranking[nRanking].cliques = jogador.stats.totalCliques;
                            nRanking++;
                        }
                        ordenarRanking(ranking, nRanking, ordemPorNome);
                    }
                }
                if (mostrarRanking) {
                    // TAB troca o critério de ordenação
                    if (IsKeyPressed(KEY_TAB)) {
                        ordemPorNome = !ordemPorNome;
                        ordenarRanking(ranking, nRanking, ordemPorNome);
                        buscou = 0; indiceAchado = -1;
                    }
                    // L = busca linear por nome, B = busca binária por cliques
                    if (IsKeyPressed(KEY_L) || IsKeyPressed(KEY_B)) {
                        modoBusca = IsKeyPressed(KEY_L) ? 1 : 2;
                        termo[0] = '\0'; termoCount = 0;
                        buscou = 0; indiceAchado = -1;
                        while (GetCharPressed() > 0) {} // descarta a letra da própria tecla
                    }
                }
            }

            if (!mostrarRanking && (acertouBotao || IsKeyPressed(KEY_SPACE))) {
                jogador.moedas += valorClique(&jogador);
                jogador.stats.totalCliques++;
                jogador.stats.cliquesManuais++;
                tempoApertado = 0.12f;
                PlaySound(som);
            }
            if (tempoApertado > 0) tempoApertado -= GetFrameTime();

            // Auto Clique: a cada 1 segundo faz os cliques automáticos
            acumAuto += GetFrameTime();
            if (acumAuto >= 1.0f) {
                acumAuto -= 1.0f;
                jogador.moedas += moedasPorSegundo(&jogador);
                jogador.stats.totalCliques += cliquesPorSegundo(&jogador);
            }

            if (jogador.stats.totalCliques >= 25)  conquistas[0][0] = 1;
            if (jogador.stats.totalCliques >= 50)  conquistas[0][1] = 1;
            if (jogador.stats.totalCliques >= 200) conquistas[1][0] = 1;
            if (jogador.stats.totalCliques >= 500) conquistas[1][1] = 1;

            // tecla S salva o jogo no arquivo binário
            if (IsKeyPressed(KEY_S) && modoBusca == 0) {
                textoMsg = salvarJogo(&jogador, conquistas) ? "Jogo salvo!" : "Erro ao salvar!";
                tempoMsg = 2.5f;
            }
            if (tempoMsg > 0) tempoMsg -= GetFrameTime();

            if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
        }

        if (IsKeyPressed(KEY_ESCAPE)) break;

        // --- RENDERIZAÇÃO ---
        BeginDrawing();
        ClearBackground(WHITE);

        if (estadoAtual == ESTADO_DIGITANDO_NOME) {
            DrawRectanglePro((Rectangle){0, 0, (float)x, (float)y}, (Vector2){0,0}, 0.0f, DARKBLUE);

            DrawTextEx(fonte, "BEM-VINDO AO JOGO!", (Vector2){(float)(x / 2 - MeasureTextEx(fonte, "BEM-VINDO AO JOGO!", 32, 1).x / 2), 100}, 32, 1, GOLD);
            DrawTextEx(fonte, "Digite seu nome e pressione ENTER:", (Vector2){(float)(x / 2 - MeasureTextEx(fonte, "Digite seu nome e pressione ENTER:", 20, 1).x / 2), 160}, 20, 1, RAYWHITE);

            // Caixa de texto
            DrawRectangle(x / 2 - 200, 210, 400, 50, LIGHTGRAY);
            DrawRectangleLines(x / 2 - 200, 210, 400, 50, DARKGRAY);

            // Nome digitado
            DrawTextEx(fonte, jogador.nome, (Vector2){(float)(x / 2 - 190), 220}, 24, 1, BLACK);

            if (letraCount == 0) {
                DrawTextEx(fonte, "O nome nao pode estar vazio!", (Vector2){(float)(x / 2 - MeasureTextEx(fonte, "O nome nao pode estar vazio!", 16, 1).x / 2), 280}, 16, 1, RED);
            }
        }
        else if (estadoAtual == ESTADO_JOGANDO) {
            // fundo liso azul-claro
            DrawRectangle(0, 0, x, y, COR_FUNDO);

            // botão: frame 2 (apertado) enquanto segura ou logo após o clique,
            // frame 1 (hover) com o mouse em cima, frame 0 (normal) no resto
            int segurando = (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
                             CheckCollisionPointCircle(GetMousePosition(), cbotao, raio))
                            || IsKeyDown(KEY_SPACE);
            int hover = CheckCollisionPointCircle(GetMousePosition(), cbotao, raio);
            int frame = (segurando || tempoApertado > 0) ? 2 : (hover ? 1 : 0);

            float tam = raio * 2.13f; // largura visível do botão = diâmetro da área de clique
            DrawTexturePro(texBotao,
                (Rectangle){frame * larguraFrame, 0, larguraFrame, (float)texBotao.height},
                (Rectangle){cbotao.x, cbotao.y, tam, tam},
                (Vector2){tam / 2, tam / 2}, 0.0f, WHITE);

            // textos do topo (os mesmos do código original, em azul-escuro por causa do fundo claro)
            DrawTextEx(fonte, TextFormat("%s - Moedas: %ld", jogador.nome, jogador.moedas),
                       (Vector2){20, 20}, 30, 1, COR_NAVY);
            DrawTextEx(fonte, TextFormat("Cliques totais: %ld", jogador.stats.totalCliques),
                       (Vector2){20, 58}, 22, 1, COR_NAVY);
            DrawTextEx(fonte, TextFormat("Cliques manuais: %ld", jogador.stats.cliquesManuais),
                       (Vector2){20, 84}, 22, 1, COR_NAVY);
            DrawTextEx(fonte, TextFormat("Por segundo: %d", moedasPorSegundo(&jogador)),
                       (Vector2){20, 110}, 22, 1, COR_NAVY);

            // caixas de upgrade (lado direito)
            desenharUpgrades(&jogador, fonte, GetMouseX(), GetMouseY(),
                             IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !mostrarRanking);

            // conquistas (canto superior direito, acima das caixas)
            int cx = (int)(x * 0.75f);
            DrawTextEx(fonte, "Conquistas:", (Vector2){(float)cx, 8}, 22, 1, COR_NAVY);
            for (int l = 0; l < 2; l++) {
                for (int c = 0; c < 2; c++) {
                    DrawRectangle(cx + c * 30, 34 + l * 30, 25, 25, conquistas[l][c] ? GOLD : COR_CONQ_OFF);
                    DrawRectangleLines(cx + c * 30, 34 + l * 30, 25, 25, COR_NAVY);
                }
            }

            if (tempoMsg > 0)
                DrawTextEx(fonte, textoMsg, (Vector2){20, 140}, 22, 1, COR_NAVY);

            DrawTextEx(fonte, "ESPAÇO clica | S salva | R ranking | ESC sai",
                       (Vector2){20, (float)(y - 34)}, 22, 1, COR_NAVY);

            // tela de ranking (por cima do jogo)
            if (mostrarRanking) {
                Rectangle painel = {x * 0.15f, y * 0.08f, x * 0.70f, y * 0.84f};
                desenharCaixa(painel, COR_CAIXA);
                DrawTextEx(fonte, "RANKING", (Vector2){painel.x + 20, painel.y + 12}, 30, 1, COR_TEXTO_CLARO);
                DrawTextEx(fonte, ordemPorNome ? "Ordem: nome (A-Z)  |  TAB muda" : "Ordem: cliques (maior primeiro)  |  TAB muda",
                           (Vector2){painel.x + 20, painel.y + 48}, 18, 1, COR_TEXTO_SUAVE);

                // mostra 6 linhas; se o achado estiver mais embaixo, a lista "rola" até ele
                int inicio = (indiceAchado >= 6) ? indiceAchado - 5 : 0;
                int mostrar = (nRanking - inicio) < 6 ? (nRanking - inicio) : 6;
                for (int k = 0; k < mostrar; k++) {
                    int i = inicio + k;
                    float ly = painel.y + 80 + k * 28;
                    if (i == indiceAchado)
                        DrawRectangle((int)painel.x + 10, (int)ly - 3, (int)painel.width - 20, 28, COR_PRECO_OK);
                    DrawTextEx(fonte, TextFormat("%d.", i + 1), (Vector2){painel.x + 20, ly}, 22, 1, COR_TEXTO_CLARO);
                    DrawTextEx(fonte, ranking[i].nome, (Vector2){painel.x + 60, ly}, 22, 1, COR_TEXTO_CLARO);
                    DrawTextEx(fonte, TextFormat("%ld", ranking[i].cliques),
                               (Vector2){painel.x + painel.width - 140, ly}, 22, 1, COR_TEXTO_CLARO);
                }

                // área da busca
                DrawTextEx(fonte, "L: busca linear (nome)   B: busca binária (cliques)",
                           (Vector2){painel.x + 20, painel.y + painel.height - 78}, 18, 1, COR_TEXTO_SUAVE);
                if (modoBusca != 0) {
                    DrawTextEx(fonte, TextFormat("%s: %s_   (ENTER busca)", modoBusca == 1 ? "Nome" : "Cliques", termo),
                               (Vector2){painel.x + 20, painel.y + painel.height - 52}, 20, 1, COR_TEXTO_CLARO);
                } else if (buscou) {
                    const char *tipo = (tipoUltimaBusca == 1) ? "Linear" : "Binária";
                    const char *res = (indiceAchado >= 0)
                        ? TextFormat("%s: achou na posição %d (%d comparações)", tipo, indiceAchado + 1, comparacoes)
                        : TextFormat("%s: não encontrado (%d comparações)", tipo, comparacoes);
                    DrawTextEx(fonte, res, (Vector2){painel.x + 20, painel.y + painel.height - 52}, 20, 1, GOLD);
                }
                DrawTextEx(fonte, "R fecha", (Vector2){painel.x + 20, painel.y + painel.height - 26}, 18, 1, COR_TEXTO_SUAVE);
            }
        }

        EndDrawing();
    }

    if (estadoAtual == ESTADO_JOGANDO) {
        salvarJogo(&jogador, conquistas); // salva automaticamente ao sair
        registrarRanking(&jogador);       // registra a partida no ranking
        free(jogador.upgrades);
    }
    UnloadTexture(texBotao);
    UnloadFont(fonte);
    UnloadSound(som);
    CloseAudioDevice();
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

// quantos cliques automáticos acontecem por segundo
int cliquesPorSegundo(Jogador *j) {
    return j->upgrades[UPG_AUTO].nivel * j->upgrades[UPG_AUTO].efeito;
}

// cada clique automático vale o mesmo que um clique normal
int moedasPorSegundo(Jogador *j) {
    return cliquesPorSegundo(j) * valorClique(j);
}

// retângulo arredondado com borda azul-escura (borda = retângulo maior atrás)
void desenharCaixa(Rectangle r, Color preenchimento) {
    float borda = 3.0f;
    DrawRectangleRounded(r, 0.25f, 8, COR_NAVY);
    Rectangle interno = {r.x + borda, r.y + borda, r.width - 2 * borda, r.height - 2 * borda};
    DrawRectangleRounded(interno, 0.25f, 8, preenchimento);
}

void desenharUpgrades(Jogador *j, Font fonte, int mouseX, int mouseY, int cliqueMouse) {
    int x = GetRenderWidth();
    int y = GetRenderHeight();

    // coluna do lado direito: posições proporcionais ao tamanho da janela
    float larg = x * 0.36f;
    float alt  = y * 0.17f;
    float gap  = y * 0.03f;
    float px   = x * 0.62f;
    float py   = y * 0.27f;

    for (int i = 0; i < NUM_UPGRADES; i++) {
        Upgrade *u = &j->upgrades[i];
        Rectangle caixa = {px, py + i * (alt + gap), larg, alt};

        int hover = CheckCollisionPointRec((Vector2){(float)mouseX, (float)mouseY}, caixa);
        if (hover && cliqueMouse) comprarUpgrade(j, i);

        desenharCaixa(caixa, hover ? COR_CAIXA_HOV : COR_CAIXA);

        // "botão" de preço à direita: verde se dá pra comprar, cinza se não
        Rectangle preco = {caixa.x + caixa.width - larg * 0.30f - 8, caixa.y + alt * 0.20f,
                           larg * 0.30f, alt * 0.60f};
        desenharCaixa(preco, j->moedas >= u->custo ? COR_PRECO_OK : COR_PRECO_NO);

        const char *txtPreco = TextFormat("%d", u->custo);
        Vector2 t = MeasureTextEx(fonte, txtPreco, 20, 1);
        DrawTextEx(fonte, txtPreco,
                   (Vector2){preco.x + (preco.width - t.x) / 2, preco.y + (preco.height - t.y) / 2},
                   20, 1, COR_TEXTO_CLARO);

        // nome e nível
        DrawTextEx(fonte, u->nome, (Vector2){caixa.x + 10, caixa.y + alt * 0.14f}, 18, 1, COR_TEXTO_CLARO);
        DrawTextEx(fonte, TextFormat("Nivel %d", u->nivel),
                   (Vector2){caixa.x + 10, caixa.y + alt * 0.52f}, 16, 1, COR_TEXTO_SUAVE);
    }
}

// ---------- Save em arquivo de texto ----------
// Formato do save.txt:
//   linha 1: nome
//   linha 2: moedas totalCliques cliquesManuais
//   linha 3: nivel e custo de cada upgrade (nivel custo nivel custo ...)
//   linha 4: as 4 conquistas (0 ou 1)
// Retorna 1 se deu certo, 0 se falhou.
int salvarJogo(Jogador *j, int conq[2][2]) {
    FILE *f = fopen(ARQUIVO_SAVE, "w");
    if (f == NULL) return 0;

    fprintf(f, "%s\n", j->nome);
    fprintf(f, "%ld %ld %ld\n", j->moedas, j->stats.totalCliques, j->stats.cliquesManuais);
    for (int i = 0; i < NUM_UPGRADES; i++)
        fprintf(f, "%d %d ", j->upgrades[i].nivel, j->upgrades[i].custo);
    fprintf(f, "\n");
    fprintf(f, "%d %d %d %d\n", conq[0][0], conq[0][1], conq[1][0], conq[1][1]);

    fclose(f);
    return 1;
}

// Lê save.txt. Só restaura se o arquivo existir e for do mesmo nome de jogador.
// Retorna 1 se carregou, 0 caso contrário.
int carregarJogo(Jogador *j, int conq[2][2]) {
    FILE *f = fopen(ARQUIVO_SAVE, "r");
    if (f == NULL) return 0; // ainda não existe save

    char nome[30];
    if (fgets(nome, sizeof(nome), f) == NULL) { fclose(f); return 0; }
    nome[strcspn(nome, "\n")] = '\0'; // tira o \n do final
    if (strcmp(nome, j->nome) != 0) { fclose(f); return 0; }

    long moedas, total, manuais;
    int niveis[NUM_UPGRADES], custos[NUM_UPGRADES], q[4];

    if (fscanf(f, "%ld %ld %ld", &moedas, &total, &manuais) != 3) { fclose(f); return 0; }
    for (int i = 0; i < NUM_UPGRADES; i++)
        if (fscanf(f, "%d %d", &niveis[i], &custos[i]) != 2) { fclose(f); return 0; }
    if (fscanf(f, "%d %d %d %d", &q[0], &q[1], &q[2], &q[3]) != 4) { fclose(f); return 0; }
    fclose(f);

    j->moedas = moedas;
    j->stats.totalCliques = total;
    j->stats.cliquesManuais = manuais;
    for (int i = 0; i < NUM_UPGRADES; i++) {
        j->upgrades[i].nivel = niveis[i];
        j->upgrades[i].custo = custos[i];
    }
    conq[0][0] = q[0]; conq[0][1] = q[1];
    conq[1][0] = q[2]; conq[1][1] = q[3];
    return 1;
}

// ---------- Ranking (arquivo de texto + ordenação) ----------
// Lê ranking.txt (linhas "cliques;nome") para o vetor. Retorna quantos registros leu.
int carregarRanking(Registro v[]) {
    FILE *f = fopen(ARQUIVO_RANKING, "r");
    if (f == NULL) return 0;

    int n = 0;
    long cliques;
    char nome[30];
    while (n < MAX_RANKING && fscanf(f, "%ld;%29[^\n]", &cliques, nome) == 2) {
        strcpy(v[n].nome, nome);
        v[n].cliques = cliques;
        n++;
    }
    fclose(f);
    return n;
}

// Acrescenta a partida atual no final do ranking.txt (modo "a" = append)
void registrarRanking(Jogador *j) {
    FILE *f = fopen(ARQUIVO_RANKING, "a");
    if (f == NULL) return;
    fprintf(f, "%ld;%s\n", j->stats.totalCliques, j->nome);
    fclose(f);
}

// Bubble Sort: porNome = 0 ordena por cliques (decrescente), 1 ordena por nome (A-Z)
void ordenarRanking(Registro v[], int n, int porNome) {
    for (int i = 0; i < n - 1; i++) {
        for (int k = 0; k < n - i - 1; k++) {
            int trocar = porNome ? strcmp(v[k].nome, v[k + 1].nome) > 0
                                 : v[k].cliques < v[k + 1].cliques;
            if (trocar) {
                Registro tmp = v[k];
                v[k] = v[k + 1];
                v[k + 1] = tmp;
            }
        }
    }
}

// ---------- Busca no ranking ----------
// Busca linear: percorre do começo ao fim. Funciona com o vetor em qualquer ordem.
// Retorna a posição (0..n-1) ou -1; comp recebe o número de comparações.
int buscaLinear(Registro v[], int n, const char *nome, int *comp) {
    for (int i = 0; i < n; i++) {
        (*comp)++;
        if (strcmp(v[i].nome, nome) == 0) return i;
    }
    return -1;
}

// Busca binária: o vetor PRECISA estar ordenado por cliques (decrescente).
// A cada passo olha o meio e descarta metade do vetor.
int buscaBinaria(Registro v[], int n, long cliques, int *comp) {
    int ini = 0, fim = n - 1;
    while (ini <= fim) {
        int meio = (ini + fim) / 2;
        (*comp)++;
        if (v[meio].cliques == cliques) return meio;
        if (v[meio].cliques > cliques) ini = meio + 1; // decrescente: o valor está depois
        else fim = meio - 1;
    }
    return -1;
}