// src/partida.c

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "jogo.h"
#include "niveis.h"
#include "tautologia.h"
#include "screen.h"
#include "keyboard.h"
#include "ui.h"
#include "ranking.h"
#include "partida.h"

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
static int ler_tecla_filtrando_setas(void) {
    int ch = readch();

    if (ch != 27) {
        return ch;
    }

    // Pode ser ESC sozinho ou início de sequência (setas).
    // Espera um pouco para ver se chegam mais bytes.
    usleep(20000); // 20 ms

    if (!keyhit()) {
        // Nada mais no buffer -> ESC sozinho.
        return 27;
    }

    // Tem mais coisa depois do ESC: consome tudo (sequência de seta, F1 etc.)
    while (keyhit()) {
        (void)readch();
    }

    // Ignora a tecla especial
    return 0;
}
static int ler_linha(char *buf, size_t tam, int x, int y) {
    size_t len = 0;
    buf[0] = '\0';
    screenGotoxy(x, y);
    screenShowCursor();
    screenUpdate();

    for (;;) {
        int ch = ler_tecla_filtrando_setas();

        if (ch == 0) {
            // seta ou tecla especial: ignora
            continue;
        }

        if (ch == 27) { // ESC "real"
            screenHideCursor();
            screenUpdate();
            buf[0] = '\0';
            return 1;
        } else if (ch == '\n' || ch == '\r') {
            buf[len] = '\0';
            screenHideCursor();
            screenUpdate();
            return 0;
        } else if (ch == 8 || ch == 127) { // backspace
            if (len > 0) {
                len--;
                buf[len] = '\0';
                screenGotoxy(x + (int)len, y);
                printf(" ");
                screenGotoxy(x + (int)len, y);
                screenUpdate();
            }
        } else if (isprint(ch)) {
            if (len < tam - 1) {
                buf[len++] = (char)ch;
                buf[len] = '\0';
                printf("%c", ch);
                screenUpdate();
            }
        }
    }
}
static int ler_formula(char *expr, size_t tam, int x, int y) {
    return ler_linha(expr, tam, x, y);
}
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
                         "Neste nível, apenas as variáveis p e q podem ser usadas. Não use r.");
                return 0;
            }
        }
    }
    msg[0] = '\0';
    return 1;
}
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
int pedir_nome_reu(char *nome, size_t tam) {
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
        int ch = ler_tecla_filtrando_setas();

        if (ch == 0) {
            // seta / tecla especial: ignora
            continue;
        }

        if (ch == 27) {
            // ESC aborta partida
            return 1;
        } else if (ch == '1' || ch == '2' || ch == '3') {
            int escolha = ch - '0';

            if (escolha == 3) {
                screenSetColor(YELLOW, BLACK);
                draw_centered(SCRSTARTY + 20, "O Réu permaneceu calado. A Corte segue em frente...");
                screenUpdate();
                usleep(1500000);
                return 0;
            }

            int correta = ((escolha == 1 && b->tipo == ARG_SOFISMA) ||
                           (escolha == 2 && b->tipo == ARG_SILOGISMO));

            if (correta) {
                *pontuacao = *pontuacao * 2;
                screenSetColor(LIGHTGREEN, BLACK);
                draw_centered(SCRSTARTY + 20, "Análise correta! Seus pontos foram duplicados!");
            } else {
                *pontuacao = *pontuacao / 2;
                screenSetColor(LIGHTRED, BLACK);
                draw_centered(SCRSTARTY + 20, "Análise equivocada... perdeu metado dos pontos.");
            }

            screenUpdate();
            (void)esperar_enter_or_esc();
            return 0;
        }
    }
}
static int jogar_nivel(const Nivel *n, int indice_nivel, int *pontuacao) {
    char expr[512];
    char ultima_expr[512] = "";
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

        // Coordenadas (layout igual à versão funcional anterior)
        int y_juiz       = SCRSTARTY + 6;
        int y_provas_lbl = y_juiz + 2.3;
        int y_provas_ln  = y_provas_lbl + 1.3;
        int y_defesa     = y_provas_ln + 2;

        int y_cmd_top    = MAXY - 4;
        int y_cmd_text   = y_cmd_top + 1;
        int y_cmd_bottom = y_cmd_top + 2;

        int y_dep_lbl = y_cmd_top - 4;
        int y_dep_inp = y_dep_lbl;
        int y_msg     = y_dep_lbl + 2;

        // Juiz (caixinha)
        screenGotoxy(left, y_juiz);
        printf("Juiz: | Réu, qual a sua defesa? |");
        screenSetColor(YELLOW, BLACK);
        

        screenGotoxy(left, y_provas_lbl);
        printf("Use as Provas para seu depoimento:");

        // Provas
        screenGotoxy(left, y_provas_lbl);
        printf("Use as Provas para seu depoimento:");

        screenSetColor(LIGHTCYAN, BLACK);
        screenGotoxy(left, y_provas_ln);

        if (n->num_itens > 0) {
            printf(" ");
            for (int i = 0; i < n->num_itens; i++) {
                printf("|| %s ", n->itens[i]);
            }
            printf("||");
        } else {
            printf("(sem itens definidos)");
        }

        screenSetColor(WHITE, BLACK);


        // Defesa (dica) com quebra de linha
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

        // Lê expressão do jogador
        memset(expr, 0, sizeof(expr));
        if (ler_formula(expr, sizeof(expr), x_input, y_dep_inp) != 0) {
            // ESC aborta partida
            return 1;
        }

        if (strlen(expr) == 0) {
            screenGotoxy(left, y_msg);
            screenSetColor(LIGHTRED, BLACK);
            printf("Depoimento vazio não é aceito pela Corte.");
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
            printf("A Corte já ouviu exatamente esse depoimento nesta fase.");
            screenSetColor(WHITE, BLACK);
            screenGotoxy(left, y_msg + 1);
            printf("Tente uma formulação diferente (não contou tentativa).");
            screenUpdate();
            if (esperar_enter_or_esc() != 0) {
                return 1;
            }
            tentativa--; // não consome
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
        // Avalia expressão
    TabelaVerdade tab;
    ResultadoFormula res = avaliar_expressao(expr, &tab);
    ultima_tab = tab;
    ultimo_res = res;
    strncpy(ultima_expr, expr, sizeof(ultima_expr) - 1);
    ultima_expr[sizeof(ultima_expr) - 1] = '\0';

    // Resultado esperado: tautologia (ou o que estiver em n->esperado)
    int correta = (res == n->esperado);

    screenGotoxy(left, y_msg);

    if (correta) {
        // Pontos conforme a tentativa: 1ª = 50, 2ª = 30, 3ª = 20
        int pontos_ganhos;
        if (tentativa == 1) {
            pontos_ganhos = 50;
        } else if (tentativa == 2) {
            pontos_ganhos = 30;
        } else {
            pontos_ganhos = 20;
        }

        screenSetColor(LIGHTGREEN, BLACK);
        printf("Depoimento consistente com a lógica da Corte.");
        screenSetColor(WHITE, BLACK);

        *pontuacao += pontos_ganhos;
        acertou_tautologia = 1;
        } else {
        screenSetColor(LIGHTRED, BLACK);
        printf("Depoimento NÃO condiz com a lógica da Corte.");
        screenSetColor(WHITE, BLACK);
        // Nenhuma perda de pontos em caso de erro
    }


        screenGotoxy(left, y_msg + 1);
        printf("Classificação da fórmula: ");
        switch (res) {
        case RESULT_TAUTOLOGIA:
            printf("TAUTOLOGIA");
            break;
        case RESULT_CONTRADICAO:
            printf("CONTRADIÇÃO");
            break;
        default:
            printf("CONTINGÊNCIA");
            break;
        }

        screenGotoxy(left, y_msg + 3);
        printf("Pressione ENTER para continuar...");
        screenUpdate();
        if (esperar_enter_or_esc() != 0) {
            return 1;
        }

        if (correta) {
            break;  // acertou antes de acabar as tentativas
        } else if (tentativa == 1) {
            dica_mostrada = 1; // mostra dica a partir da 2ª tentativa
        } else if (tentativa == 3) {
            break; // acabou as tentativas
        }
    }

    // Tela de pontuação extra pela última fórmula (se não acertou a tautologia)
    if (!acertou_tautologia) {
        int linhas_V = contar_verdadeiras(&ultima_tab);
        if (pontuacao) *pontuacao += linhas_V;

        screenInit(1);

        // Título
        screenSetColor(LIGHTRED, BLACK);
        draw_centered(SCRSTARTY + 1, "PONTUAÇÃO EXTRA PELA ÚLTIMA FÓRMULA");
        screenSetColor(WHITE, BLACK);

        int y = SCRSTARTY + 2;

        // Texto de classificação
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("Classificação da última fórmula.");

        // Última fórmula digitada
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("Nível %s - Última fórmula digitada:", n->codigo);
        draw_wrapped_text(SCRSTARTX + 3, y, 70, ultima_expr[0] ? ultima_expr : expr);
        y += 3;

        // Tabela-verdade
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("Tabela-verdade da sua fórmula:");
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("p q r | V");
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("-------------");

        for (int i = 0; i < ultima_tab.n_linhas; i++) {
    screenGotoxy(SCRSTARTX + 3, y++);
    printf("%c %c %c | %c",
           ultima_tab.valores_p[i] ? 'V' : 'F',
           ultima_tab.valores_q[i] ? 'V' : 'F',
           ultima_tab.valores_r[i] ? 'V' : 'F',
           ultima_tab.resultado[i] ? 'V' : 'F');
}

        y++;
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("Pontos extras ganhos neste nível: %d", linhas_V);
        screenGotoxy(SCRSTARTX + 3, y++);
        printf("Fórmula-alvo deste nível: %s", n->formula);

        int y_press = SCRENDY;
        draw_centered(y_press, "Pressione ENTER ou ESC para continuar...");

        screenUpdate();
        if (esperar_enter_or_esc() != 0) {
            return 1;
        }
    }

    (void)ultimo_res; // reservado para uso futuro

    return 0;
}
int executar_partida(const char *nome_reu) {
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
