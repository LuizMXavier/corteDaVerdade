#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "screen.h"
#include "keyboard.h"
#include "ui.h"
#include "ranking.h"

#define MAX_RANK 10
#define RANK_FILE "rank.dat"

typedef struct {
    char nome[NOME_MAX];
    int pontos;
} RankEntry;

static RankEntry g_rank[MAX_RANK];
static int g_rank_count = 0;

static int esperar_enter_or_esc_ranking(void) {
    int ch;
    for (;;) {
        ch = readch();
        if (ch == 27) {
            return 1;
        }
        if (ch == '\n' || ch == '\r') {
            return 0;
        }
    }
}
static int cmp_rank(const void *a, const void *b) {
    const RankEntry *ra = (const RankEntry *)a;
    const RankEntry *rb = (const RankEntry *)b;
    if (rb->pontos != ra->pontos)
        return rb->pontos - ra->pontos; // decrescente
    return 0;
}
void carregar_rank(void) {
    g_rank_count = 0;

    FILE *f = fopen(RANK_FILE, "r");
    if (!f) return;

    char linha[128];
    while (fgets(linha, sizeof(linha), f) && g_rank_count < MAX_RANK) {
        int pts;
        char nome[NOME_MAX];
        if (sscanf(linha, "%d;%31[^\n]", &pts, nome) == 2) {
            g_rank[g_rank_count].pontos = pts;
            strncpy(g_rank[g_rank_count].nome, nome, NOME_MAX - 1);
            g_rank[g_rank_count].nome[NOME_MAX - 1] = '\0';
            g_rank_count++;
        }
    }
    fclose(f);

    if (g_rank_count > 1)
        qsort(g_rank, g_rank_count, sizeof(RankEntry), cmp_rank);
}
void salvar_rank(void) {
    FILE *f = fopen(RANK_FILE, "w");
    if (!f) return;

    if (g_rank_count > 1)
        qsort(g_rank, g_rank_count, sizeof(RankEntry), cmp_rank);

    for (int i = 0; i < g_rank_count; i++) {
        fprintf(f, "%d;%s\n", g_rank[i].pontos, g_rank[i].nome);
    }
    fclose(f);
}
void atualizar_rank(const char *nome, int pontos) {
    if (pontos <= 0) return;

    if (g_rank_count == MAX_RANK && pontos <= g_rank[g_rank_count - 1].pontos)
        return;

    if (g_rank_count < MAX_RANK) {
        g_rank[g_rank_count].pontos = pontos;
        strncpy(g_rank[g_rank_count].nome, nome, NOME_MAX - 1);
        g_rank[g_rank_count].nome[NOME_MAX - 1] = '\0';
        g_rank_count++;
    } else {
        g_rank[g_rank_count - 1].pontos = pontos;
        strncpy(g_rank[g_rank_count - 1].nome, nome, NOME_MAX - 1);
        g_rank[g_rank_count - 1].nome[NOME_MAX - 1] = '\0';
    }

    if (g_rank_count > 1)
        qsort(g_rank, g_rank_count, sizeof(RankEntry), cmp_rank);
}
void mostrar_rank(void) {
    screenInit(1);

    screenSetColor(LIGHTRED, BLACK);
    draw_centered(SCRSTARTY + 2, "RANK - CORTE DA VERDADE");
    screenSetColor(WHITE, BLACK);

    if (g_rank_count == 0) {
        draw_centered(SCRSTARTY + 5, "Ainda não há registros no leaderboard.");
    } else {
        int y = SCRSTARTY + 5;
        screenGotoxy(SCRSTARTX + 4, y++);
        printf("Pos | Nome do Réu                  | Pontos");
        screenGotoxy(SCRSTARTX + 4, y++);
        printf("-----------------------------------------------");

        for (int i = 0; i < g_rank_count; i++) {
            screenGotoxy(SCRSTARTX + 4, y++);
            printf("%2d. | %-26s | %5d",
                   i + 1,
                   g_rank[i].nome,
                   g_rank[i].pontos);
        }
    }

    draw_centered(SCRSTARTY + 20, "Pressione ENTER ou ESC para voltar ao menu...");
    screenUpdate();
    (void)esperar_enter_or_esc_ranking();
}
