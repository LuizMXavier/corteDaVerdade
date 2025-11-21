#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "jogo.h"
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

const int NUM_NIVEIS = 9;

const Nivel NIVEIS[9] = {
    // --- F: 1 variável (FÁCIL) ---
    {
        "F1",
        "p v ~p",
        "p: O réu é culpado.",
        "O réu é culpado ou não é culpado.",
        RESULT_TAUTOLOGIA,
        { "p", "v", "~", "p", "p", "v", "~", "p" },
        8
    },
    {
        "F2",
        "~(p ^ ~p)",
        "p: A moção é válida.",
        "Não é verdade que a moção é válida e não é válida.",
        RESULT_TAUTOLOGIA,
        { "p", "^", "~", "p", "~", "p", "^", "p" },
        8
    },
    {
        "F3",
        "p -> p",
        "p: A evidência é admitida.",
        "Se a evidência é admitida, então a evidência é admitida.",
        RESULT_TAUTOLOGIA,
        { "p", "->", "p", "p", "->", "p", "p", "->" },
        8
    },

    // --- M: 2 variáveis (MÉDIO) ---
    {
        "M1",
        "p -> (p v q)",
        "p: A corte está em sessão. q: O caso foi arquivado.",
        "Se a corte está em sessão, então a corte está em sessão ou o caso foi arquivado.",
        RESULT_TAUTOLOGIA,
        { "p", "->", "p", "v", "q", "p", "v", "q" },
        8
    },
    {
        "M2",
        "(p ^ q) -> p",
        "p: O promotor provou o crime. q: O juiz aceitou a prova.",
        "Se o promotor provou o crime e o juiz aceitou a prova,"
        "então o promotor provou o crime.",
        RESULT_TAUTOLOGIA,
        { "p", "^", "q", "->", "p", "p", "^", "q" },
        8
    },
    {
        "M3",
        "[p ^ (p -> q)] -> q",
        "p: A lei é constitucional. q: O réu será condenado.",
        "Se a lei é constitucional e (se a lei é constitucional"
        ",então o réu será condenado),"
        "então o réu será condenado.",
        RESULT_TAUTOLOGIA,
        { "p", "^", "p", "->", "q", "->", "q", "p" },
        8
    },

    // --- D: 3 variáveis (DIFÍCIL) ---
    {
        "D1",
        "[(p -> q) ^ (q -> r)] -> (p -> r)",
        "p: O depoimento e aceito. q: O juri ouve a prova. r: A decisao e justa.",
        "Se (se o depoimento é aceito, então o juri ouve a prova) "
        "e (se o juri ouve a prova, então a decisão é justa), "
        "então (se o depoimento é aceito, então a decisao é justa). ",
        RESULT_TAUTOLOGIA,
        { "p", "->", "q", "^", "q", "->", "r", "p" },
        8
    },
    {
        "D2",
        "[p -> (q ^ r)] -> (p -> q)",
        "p: O crime foi grave. q: A pena e maxima. r: Nao ha recurso.",
        "Se (se o crime foi grave, então a pena é máxima e não há recurso), "
        "então (se o crime foi grave, então a pena é máxima).",
        RESULT_TAUTOLOGIA,
        { "p", "->", "q", "^", "r", "->", "p", "->" },
        8
    },
    {
        "D3",
        "[p -> (q v r)] <-> [(p ^ ~q) -> r]",
        "p: O juiz decide hoje. q: A defesa apela. r: O reu aguarda em liberdade.",
        "(Se o juiz decide hoje, então a defesa apela ou o réu aguarda em liberdade) "
        "se, e somente se, (se o juiz decide hoje e não a defesa apela, "
        "então o réu aguarda em liberdade).",
        RESULT_TAUTOLOGIA,
        { "p", "->", "q", "v", "r", "<->", "p", "^" },
        8
    }
};

// ---------------------------------------------------------
// Fases bônus (uma após cada bloco de dificuldade)
// ---------------------------------------------------------

typedef enum {
    ARG_SOFISMA,
    ARG_SILOGISMO
} TipoArgumento;

typedef struct {
    const char *titulo;
    const char *texto;
    TipoArgumento tipo;
} FaseBonus;

