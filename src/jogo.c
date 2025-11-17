#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "jogo.h"
#include "screen.h"
#include "keyboard.h"

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
        // sacola específica que você definiu
        { "->", "p", "q", "v", "^", "~", "<->", "p" },
        8
    },
    {
        "F2",
        "~(p ^ ~p)",
        "p: A moção é válida.",
        "NÃO é verdade que a moção é válida E NÃO é válida.",
        RESULT_TAUTOLOGIA,
        // F: só p permitido → não mostramos q/r na sacola
        { "p", "p", "v", "^", "~", "->", "<->", "(" },
        8
    },
    {
        "F3",
        "p -> p",
        "p: A evidência é admitida.",
        "SE a evidência é admitida, ENTÃO a evidência é admitida.",
        RESULT_TAUTOLOGIA,
        { "p", "p", "v", "^", "~", "->", "<->", "(" },
        8
    },
    {
        "F4",
        "p <-> p",
        "p: O veredito é definitivo.",
        "O veredito é definitivo SE, E SOMENTE SE, o veredito é definitivo.",
        RESULT_TAUTOLOGIA,
        { "p", "p", "v", "^", "~", "->", "<->", "(" },
        8
    },
    {
        "F5",
        "~~p -> p",
        "p: A audiência será marcada.",
        "SE NÃO É verdade que a audiência NÃO será marcada, ENTÃO a audiência será marcada.",
        RESULT_TAUTOLOGIA,
        { "p", "p", "v", "^", "~", "->", "<->", "(" },
        8
    },

    // --- M: 2 variáveis (MÉDIO) ---
    {
        "M1",
        "p -> (p v q)",
        "p: A corte está em sessão. q: O caso foi arquivado.",
        "SE a corte está em sessão, ENTÃO a corte está em sessão OU o caso foi arquivado.",
        RESULT_TAUTOLOGIA,
        // M: p e q permitidos (mostramos p,q + conectivos)
        { "p", "q", "v", "^", "~", "->", "<->", "(" },
        8
    },
    {
        "M2",
        "(p ^ q) -> p",
        "p: O promotor provou o crime. q: O juiz aceitou a prova.",
        "SE o promotor provou o crime E o juiz aceitou a prova, ENTÃO o promotor provou o crime.",
        RESULT_TAUTOLOGIA,
        { "p", "q", "v", "^", "~", "->", "<->", "(" },
        8
    },
    {
        "M3",
        "[p ^ (p -> q)] -> q",
        "p: A lei é constitucional. q: O réu será condenado.",
        "SE a lei é constitucional E (SE a lei é constitucional, ENTÃO o réu será condenado), ENTÃO o réu será condenado. (Modus Ponens)",
        RESULT_TAUTOLOGIA,
        { "p", "q", "v", "^", "~", "->", "<->", "(" },
        8
    },
    {
        "M4",
        "~(p -> q) <-> (p ^ ~q)",
        "p: O oficial testemunhou. q: O réu será solto.",
        "NÃO (SE o oficial testemunhou, ENTÃO o réu será solto) SE, E SOMENTE SE, o oficial testemunhou E NÃO o réu será solto.",
        RESULT_TAUTOLOGIA,
        { "p", "q", "v", "^", "~", "->", "<->", "(" },
        8
    },
    {
        "M5",
        "[(p v q) ^ ~p] -> q",
        "p: O crime foi premeditado. q: A fiança será negada.",
        "SE o crime foi premeditado OU a fiança será negada, E NÃO o crime foi premeditado, ENTÃO a fiança será negada. (Silogismo Disjuntivo)",
        RESULT_TAUTOLOGIA,
        { "p", "q", "v", "^", "~", "->", "<->", "(" },
        8
    },

    // --- D: 3 variáveis (DIFÍCIL) ---
    {
        "D1",
        "[(p -> q) ^ (q -> r)] -> (p -> r)",
        "p: O depoimento é aceito. q: O júri ouve a prova. r: A decisão é justa.",
        "SE (SE o depoimento é aceito, ENTÃO o júri ouve a prova) E (SE o júri ouve a prova, ENTÃO a decisão é justa), ENTÃO (SE o depoimento é aceito, ENTÃO a decisão é justa). (Silogismo Hipotético)",
        RESULT_TAUTOLOGIA,
        // D: p,q,r permitidos
        { "p", "q", "r", "v", "^", "~", "->", "<->" },
        8
    },
    {
        "D2",
        "[p -> (q ^ r)] -> (p -> q)",
        "p: O crime foi grave. q: A pena é máxima. r: Não há recurso.",
        "SE (SE o crime foi grave, ENTÃO a pena é máxima E NÃO há recurso), ENTÃO (SE o crime foi grave, ENTÃO a pena é máxima).",
        RESULT_TAUTOLOGIA,
        { "p", "q", "r", "v", "^", "~", "->", "<->" },
        8
    },
    {
        "D3",
        "[p -> (q v r)] <-> [(p ^ ~q) -> r]",
        "p: O juiz decide hoje. q: A defesa apela. r: O réu aguarda em liberdade.",
        "(SE o juiz decide hoje, ENTÃO a defesa apela OU o réu aguarda em liberdade) SE, E SOMENTE SE, (SE o juiz decide hoje E NÃO a defesa apela, ENTÃO o réu aguarda em liberdade).",
        RESULT_TAUTOLOGIA,
        { "p", "q", "r", "v", "^", "~", "->", "<->" },
        8
    },
    {
        "D4",
        "[(p v q) -> r] <-> [(p -> r) ^ (q -> r)]",
        "p: A prova A é conclusiva. q: A prova B é conclusiva. r: O réu será absolvido.",
        "(SE a prova A é conclusiva OU a prova B é conclusiva, ENTÃO o réu será absolvido) SE, E SOMENTE SE, (SE a prova A é conclusiva, ENTÃO o réu será absolvido E SE a prova B é conclusiva, ENTÃO o réu será absolvido). (Lei da Distribuição)",
        RESULT_TAUTOLOGIA,
        { "p", "q", "r", "v", "^", "~", "->", "<->" },
        8
    },
    {
        "D5",
        "[(p ^ q) -> r] <-> [p -> (q -> r)]",
        "p: O ato foi doloso. q: O dano foi causado. r: Haverá indenização.",
        "(SE o ato foi doloso E o dano foi causado, ENTÃO haverá indenização) SE, E SOMENTE SE, (SE o ato foi doloso, ENTÃO (SE o dano foi causado, ENTÃO haverá indenização)). (Lei da Exportação)",
        RESULT_TAUTOLOGIA,
        { "p", "q", "r", "v", "^", "~", "->", "<->" },
        8
    }
};

