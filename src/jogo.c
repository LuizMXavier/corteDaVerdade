#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "jogo.h"
#include "screen.h"
#include "keyboard.h"

// ---------------------------------------------------------
// Ranking (leaderboard)
// ---------------------------------------------------------

#define MAX_RANK 10
#define NOME_MAX 32
#define RANK_FILE "rank.dat"

typedef struct {
    char nome[NOME_MAX];
    int pontos;
} RankEntry;

static RankEntry g_rank[MAX_RANK];
static int g_rank_count = 0;

// ---------------------------------------------------------
// Definição dos 15 níveis
// ---------------------------------------------------------

const int NUM_NIVEIS = 15;

const Nivel NIVEIS[15] = {
    // --- F: 1 variável (FÁCIL) ---
    {
        "F1",
        "p v ~p",
        "p: O réu é culpado.",
        "O réu é culpado OU NÃO é culpado.",
        RESULT_TAUTOLOGIA,
        { "p", "v", "~", "p", "p", "v", "~", "p" },
        8
    },
    {
        "F2",
        "~(p ^ ~p)",
        "p: A moção é válida.",
        "NÃO é verdade que a moção é válida E NÃO é válida.",
        RESULT_TAUTOLOGIA,
        { "p", "^", "~", "p", "~", "p", "^", "p" },
        8
    },
    {
        "F3",
        "p -> p",
        "p: A evidência é admitida.",
        "SE a evidência é admitida, ENTÃO a evidência é admitida.",
        RESULT_TAUTOLOGIA,
        { "p", "->", "p", "p", "->", "p", "p", "->" },
        8
    },
    {
        "F4",
        "p <-> p",
        "p: O veredito é definitivo.",
        "O veredito é definitivo SE, E SOMENTE SE, o veredito é definitivo.",
        RESULT_TAUTOLOGIA,
        { "p", "<->", "p", "p", "<->", "p", "p", "<->" },
        8
    },
    {
        "F5",
        "~~p -> p",
        "p: A audiência será marcada.",
        "SE NÃO é verdade que a audiência NÃO será marcada, ENTÃO a audiência será marcada.",
        RESULT_TAUTOLOGIA,
        { "~", "~", "p", "->", "p", "~", "p", "->" },
        8
    },

    // --- M: 2 variáveis (MÉDIO) ---
    {
        "M1",
        "p -> (p v q)",
        "p: A corte está em sessão. q: O caso foi arquivado.",
        "SE a corte está em sessão, ENTÃO a corte está em sessão OU o caso foi arquivado.",
        RESULT_TAUTOLOGIA,
        { "p", "->", "p", "v", "q", "p", "v", "q" },
        8
    },
    {
        "M2",
        "(p ^ q) -> p",
        "p: O promotor provou o crime. q: O juiz aceitou a prova.",
        "SE o promotor provou o crime E o juiz aceitou a prova, ENTÃO o promotor provou o crime.",
        RESULT_TAUTOLOGIA,
        { "p", "^", "q", "->", "p", "p", "^", "q" },
        8
    },
    {
        "M3",
        "[p ^ (p -> q)] -> q",
        "p: A lei é constitucional. q: O réu será condenado.",
        "SE a lei é constitucional E (SE a lei é constitucional, ENTÃO o réu será condenado), "
        "ENTÃO o réu será condenado. (Modus Ponens)",
        RESULT_TAUTOLOGIA,
        { "p", "^", "p", "->", "q", "->", "q", "p" },
        8
    },
    {
        "M4",
        "~(p -> q) <-> (p ^ ~q)",
        "p: O oficial testemunhou. q: O réu será solto.",
        "NÃO (SE o oficial testemunhou, ENTÃO o réu será solto) SE, E SOMENTE SE, "
        "o oficial testemunhou E NÃO o réu será solto.",
        RESULT_TAUTOLOGIA,
        { "~", "p", "->", "q", "<->", "p", "^", "~" },
        8
    },
    {
        "M5",
        "[(p v q) ^ ~p] -> q",
        "p: O crime foi premeditado. q: A fiança será negada.",
        "SE o crime foi premeditado OU a fiança será negada, E NÃO o crime foi premeditado, "
        "ENTÃO a fiança será negada. (Silogismo Disjuntivo)",
        RESULT_TAUTOLOGIA,
        { "p", "v", "q", "^", "~", "p", "->", "q" },
        8
    },

    // --- D: 3 variáveis (DIFÍCIL) ---
    {
        "D1",
        "[(p -> q) ^ (q -> r)] -> (p -> r)",
        "p: O depoimento é aceito. q: O júri ouve a prova. r: A decisão é justa.",
        "SE (SE o depoimento é aceito, ENTÃO o júri ouve a prova) E "
        "(SE o júri ouve a prova, ENTÃO a decisão é justa), "
        "ENTÃO (SE o depoimento é aceito, ENTÃO a decisão é justa). "
        "(Silogismo Hipotético)",
        RESULT_TAUTOLOGIA,
        { "p", "->", "q", "^", "q", "->", "r", "p" },
        8
    },
    {
        "D2",
        "[p -> (q ^ r)] -> (p -> q)",
        "p: O crime foi grave. q: A pena é máxima. r: Não há recurso.",
        "SE (SE o crime foi grave, ENTÃO a pena é máxima E NÃO há recurso), "
        "ENTÃO (SE o crime foi grave, ENTÃO a pena é máxima).",
        RESULT_TAUTOLOGIA,
        { "p", "->", "q", "^", "r", "->", "p", "->" },
        8
    },
    {
        "D3",
        "[p -> (q v r)] <-> [(p ^ ~q) -> r]",
        "p: O juiz decide hoje. q: A defesa apela. r: O réu aguarda em liberdade.",
        "(SE o juiz decide hoje, ENTÃO a defesa apela OU o réu aguarda em liberdade) "
        "SE, E SOMENTE SE, (SE o juiz decide hoje E NÃO a defesa apela, "
        "ENTÃO o réu aguarda em liberdade).",
        RESULT_TAUTOLOGIA,
        { "p", "->", "q", "v", "r", "<->", "p", "^" },
        8
    },
    {
        "D4",
        "[(p v q) -> r] <-> [(p -> r) ^ (q -> r)]",
        "p: A prova A é conclusiva. q: A prova B é conclusiva. r: O réu será absolvido.",
        "(SE a prova A é conclusiva OU a prova B é conclusiva, ENTÃO o réu será absolvido) "
        "SE, E SOMENTE SE, (SE a prova A é conclusiva, ENTÃO o réu será absolvido "
        "E SE a prova B é conclusiva, ENTÃO o réu será absolvido). (Lei da Distribuição)",
        RESULT_TAUTOLOGIA,
        { "p", "v", "q", "->", "r", "<->", "p", "->" },
        8
    },
    {
        "D5",
        "[(p ^ q) -> r] <-> [p -> (q -> r)]",
        "p: O ato foi doloso. q: O dano foi causado. r: Haverá indenização.",
        "(SE o ato foi doloso E o dano foi causado, ENTÃO haverá indenização) SE, E SOMENTE SE, "
        "(SE o ato foi doloso, ENTÃO (SE o dano foi causado, ENTÃO haverá indenização)). "
        "(Lei da Exportação)",
        RESULT_TAUTOLOGIA,
        { "p", "^", "q", "->", "r", "<->", "p", "->" },
        8
    }
};