static const FaseBonus FASES_BONUS[3] = {
    {
        "Bônus 1 - Fase Fácil",
        "Se alguns réus que confessam são culpados, então todo réu que confessa é culpado. "
        "Logo, basta confessar para ser culpado.",
        ARG_SOFISMA
    },
    {
        "Bônus 2 - Fase Média",
        "Todo crime premeditado deve ser julgado com mais rigor. "
        "Este crime foi premeditado. Logo, este crime deve ser julgado com mais rigor.",
        ARG_SILOGISMO
    },
    {
        "Bônus 3 - Fase Difícil",
        "Todo laudo falsificado e invalido. Este laudo foi falsificado. "
        "Logo, este laudo e invalido.",
        ARG_SILOGISMO
    }
};



static int contar_verdadeiras(const TabelaVerdade *tab) {
    if (!tab) return 0;
    int count = 0;
    for (int i = 0; i < tab->n_linhas; i++) {
        if (tab->resultado[i] == 1) count++;
    }
    return count;
}

// Remove espaços e normaliza para comparação de fórmulas
static void normalizar_formula(const char *src, char *dst, size_t tam) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j < tam - 1; i++) {
        unsigned char c = (unsigned char)src[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            continue;
        dst[j++] = (char)tolower(c);
    }
    dst[j] = '\0';
}

// ---------------------------------------------------------
// Entrada de teclado: ENTER / ESC e leitura de linha
// ---------------------------------------------------------

// 0 = ENTER, 1 = ESC
static int esperar_enter_or_esc(void) {
    int ch;
    for (;;) {
        ch = readch();
        if (ch == 27) { // ESC
            return 1;
        }
        if (ch == '\n' || ch == '\r') {
            return 0;
        }
    }
}

// 0 = OK, 1 = ESC
static int ler_linha(char *buf, size_t tam, int x, int y) {
    size_t len = 0;
    buf[0] = '\0';
    screenGotoxy(x, y);
    screenShowCursor();
    screenUpdate();

    for (;;) {
        int ch = readch();
        if (ch == 27) { // ESC
            screenHideCursor();
            screenUpdate();
            buf[0] = '\0';
            return 1;
        } else if (ch == '\n' || ch == '\r') {
            buf[len] = '\0';
            break;
        } else if (ch == 8 || ch == 127) { // backspace
            if (len > 0) {
                len--;
                buf[len] = '\0';
                screenGotoxy(x + (int)len, y);
                printf(" ");
                screenGotoxy(x + (int)len, y);
                screenUpdate();
            }
        } else if (ch >= 32 && ch < 127) { // caractere imprimível
            if (len < tam - 1) {
                buf[len++] = (char)ch;
                putchar(ch);
                screenUpdate();
            }
        }
    }

    screenHideCursor();
    screenUpdate();
    return 0;
}

// 0 = OK, 1 = ESC
static int ler_formula(char *expr, size_t tam, int x, int y) {
    return ler_linha(expr, tam, x, y);
}

// ---------------------------------------------------------
// Validação de variáveis permitidas por nível
// ---------------------------------------------------------

static int variaveis_validas(const Nivel *n, const char *expr, char *msg, size_t tam_msg) {
    char tipo = n->codigo[0]; // 'F', 'M' ou 'D'
    int permite_q = (tipo != 'F');    // F: só p
    int permite_r = (tipo == 'D');    // só D pode r

    for (int i = 0; expr[i] != '\0'; i++) {
        char c = expr[i];
        if (c == 'q' || c == 'Q') {
            if (!permite_q) {
                snprintf(msg, tam_msg,
                         "Neste nível, apenas a variável p pode ser usada. Não use q.");
                return 0;
            }
        } else if (c == 'r' || c == 'R') {
            if (!permite_r) {
                snprintf(msg, tam_msg,
                         "Neste nível, não é permitido usar a variável r.");
                return 0;
            }
        }
    }
    msg[0] = '\0';
    return 1;
}



// ---------------------------------------------------------
// Animação de transição e intro
// ---------------------------------------------------------

