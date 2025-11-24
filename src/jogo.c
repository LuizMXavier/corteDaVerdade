#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "jogo.h"
#include "niveis.h"
#include "partida.h"
#include "screen.h"
#include "keyboard.h"
#include "ui.h"
#include "ranking.h"


// ---------------------------------------------------------
// Ranking (leaderboard)
// ---------------------------------------------------------



// ---------------------------------------------------------
// Definição dos níveis (9 níveis principais: 3 F, 3 M, 3 D)
// ---------------------------------------------------------


// ---------------------------------------------------------
// Fases bônus (uma após cada bloco de dificuldade)
// ---------------------------------------------------------



static void intro_animation(void) {
    screenInit(1);

    // Título
    screenSetColor(LIGHTRED, BLACK);
    draw_centered(SCRSTARTY + 6, "CORTE DA VERDADE");

    // Frases centrais
    screenSetColor(WHITE, BLACK);
    draw_centered(SCRSTARTY + 8, "Você é o Réu neste tribunal");

    const char *p1 = "Sua única defesa é a ";
    const char *p2 = "VERDADE";
    int len1 = (int)strlen(p1);
    int len2 = (int)strlen(p2);
    int x = (MAXX - (len1 + len2)) / 2;
    if (x < MINX) x = MINX;

    screenGotoxy(x, SCRSTARTY + 9);
    screenSetColor(WHITE, BLACK);
    printf("%s", p1);
    screenSetColor(LIGHTGREEN, BLACK);
    printf("%s", p2);

    // Mensagem inferior
    screenSetColor(YELLOW, BLACK);
    draw_centered(SCRSTARTY + 13, "Pressione ENTER ou ESC para ir ao menu...");
    screenSetColor(WHITE, BLACK);

    screenUpdate();
    (void)esperar_enter_or_esc();
}

// ---------------------------------------------------------
// Menu principal e tutorial
// ---------------------------------------------------------

static int menu_principal(void) {
    for (;;) {
        screenInit(1);

        int left = SCRSTARTX + 4;
        int y = SCRSTARTY + 4;

        screenSetColor(WHITE, BLACK);
        screenGotoxy(left, y++);
        printf("##################################################");
        screenGotoxy(left, y++);
        printf("1. Iniciar Julgamento");
        screenGotoxy(left, y++);
        printf("2. Rank Top 10");
        screenGotoxy(left, y++);
        printf("3. Tutorial");
        screenGotoxy(left, y++);
        printf("4. Sair do jogo");
        screenGotoxy(left, y++);
        printf("##################################################");

        draw_centered(SCRSTARTY + 14, "Escolha uma opção (1-4).");
        screenUpdate();

        int ch = readch();
        if (ch == '1') {
            return 1;
        } else if (ch == '2') {
            return 2;
        } else if (ch == '3') {
            return 3;
        } else if (ch == '4') {
            return 4;
        }
        // ESC ou outras teclas: reexibe menu
    }
}

static void mostrar_tutorial(void) {
    screenInit(1);

    screenSetColor(LIGHTRED, BLACK);
    draw_centered(SCRSTARTY + 1, "TUTORIAL - CORTE DA VERDADE");
    screenSetColor(WHITE, BLACK);

    int x = SCRSTARTX + 4;
    int y = SCRSTARTY + 3;

    screenGotoxy(x, y++);
    printf("- Você é o Réu. Cada nível é um caso lógico.");
    screenGotoxy(x, y++);
    printf("- Use as Provas (p, q, r e conectivos) para montar a DEFESA.");
    screenGotoxy(x, y++);
    printf("- São 9 níveis principais (3 fáceis, 3 médios, 3 difíceis).");
    screenGotoxy(x, y++);
    printf("- Ao final de cada bloco, há uma FASE BÔNUS de interpretação.");
    screenGotoxy(x, y++);
    printf("- Em cada nível existe uma fórmula-alvo específica.");
    screenGotoxy(x, y++);
    printf("- Só será aceito o depoimento que corresponder à fórmula do nível.");
    screenGotoxy(x, y++);
    printf("- Você tem 3 tentativas por nível:");
    screenGotoxy(x + 2, y++);
    printf("* 1a tentativa correta: 50 pontos");
    screenGotoxy(x + 2, y++);
    printf("* 2a tentativa correta: 30 pontos");
    screenGotoxy(x + 2, y++);
    printf("* 3a tentativa correta: 20 pontos");
    screenGotoxy(x, y++);
    printf("- Se errar todas, a Corte analisa sua última fórmula e");
    screenGotoxy(x, y++);
    printf("  cada linha VERDADEIRA da tabela-verdade vale 1 ponto extra.");
    screenGotoxy(x, y++);
    printf("- Não é permitido repetir a mesma expressão no mesmo nível.");
    screenGotoxy(x, y++);
    printf("- Nas fases bônus:");
    screenGotoxy(x + 2, y++);
    printf("* Sofisma/Silogismo correto: sua pontuação é dobrada.");
    screenGotoxy(x + 2, y++);
    printf("* Resposta errada: você perde metade dos pontos.");
    screenGotoxy(x + 2, y++);
    printf("* Permanecer calado: sua pontuação é mantida.");
    screenGotoxy(x, y++);
    printf("- ESC durante o julgamento volta ao menu (com seus pontos).");
    screenGotoxy(x, y++);
    printf("- Ao final, sua pontuação pode entrar no Rank (Top 10).");

    draw_centered(SCRSTARTY + 19, "Pressione ENTER ou ESC para voltar ao menu...");
    screenUpdate();
    (void)esperar_enter_or_esc();
}



// ---------------------------------------------------------
// Loop principal (menu + jogo + rank)
// ---------------------------------------------------------

void jogo_corte_da_verdade(void) {
    carregar_rank();

    intro_animation();

    for (;;) {
        int opcao = menu_principal();
        if (opcao == 4) { // Sair do jogo
            break;
        } else if (opcao == 1) {
            char nome_reu[NOME_MAX];
            if (!pedir_nome_reu(nome_reu, sizeof(nome_reu))) {
                continue; // ESC no nome
            }
            int pontos = executar_partida(nome_reu);
            atualizar_rank(nome_reu, pontos);
            salvar_rank();
        } else if (opcao == 2) {
            mostrar_rank();
        } else if (opcao == 3) {
            mostrar_tutorial();
        }
    }

    screenInit(1);
    screenSetColor(WHITE, BLACK);
    draw_centered(SCRSTARTY + 8, "Obrigado por jogar a CORTE DA VERDADE!");
    draw_centered(SCRSTARTY + 10, "Pressione ENTER ou ESC para sair...");
    screenUpdate();
    (void)esperar_enter_or_esc();
}
// 0 = ENTER, 1 = ESC
int esperar_enter_or_esc(void) {
    for (;;) {
        int ch = readch();   // função da cli-lib (keyboard.h)

        if (ch == 27) {      // ESC
            return 1;
        }

        if (ch == '\n' || ch == '\r') {  // ENTER
            return 0;
        }

        // Qualquer outra tecla é ignorada
    }
}