// ---------------------------------------------------------
// Utilidades de desenho / layout
// ---------------------------------------------------------

static void draw_centered(int y, const char *text) {
    int len = (int)strlen(text);
    int x = (MAXX - len) / 2;
    if (x < MINX) x = MINX;
    screenGotoxy(x, y);
    printf("%s", text);
}

static void draw_horizontal_bar(int y, int width) {
    screenGotoxy(SCRSTARTX + 2, y);
    for (int i = 0; i < width; i++) {
        putchar('-');
    }
}

static void draw_wrapped_text(int x, int y, int width, const char *text) {
    int col = 0;
    screenGotoxy(x, y);
    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        if (c == '\n') {
            y++;
            col = 0;
            screenGotoxy(x, y);
            continue;
        }
        if (col >= width) {
            y++;
            col = 0;
            screenGotoxy(x, y);
            continue;
        }
        putchar(c);
        col++;
    }
}

static const char *texto_resultado(ResultadoFormula r) {
    switch (r) {
        case RESULT_TAUTOLOGIA:   return "Tautologia (sempre verdadeira)";
        case RESULT_CONTRADICAO:  return "Contradição (sempre falsa)";
        case RESULT_CONTINGENCIA: return "Contingência (às vezes verdadeira, às vezes falsa)";
        default:                  return "Desconhecido";
    }
}

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
    int permite_q = (tipo != 'F');    // F: não
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
// Ranking: carregar, salvar, atualizar e mostrar
// ---------------------------------------------------------

static int cmp_rank(const void *a, const void *b) {
    const RankEntry *ra = (const RankEntry *)a;
    const RankEntry *rb = (const RankEntry *)b;
    if (rb->pontos != ra->pontos)
        return rb->pontos - ra->pontos; // decrescente
    return 0;
}