static void anim_transicao(int proximo_numero_nivel) {
    if (proximo_numero_nivel < 1 || proximo_numero_nivel > NUM_NIVEIS) return;

    const char *base = "A CORTE DELIBERA";
    const char *dots = "...";

    for (int i = 0; i <= 3; i++) {
        screenInit(1);
        screenSetColor(LIGHTRED, BLACK);
        draw_centered(SCRSTARTY + 2, "CORTE DA VERDADE");
        screenSetColor(WHITE, BLACK);

        char linha[64];
        snprintf(linha, sizeof(linha), "%s%.*s", base, i, dots);
        draw_centered(SCRSTARTY + 6, linha);

        char prox[64];
        snprintf(prox, sizeof(prox), "Próximo nível: %d/%d", proximo_numero_nivel, NUM_NIVEIS);
        draw_centered(SCRSTARTY + 8, prox);

        screenUpdate();
        usleep(250000); // 0,25 s
    }
}

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
// Nome do réu
// ---------------------------------------------------------

// retorna 1 = ok, 0 = cancelado (ESC)
static int pedir_nome_reu(char *nome, size_t tam) {
    screenInit(1);

    screenSetColor(LIGHTRED, BLACK);
    draw_centered(SCRSTARTY + 3, "CORTE DA VERDADE");
    screenSetColor(WHITE, BLACK);
    draw_centered(SCRSTARTY + 5, "Informe o nome do Réu:");

    screenGotoxy(SCRSTARTX + 4, SCRSTARTY + 7);
    printf("Nome do Réu: ");
    int x_input = SCRSTARTX + 4 + (int)strlen("Nome do Réu: ");

    if (ler_linha(nome, tam, x_input, SCRSTARTY + 7) != 0) {
        // ESC: volta ao menu
        nome[0] = '\0';
        return 0;
    }

    if (nome[0] == '\0') {
        strncpy(nome, "Anonimo", tam - 1);
        nome[tam - 1] = '\0';
    }

    return 1;
}

// ---------------------------------------------------------
// Fase Bônus
// ---------------------------------------------------------
// retorna 0 = ok, 1 = ESC (abortar partida)

static int jogar_bonus(const FaseBonus *b, int *pontuacao) {
    screenInit(1);

    screenSetColor(LIGHTRED, BLACK);
    draw_centered(SCRSTARTY + 2, "FASE BÔNUS - NOVAS EVIDÊNCIAS");
    screenSetColor(WHITE, BLACK);

    int x = SCRSTARTX + 4;
    int y = SCRSTARTY + 4;

    screenGotoxy(x, y++);
    printf("%s", b->titulo);
    y++;

    screenGotoxy(x, y++);
    printf("A acusação traz novas evidências:");
    draw_wrapped_text(x, y, 70, b->texto);
    y += 4;

    screenGotoxy(x, y++);
    printf("Como o Réu se posiciona?");
    screenGotoxy(x, y++);
    printf("1) Sofisma");
    screenGotoxy(x, y++);
    printf("2) Silogismo");
    screenGotoxy(x, y++);
    printf("3) Permanecer calado");

    char buf[80];
    snprintf(buf, sizeof(buf), "Pontuação atual: %d pontos.", *pontuacao);
    draw_centered(SCRSTARTY + 16, buf);
    draw_centered(SCRSTARTY + 18, "Escolha 1, 2 ou 3 (ESC volta ao menu).");

    screenUpdate();

    for (;;) {
        int ch = readch();
        if (ch == 27) {
            // ESC aborta partida
            return 1;
        } else if (ch == '1' || ch == '2' || ch == '3') {
            int escolha = ch - '0';
            int antiga = *pontuacao;
            const char *msg_res;

            if (escolha == 3) {
                msg_res = "Você permaneceu calado. Sua pontuação foi mantida.";
            } else {
                int correta =
                    ((b->tipo == ARG_SOFISMA && escolha == 1) ||
                     (b->tipo == ARG_SILOGISMO && escolha == 2));
                if (correta) {
                    *pontuacao *= 2;
                    msg_res = "Interpretação correta! Sua pontuação foi dobrada.";
                } else {
                    *pontuacao /= 2;
                    msg_res = "Interpretação equivocada. Você perdeu metade dos pontos.";
                }
            }

            screenInit(1);
            screenSetColor(LIGHTRED, BLACK);
            draw_centered(SCRSTARTY + 3, "RESULTADO DA FASE BÔNUS");
            screenSetColor(WHITE, BLACK);

            char linha1[100];
            snprintf(linha1, sizeof(linha1), "Pontuação anterior: %d", antiga);
            draw_centered(SCRSTARTY + 6, linha1);

            char linha2[100];
            snprintf(linha2, sizeof(linha2), "Pontuação atual: %d", *pontuacao);
            draw_centered(SCRSTARTY + 8, linha2);

            draw_centered(SCRSTARTY + 11, msg_res);

            draw_centered(SCRSTARTY + 15, "Pressione ENTER ou ESC para continuar...");
            screenUpdate();
            (void)esperar_enter_or_esc();
            return 0;
        }
    }
}