// ---------------------------------------------------------
// Utilidades auxiliares
// ---------------------------------------------------------

static void draw_centered(int y, const char *text) {
    int len = (int)strlen(text);
    int x = (MAXX - len) / 2;
    if (x < MINX) x = MINX;
    screenGotoxy(x, y);
    printf("%s", text);
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
        case RESULT_CONTRADICAO:  return "Contradicao (sempre falsa)";
        case RESULT_CONTINGENCIA: return "Contingencia (as vezes verdadeira, as vezes falsa)";
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

static void esperar_enter() {
    int ch;
    do {
        ch = readch();
    } while (ch != '\n' && ch != '\r');
}

static void ler_formula(char *expr, size_t tam, int x, int y) {
    size_t len = 0;
    expr[0] = '\0';
    screenGotoxy(x, y);
    screenShowCursor();
    screenUpdate();

    for (;;) {
        int ch = readch();
        if (ch == '\n' || ch == '\r') {
            expr[len] = '\0';
            break;
        } else if (ch == 8 || ch == 127) { // backspace
            if (len > 0) {
                len--;
                expr[len] = '\0';
                screenGotoxy(x + (int)len, y);
                printf(" ");
                screenGotoxy(x + (int)len, y);
                screenUpdate();
            }
        } else if (ch >= 32 && ch < 127) { // caractere imprimível
            if (len < tam - 1) {
                expr[len++] = (char)ch;
                putchar(ch);
                screenUpdate();
            }
        }
    }

    screenHideCursor();
    screenUpdate();
}

// verifica se o jogador usou variáveis permitidas no nível
static int variaveis_validas(const Nivel *n, const char *expr, char *msg, size_t tam_msg) {
    char tipo = n->codigo[0]; // 'F', 'M' ou 'D'
    int permite_q = (tipo != 'F');    // F: não
    int permite_r = (tipo == 'D');    // só D pode r

    for (int i = 0; expr[i] != '\0'; i++) {
        char c = expr[i];
        if (c == 'q' || c == 'Q') {
            if (!permite_q) {
                snprintf(msg, tam_msg,
                         "Neste nivel, apenas a variavel p pode ser usada. Nao use q.");
                return 0;
            }
        } else if (c == 'r' || c == 'R') {
            if (!permite_r) {
                snprintf(msg, tam_msg,
                         "Neste nivel, nao eh permitido usar a variavel r.");
                return 0;
            }
        }
    }
    msg[0] = '\0';
    return 1;
}

// barra de progresso [#####.....] 3/15
static void desenhar_progresso(int numero_nivel) {
    int largura = 20;
    int preenchido = numero_nivel * largura / NUM_NIVEIS;
    if (preenchido > largura) preenchido = largura;

    char barra[32];
    int k = 0;
    barra[k++] = '[';
    for (int i = 0; i < largura; i++) {
        barra[k++] = (i < preenchido) ? '#' : '.';
    }
    barra[k++] = ']';
    barra[k] = '\0';

    char texto[64];
    snprintf(texto, sizeof(texto), "%s %d/%d", barra, numero_nivel, NUM_NIVEIS);
    draw_centered(7, texto);
}

// animação de transição entre níveis
static void anim_transicao(int proximo_numero_nivel) {
    if (proximo_numero_nivel < 1 || proximo_numero_nivel > NUM_NIVEIS) return;

    const char *base = "A CORTE DELIBERA";
    const char *dots = "...";

    for (int i = 0; i <= 3; i++) {
        screenInit(1);
        screenSetColor(LIGHTRED, BLACK);
        draw_centered(5, "CORTE DA VERDADE");
        screenSetColor(WHITE, BLACK);

        char linha[64];
        snprintf(linha, sizeof(linha), "%s%.*s", base, i, dots);
        draw_centered(9, linha);

        char prox[64];
        snprintf(prox, sizeof(prox), "Proximo nivel: %d/%d", proximo_numero_nivel, NUM_NIVEIS);
        draw_centered(11, prox);

        screenUpdate();
        usleep(300000); // 0.3s
    }
}

// ---------------------------------------------------------
// Animação de abertura
// ---------------------------------------------------------

static void intro_animation() {
    const char *titulo = "CORTE DA VERDADE";
    const char *sub    = "Um jogo de logica proposicional";

    screenInit(1);

    screenSetColor(LIGHTRED, BLACK);
    draw_centered(5, titulo);
    screenSetColor(LIGHTGRAY, BLACK);
    draw_centered(7, sub);
    screenSetColor(WHITE, BLACK);
    draw_centered(10, "Voce e o reu. Sua unica defesa:");
    draw_centered(11, "montar tautologias perfeitas.");
    draw_centered(13, "Pressione ENTER para iniciar o julgamento...");
    screenUpdate();

    esperar_enter();
}

// ---------------------------------------------------------
// Um nível: 3 tentativas + pontuação
// ---------------------------------------------------------

static void jogar_nivel(const Nivel *n, int indice_nivel, int *pontuacao) {
    char expr[512];
    int acertou_tautologia = 0;
    int dica_mostrada = 0;
    TabelaVerdade ultima_tab;
    memset(&ultima_tab, 0, sizeof(ultima_tab));
    ResultadoFormula ultimo_res = RESULT_CONTINGENCIA;

    int numero_nivel = indice_nivel + 1;

    for (int tentativa = 1; tentativa <= 3; tentativa++) {
        // redesenha tela do nível
        screenInit(1);

        screenSetColor(LIGHTRED, BLACK);
        draw_centered(2, "CORTE DA VERDADE");
        screenSetColor(WHITE, BLACK);

        char buf[64];
        snprintf(buf, sizeof(buf), "Nivel %s (%d/15) - Tentativa %d/3",
                 n->codigo, numero_nivel, tentativa);
        draw_centered(4, buf);

        snprintf(buf, sizeof(buf), "Pontuacao atual: %d", *pontuacao);
        draw_centered(6, buf);

        // barra de progresso
        desenhar_progresso(numero_nivel);

        // Significado das variáveis
        screenGotoxy(SCRSTARTX + 1, 9);
        printf("Significado das variaveis:");
        draw_wrapped_text(SCRSTARTX + 1, 10, 70, n->variaveis_texto);

        // Sacola de itens, se houver
        if (n->num_itens > 0) {
            screenGotoxy(SCRSTARTX + 1, 12);
            printf("Itens disponiveis na sacola:");
            screenGotoxy(SCRSTARTX + 1, 13);
            printf("|");
            for (int i = 0; i < n->num_itens; i++) {
                printf(" %s |", n->itens[i]);
            }
        }

        // Dica, se habilitada
        if (dica_mostrada) {
            screenGotoxy(SCRSTARTX + 1, 15);
            printf("DICA (frase em linguagem natural):");
            draw_wrapped_text(SCRSTARTX + 1, 16, 70, n->frase_tautologica);
        }

        // Instruções de entrada
        screenGotoxy(SCRSTARTX + 1, 18);
        printf("Digite uma formula usando p, q, r e (v, ^, ~, ->, <->).");
        screenGotoxy(SCRSTARTX + 1, 19);
        printf("Exemplo: p v ~p");

        screenGotoxy(SCRSTARTX + 1, 21);
        printf("F(p,q,r) = ");
        int x_input = SCRSTARTX + 1 + (int)strlen("F(p,q,r) = ");
        screenUpdate();

        ler_formula(expr, sizeof(expr), x_input, 21);

        // valida variáveis usadas
        char msg[160];
        if (!variaveis_validas(n, expr, msg, sizeof(msg))) {
            screenGotoxy(SCRSTARTX + 1, 23);
            printf("%s", msg);
            screenGotoxy(SCRSTARTX + 1, 24);
            printf("Pressione ENTER e tente novamente (nao contou tentativa)...");
            screenUpdate();
            esperar_enter();
            tentativa--; // não consome tentativa
            continue;
        }

        // Avalia expressão
        TabelaVerdade tab;
        ResultadoFormula res = avaliar_expressao(expr, &tab);
        ultima_tab = tab;
        ultimo_res = res;

        screenGotoxy(SCRSTARTX + 1, 23);

        if (res == RESULT_TAUTOLOGIA) {
            int ganho = 0;
            if (tentativa == 1)      ganho = 50;
            else if (tentativa == 2) ganho = 30;
            else                     ganho = 20;

            if (pontuacao) *pontuacao += ganho;

            printf("Sua formula e uma TAUTOLOGIA! Classificacao: %s.", texto_resultado(res));

            screenGotoxy(SCRSTARTX + 1, 24);
            printf("Voce ganhou %d pontos neste nivel. Pressione ENTER para continuar...", ganho);

            screenUpdate();
            esperar_enter();
            acertou_tautologia = 1;
            break;
        } else {
            printf("Nao e tautologia. Classificacao: %s.", texto_resultado(res));

            if (tentativa == 1) {
                screenGotoxy(SCRSTARTX + 1, 24);
                printf("A Corte lhe dara uma dica na proxima tentativa. Pressione ENTER...");
                dica_mostrada = 1;
            } else if (tentativa == 2) {
                screenGotoxy(SCRSTARTX + 1, 24);
                printf("Ultima tentativa! Pressione ENTER para tentar novamente...");
            } else {
                screenGotoxy(SCRSTARTX + 1, 24);
                printf("Fim das tentativas neste nivel. Pressione ENTER...");
            }

            screenUpdate();
            esperar_enter();
        }
    }

    if (!acertou_tautologia) {
        int linhas_V = contar_verdadeiras(&ultima_tab);
        if (pontuacao) *pontuacao += linhas_V;

        screenInit(1);

        screenSetColor(LIGHTRED, BLACK);
        draw_centered(2, "PONTUACAO EXTRA PELA ULTIMA FORMULA");
        screenSetColor(WHITE, BLACK);

        screenGotoxy(SCRSTARTX + 1, 4);
        printf("Classificacao da ultima formula: %s", texto_resultado(ultimo_res));

        screenGotoxy(SCRSTARTX + 1, 5);
        printf("Nivel %s - Ultima formula digitada:", n->codigo);
        draw_wrapped_text(SCRSTARTX + 1, 6, 70, expr);

        screenGotoxy(SCRSTARTX + 1, 9);
        printf("Tabela verdade da sua formula:");
        screenGotoxy(SCRSTARTX + 1, 10);
        printf("p q r | F");
        screenGotoxy(SCRSTARTX + 1, 11);
        printf("-------------");

        int linha = 12;
        for (int i = 0; i < ultima_tab.n_linhas; i++) {
            screenGotoxy(SCRSTARTX + 1, linha++);
            printf("%d %d %d | %d",
                   ultima_tab.valores_p[i],
                   ultima_tab.valores_q[i],
                   ultima_tab.valores_r[i],
                   ultima_tab.resultado[i]);
        }

        screenGotoxy(SCRSTARTX + 1, linha + 1);
        printf("Linhas com F = Verdadeiro: %d", linhas_V);
        screenGotoxy(SCRSTARTX + 1, linha + 2);
        printf("Pontos extras ganhos neste nivel: %d", linhas_V);

        // mostra fórmula-alvo como exemplo de tautologia
        screenGotoxy(SCRSTARTX + 1, linha + 4);
        printf("Exemplo de tautologia para este caso:");
        screenGotoxy(SCRSTARTX + 1, linha + 5);
        printf("F_exemplo(p,q,r) = %s", n->formula);

        screenGotoxy(SCRSTARTX + 1, linha + 7);
        printf("Pressione ENTER para seguir para o proximo nivel...");
        screenUpdate();
        esperar_enter();
    }
}

// ---------------------------------------------------------
// Loop principal
// ---------------------------------------------------------

void jogo_corte_da_verdade(void) {
    int pontuacao = 0;

    intro_animation();

    for (int i = 0; i < NUM_NIVEIS; i++) {
        jogar_nivel(&NIVEIS[i], i, &pontuacao);

        // animação de transição, exceto depois do último nível
        if (i < NUM_NIVEIS - 1) {
            int proximo_numero = i + 2; // níveis são 1..15
            anim_transicao(proximo_numero);
        }
    }

    // tela final
    screenInit(1);
    screenSetColor(LIGHTRED, BLACK);
    draw_centered(5, "FIM DO JULGAMENTO");
    screenSetColor(WHITE, BLACK);

    char buf[64];
    snprintf(buf, sizeof(buf), "Pontuacao final acumulada: %d pontos.", pontuacao);
    draw_centered(8, buf);

    int max_base = NUM_NIVEIS * 50; // 50 pontos por nivel (sem contar extras)
    int faixa_alto = max_base * 80 / 100;  // 80%
    int faixa_medio = max_base * 50 / 100; // 50%

    const char *classif;
    if (pontuacao >= faixa_alto) {
        classif = "Reu ABSOLVIDO (Mestre da logica)!";
    } else if (pontuacao >= faixa_medio) {
        classif = "Reu em LIBERDADE CONDICIONAL (Bom argumentador).";
    } else {
        classif = "Reu CONDENADO (Precisa revisar sua logica).";
    }

    draw_centered(10, classif);
    draw_centered(12, "Obrigado por jogar a CORTE DA VERDADE!");
    draw_centered(14, "Pressione ENTER para encerrar...");

    screenUpdate();
    esperar_enter();
}