static void carregar_rank(void) {
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

static void salvar_rank(void) {
    FILE *f = fopen(RANK_FILE, "w");
    if (!f) return;

    if (g_rank_count > 1)
        qsort(g_rank, g_rank_count, sizeof(RankEntry), cmp_rank);

    for (int i = 0; i < g_rank_count; i++) {
        fprintf(f, "%d;%s\n", g_rank[i].pontos, g_rank[i].nome);
    }
    fclose(f);
}

static void atualizar_rank(const char *nome, int pontos) {
    if (pontos <= 0) return;

    // Se já houver 10, só entra se for maior que o último
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

static void mostrar_rank(void) {
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

    draw_centered(SCRSTARTY + 17, "Pressione ENTER ou ESC para voltar ao menu...");
    screenUpdate();
    (void)esperar_enter_or_esc();
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
    printf("- São 15 níveis (5 fáceis, 5 médios, 5 difíceis).");
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
    printf("- ESC durante o julgamento volta ao menu (com seus pontos).");
    screenGotoxy(x, y++);
    printf("- Ao final, sua pontuação pode entrar no Rank (Top 10).");

    draw_centered(SCRSTARTY + 17, "Pressione ENTER ou ESC para voltar ao menu...");
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
// Jogabilidade de um nível (com checagem de fórmula-alvo)
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

    (void)indice_nivel; // não usado diretamente

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

        // Cabeçalho: CORTE DA VERDADE | Nível | Tentativa | Pontos
        screenSetColor(LIGHTRED, BLACK);
        printf("CORTE DA VERDADE ");
        screenSetColor(WHITE, BLACK);
        printf("| Nível %s ", n->codigo);
        printf("| Tentativa %d/3 ", tentativa);
        printf("| Pontos: %d", *pontuacao);

        draw_horizontal_bar(SCRSTARTY + 4, width);

        // Coordenadas
        int y_juiz       = SCRSTARTY + 6;
        int y_provas_lbl = y_juiz + 3;
        int y_provas_ln  = y_provas_lbl + 1;
        int y_defesa     = y_provas_ln + 2;

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

        // Defesa (dica)
        if (dica_mostrada) {
            screenGotoxy(left, y_defesa);
            printf("Defesa:");

            screenGotoxy(left + 9, y_defesa - 1);
            printf("+------------------------------------------------------+");
            screenGotoxy(left + 9, y_defesa);
            screenSetColor(YELLOW, BLACK);
            printf(" %s", n->frase_tautologica);
            screenSetColor(WHITE, BLACK);
            screenGotoxy(left + 9, y_defesa + 1);
            printf("+------------------------------------------------------+");
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

        // REGRAS DE ACEITAÇÃO: precisa ser tautologia E bater com a fórmula-alvo
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

            // Mensagem centralizada entre comandos e borda
            draw_centered(y_cmd_bottom + 1,
                          "Pressione ENTER ou ESC para continuar...");

            screenUpdate();
            if (esperar_enter_or_esc() != 0) {
                // ESC aqui: encerra partida com pontuação atual
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

        screenSetColor(LIGHTRED, BLACK);
        draw_centered(SCRSTARTY + 1, "PONTUAÇÃO EXTRA PELA ÚLTIMA FÓRMULA");
        screenSetColor(WHITE, BLACK);

        screenGotoxy(SCRSTARTX + 3, SCRSTARTY + 3);
        printf("Classificação da última fórmula: %s", texto_resultado(ultimo_res));

        screenGotoxy(SCRSTARTX + 3, SCRSTARTY + 5);
        printf("Nível %s - Última fórmula digitada:", n->codigo);
        draw_wrapped_text(SCRSTARTX + 3, SCRSTARTY + 6, 70, expr);

        screenGotoxy(SCRSTARTX + 3, SCRSTARTY + 9);
        printf("Tabela-verdade da sua fórmula:");
        screenGotoxy(SCRSTARTX + 3, SCRSTARTY + 10);
        printf("p q r | F");
        screenGotoxy(SCRSTARTX + 3, SCRSTARTY + 11);
        printf("-------------");

        int linha = SCRSTARTY + 12;
        for (int i = 0; i < ultima_tab.n_linhas; i++) {
            screenGotoxy(SCRSTARTX + 3, linha++);
            printf("%d %d %d | %d",
                   ultima_tab.valores_p[i],
                   ultima_tab.valores_q[i],
                   ultima_tab.valores_r[i],
                   ultima_tab.resultado[i]);
        }

        screenGotoxy(SCRSTARTX + 3, linha + 1);
        printf("Linhas com F = Verdadeiro: %d", linhas_V);
        screenGotoxy(SCRSTARTX + 3, linha + 2);
        printf("Pontos extras ganhos neste nível: %d", linhas_V);

        screenGotoxy(SCRSTARTX + 3, linha + 4);
        printf("Fórmula-alvo deste nível:");
        screenGotoxy(SCRSTARTX + 3, linha + 5);
        printf("F_objetivo(p,q,r) = %s", n->formula);

        screenGotoxy(SCRSTARTX + 3, linha + 7);
        printf("Pressione ENTER ou ESC para continuar...");
        screenUpdate();
        if (esperar_enter_or_esc() != 0) {
            return 1;
        }
    }

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
            // ESC durante o julgamento → parar aqui com pontuação atual
            break;
        }

        if (i < NUM_NIVEIS - 1) {
            int proximo_numero = i + 2;
            anim_transicao(proximo_numero);
        }
    }

    // Tela final da partida (mesmo se parou antes do nível 15)
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

    int max_base = NUM_NIVEIS * 50; // 50 pontos por nível (sem extras)
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
                // ESC no nome → volta ao menu
                continue;
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