// ---------------------------------------------------------
// Jogabilidade de um nível
// ---------------------------------------------------------
// retorna 0 = nível jogado normalmente
//         1 = jogador apertou ESC (abortar partida)

static int jogar_nivel(const Nivel *n, int indice_nivel, int *pontuacao) {
    char expr[512];
    int acertou_tautologia = 0;
    int dica_mostrada = 0;
    TabelaVerdade ultima_tab;
    memset(&ultima_tab, 0, sizeof(ultima_tab));
    ResultadoFormula ultimo_res = RESULT_CONTINGENCIA;

    (void)indice_nivel;

    char alvo_norm[512];
    normalizar_formula(n->formula, alvo_norm, sizeof(alvo_norm));

    // histórico para evitar repetição de expressão no mesmo nível
    char historico[3][512];
    int n_hist = 0;

    for (int tentativa = 1; tentativa <= 3; tentativa++) {
        screenInit(1);

        int left  = SCRSTARTX + 2;
        int width = 70;

        // Barra de topo
        draw_horizontal_bar(SCRSTARTY + 2, width);
        screenGotoxy(left, SCRSTARTY + 3);

        // Cabeçalho
        screenSetColor(LIGHTRED, BLACK);
        printf("CORTE DA VERDADE ");
        screenSetColor(WHITE, BLACK);
        printf("| Nível %s ", n->codigo);
        printf("| Tentativa %d/3 ", tentativa);
        printf("| Pontos: %d", *pontuacao);

        draw_horizontal_bar(SCRSTARTY + 4, width);

        // Coordenadas (subimos um pouco as Provas)
        int y_juiz       = SCRSTARTY + 5;
        int y_provas_lbl = y_juiz + 2;
        int y_provas_ln  = y_provas_lbl + 1;
        int y_defesa     = y_provas_ln + 1;

        int y_cmd_top    = MAXY - 4;
        int y_cmd_text   = y_cmd_top + 1;
        int y_cmd_bottom = y_cmd_top + 2;

        int y_dep_lbl = y_cmd_top - 4;
        int y_dep_inp = y_dep_lbl;
        int y_msg     = y_dep_lbl + 2;

        // Juiz
        screenGotoxy(left, y_juiz);
        printf("Juiz:");
        screenSetColor(WHITE, BLACK);
        screenGotoxy(left + 8, y_juiz - 1);
        printf("+-------------------------------+");
        screenGotoxy(left + 8, y_juiz);
        printf("| Réu, qual a sua defesa?      |");
        screenGotoxy(left + 8, y_juiz + 1);
        printf("+-------------------------------+");

        // Provas
        screenGotoxy(left, y_provas_lbl);
        printf("Use as Provas para seu depoimento:");

        char linha_provas[256];
        linha_provas[0] = '\0';
        if (n->num_itens > 0) {
            strcat(linha_provas, " ");
            for (int i = 0; i < n->num_itens; i++) {
                char temp[32];
                snprintf(temp, sizeof(temp), "|| %s ", n->itens[i]);
                strncat(linha_provas, temp,
                        sizeof(linha_provas) - strlen(linha_provas) - 1);
            }
            strncat(linha_provas, "||",
                    sizeof(linha_provas) - strlen(linha_provas) - 1);
        } else {
            strncpy(linha_provas, "(sem itens definidos)",
                    sizeof(linha_provas) - 1);
            linha_provas[sizeof(linha_provas) - 1] = '\0';
        }

        int len_provas = (int)strlen(linha_provas);
        int x_provas = (MAXX - len_provas) / 2;
        if (x_provas < left) x_provas = left;

        screenSetColor(LIGHTCYAN, BLACK);
        screenGotoxy(x_provas, y_provas_ln);
        printf("%s", linha_provas);
        screenSetColor(WHITE, BLACK);

        // Defesa (dica) com quebra de linha (largura menor pra não estourar)
        if (dica_mostrada) {
            screenGotoxy(left, y_defesa);
            printf("Defesa:");
            screenSetColor(YELLOW, BLACK);
            draw_wrapped_text(left + 9, y_defesa, 60, n->frase_tautologica);
            screenSetColor(WHITE, BLACK);
        }

        // Depoimento
        screenGotoxy(left, y_dep_lbl);
        printf("Depoimento: ");
        int x_input = left + (int)strlen("Depoimento: ");

        // Comandos
        draw_horizontal_bar(y_cmd_top, width);
        screenGotoxy(left, y_cmd_text);
        printf("COMANDOS | ENTER: Validar | ESC: Voltar ao menu");
        draw_horizontal_bar(y_cmd_bottom, width);

        screenUpdate();

        if (ler_formula(expr, sizeof(expr), x_input, y_dep_inp) != 0) {
            // ESC durante digitação → aborta partida
            return 1;
        }

        // Normaliza expressão digitada
        char expr_norm[512];
        normalizar_formula(expr, expr_norm, sizeof(expr_norm));

        // Verifica repetição na mesma fase
        int repetida = 0;
        for (int i = 0; i < n_hist; i++) {
            if (strcmp(expr_norm, historico[i]) == 0) {
                repetida = 1;
                break;
            }
        }
        if (repetida) {
            screenGotoxy(left, y_msg);
            screenSetColor(LIGHTRED, BLACK);
            printf("Essa expressão já foi usada neste nível.");
            screenSetColor(WHITE, BLACK);
            screenGotoxy(left, y_msg + 1);
            printf("Tente uma nova forma de defesa.");
            screenUpdate();
            if (esperar_enter_or_esc() != 0) {
                return 1;
            }
            tentativa--; // não consome tentativa
            continue;
        }
        if (n_hist < 3) {
            strncpy(historico[n_hist], expr_norm, sizeof(historico[n_hist]) - 1);
            historico[n_hist][sizeof(historico[n_hist]) - 1] = '\0';
            n_hist++;
        }

        // Valida variáveis
        char msg[160];
        if (!variaveis_validas(n, expr, msg, sizeof(msg))) {
            screenGotoxy(left, y_msg);
            screenSetColor(LIGHTRED, BLACK);
            printf("%s", msg);
            screenSetColor(WHITE, BLACK);
            screenGotoxy(left, y_msg + 1);
            printf("Pressione ENTER e tente novamente (não contou tentativa)...");
            screenUpdate();
            if (esperar_enter_or_esc() != 0) {
                return 1;
            }
            tentativa--; // não consome tentativa
            continue;
        }

        // Avalia expressão
        TabelaVerdade tab;
        ResultadoFormula res = avaliar_expressao(expr, &tab);
        ultima_tab = tab;
        ultimo_res = res;

        screenGotoxy(left, y_msg);

        // Precisa ser tautologia E bater com a fórmula-alvo
        if (res == RESULT_TAUTOLOGIA && strcmp(expr_norm, alvo_norm) == 0) {
            int ganho = 0;
            if (tentativa == 1)      ganho = 50;
            else if (tentativa == 2) ganho = 30;
            else                     ganho = 20;

            if (pontuacao) *pontuacao += ganho;

            screenSetColor(LIGHTGREEN, BLACK);
            printf("Depoimento aceito!");
            screenSetColor(WHITE, BLACK);

            screenGotoxy(left, y_msg + 1);
            printf("Você ganhou %d pontos neste nível.", ganho);

            draw_centered(y_cmd_bottom + 1,
                          "Pressione ENTER ou ESC para continuar...");

            screenUpdate();
            if (esperar_enter_or_esc() != 0) {
                return 1;
            }
            acertou_tautologia = 1;
            break;
        } else {
            screenSetColor(LIGHTRED, BLACK);
            printf("Depoimento inválido.");
            screenSetColor(WHITE, BLACK);

            screenGotoxy(left, y_msg + 1);
            if (tentativa == 1) {
                printf("A Corte revelará a DEFESA na próxima tentativa.");
                dica_mostrada = 1;
            } else if (tentativa == 2) {
                printf("ÚLTIMA tentativa chegando! Capriche.");
            } else {
                printf("Fim das tentativas deste nível.");
            }

            screenGotoxy(left, y_msg + 3);
            printf("Pressione ENTER ou ESC para continuar...");
            screenUpdate();
            if (esperar_enter_or_esc() != 0) {
                return 1;
            }
        }
    }

    // Se não acertou, aplica regra de pontos extras com tabela-verdade
    if (!acertou_tautologia) {
        int linhas_V = contar_verdadeiras(&ultima_tab);
        if (pontuacao) *pontuacao += linhas_V;

        screenInit(1);

        // Título
        screenSetColor(LIGHTRED, BLACK);
        draw_centered(SCRSTARTY + 1, "PONTUAÇÃO EXTRA PELA ÚLTIMA FÓRMULA");
        screenSetColor(WHITE, BLACK);

        // Começa mais para cima para sobrar espaço embaixo
        int y = SCRSTARTY + 2;

        // Texto de classificação
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("Classificação da última fórmula.");
        

        // Última fórmula digitada
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("Nível %s - Última fórmula digitada:", n->codigo);
        draw_wrapped_text(SCRSTARTX + 3, y, 70, expr);
        y += 3;

        // Tabela-verdade (mais para cima)
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("Tabela-verdade da sua fórmula:");
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("p q r | F");
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("-------------");

        for (int i = 0; i < ultima_tab.n_linhas; i++) {
            screenGotoxy(SCRSTARTX + 3, y++);
            printf("%d %d %d | %d",
                   ultima_tab.valores_p[i],
                   ultima_tab.valores_q[i],
                   ultima_tab.valores_r[i],
                   ultima_tab.resultado[i]);
        }

        // Uma linha em branco
        y += 1;

        // Resumo da pontuação extra
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("Linhas com V: %d", linhas_V);
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("Pontos extras ganhos neste nível: %d", linhas_V);
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("Fórmula-alvo deste nível: %s", n->formula);

        // Mensagem de continuar: ancorada perto do rodapé
        int y_press = SCRENDY;
        draw_centered(y_press, "Pressione ENTER ou ESC para continuar...");

        screenUpdate();
        if (esperar_enter_or_esc() != 0) {
            return 1;
        }
    }

    (void)ultimo_res; // mantido caso queira usar depois

    return 0;
}

// ---------------------------------------------------------
// Execução de uma partida completa
// ---------------------------------------------------------
// retorna a pontuação acumulada (mesmo se ESC no meio)

static int executar_partida(const char *nome_reu) {
    int pontuacao = 0;

    for (int i = 0; i < NUM_NIVEIS; i++) {
        int abortado = jogar_nivel(&NIVEIS[i], i, &pontuacao);
        if (abortado) {
            break; // ESC durante o julgamento
        }

        // Fase bônus após cada bloco
        if (i == 2 || i == 5 || i == 8) {
            int idx_bonus = (i == 2) ? 0 : (i == 5 ? 1 : 2);
            int abortado_bonus = jogar_bonus(&FASES_BONUS[idx_bonus], &pontuacao);
            if (abortado_bonus) {
                break;
            }
        }

        if (i < NUM_NIVEIS - 1) {
            int proximo_numero = i + 2;
            anim_transicao(proximo_numero);
        }
    }

    // Tela final da partida
    screenInit(1);
    screenSetColor(LIGHTRED, BLACK);
    draw_centered(SCRSTARTY + 3, "FIM DO JULGAMENTO");
    screenSetColor(WHITE, BLACK);

    char linha_nome[80];
    snprintf(linha_nome, sizeof(linha_nome), "Réu: %s", nome_reu);
    draw_centered(SCRSTARTY + 5, linha_nome);

    char buf[80];
    snprintf(buf, sizeof(buf), "Pontuação final acumulada: %d pontos.", pontuacao);
    draw_centered(SCRSTARTY + 7, buf);

    int max_base = NUM_NIVEIS * 50; // sem contar bônus / extras
    int faixa_alto = max_base * 80 / 100;
    int faixa_medio = max_base * 50 / 100;

    const char *classif;
    if (pontuacao >= faixa_alto) {
        classif = "Réu ABSOLVIDO (Mestre da lógica)!";
    } else if (pontuacao >= faixa_medio) {
        classif = "Réu em LIBERDADE CONDICIONAL (Bom argumentador).";
    } else {
        classif = "Réu CONDENADO (Precisa revisar sua lógica).";
    }

    draw_centered(SCRSTARTY + 9, classif);
    draw_centered(SCRSTARTY + 12, "Pressione ENTER ou ESC para voltar ao menu...");

    screenUpdate();
    (void)esperar_enter_or_esc();

    return pontuacao;
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
