#include <stdio.h>
#include <ctype.h>
#include "tautologia.h"

// ---------- PARSER PARA OS CONECTIVOS v, ^, ~, ->, <-> ----------

typedef struct {
    const char *input;
    int pos;
    int val_p, val_q, val_r;
} Parser;

static void skip_ws(Parser *p) {
    while (p->input[p->pos] == ' ' || p->input[p->pos] == '\t')
        p->pos++;
}

static int match_char(Parser *p, char c) {
    skip_ws(p);
    if (p->input[p->pos] == c) {
        p->pos++;
        return 1;
    }
    return 0;
}

static int match_str(Parser *p, const char *s) {
    skip_ws(p);
    int i = 0;
    int start = p->pos;
    while (s[i] != '\0') {
        if (p->input[start + i] != s[i]) return 0;
        i++;
    }
    p->pos = start + i;
    return 1;
}

static int parse_expr(Parser *p); // forward

// primary := VAR | '(' expr ')' | '[' expr ']'
static int parse_primary(Parser *p) {
    skip_ws(p);
    char c = p->input[p->pos];

    if (c == '(' || c == '[') {
        char close = (c == '(') ? ')' : ']';
        p->pos++;
        int val = parse_expr(p);
        skip_ws(p);
        if (p->input[p->pos] == close) p->pos++;
        return val;
    }

    if (c == 'p' || c == 'P') {
        p->pos++;
        return p->val_p;
    }
    if (c == 'q' || c == 'Q') {
        p->pos++;
        return p->val_q;
    }
    if (c == 'r' || c == 'R') {
        p->pos++;
        return p->val_r;
    }

    // caractere inesperado ou fim da string: retorna 0 para não quebrar
    if (c == '\0' || c == '\n') {
        return 0;
    }

    // ignora caractere desconhecido
    p->pos++;
    return 0;
}

// unary := '~' unary | primary
static int parse_unary(Parser *p) {
    skip_ws(p);
    if (match_char(p, '~')) {
        int val = parse_unary(p);
        return !val;
    }
    return parse_primary(p);
}

// and := unary ('^' unary)*
static int parse_and(Parser *p) {
    int left = parse_unary(p);
    for (;;) {
        skip_ws(p);
        if (match_char(p, '^')) {
            int right = parse_unary(p);
            left = left && right;
        } else {
            break;
        }
    }
    return left;
}

// or := and ('v' and)*
static int parse_or(Parser *p) {
    int left = parse_and(p);
    for (;;) {
        skip_ws(p);
        char c = p->input[p->pos];
        if (c == 'v' || c == 'V') {
            p->pos++;
            int right = parse_and(p);
            left = left || right;
        } else {
            break;
        }
    }
    return left;
}

// impl := or ('->' or)*
static int parse_impl(Parser *p) {
    int left = parse_or(p);
    for (;;) {
        skip_ws(p);
        int start = p->pos;
        if (match_str(p, "->")) {
            int right = parse_or(p);
            left = (!left) || right;
        } else {
            p->pos = start;
            break;
        }
    }
    return left;
}

// bicond := impl ('<->' impl)*
static int parse_bicond(Parser *p) {
    int left = parse_impl(p);
    for (;;) {
        skip_ws(p);
        int start = p->pos;
        if (match_str(p, "<->")) {
            int right = parse_impl(p);
            // equivalência: (A ∧ B) v (~A ∧ ~B)
            left = (left && right) || (!left && !right);
        } else {
            p->pos = start;
            break;
        }
    }
    return left;
}

static int parse_expr(Parser *p) {
    return parse_bicond(p);
}

// avalia a fórmula para valores específicos de p, q, r
static int avaliar_formula(const char *expr, int vp, int vq, int vr) {
    Parser p;
    p.input = expr;
    p.pos   = 0;
    p.val_p = vp;
    p.val_q = vq;
    p.val_r = vr;
    int val = parse_expr(&p);
    return val ? 1 : 0;
}

// ------------- DETECÇÃO DE VARIÁVEIS USADAS P, Q, R ---------------

static int usa_variavel(const char *expr, char minuscula, char maiuscula) {
    for (int i = 0; expr[i] != '\0'; i++) {
        if (expr[i] == minuscula || expr[i] == maiuscula) return 1;
    }
    return 0;
}

// ------------- AVALIA EXPRESSÃO E MONTA TABELA VERDADE ------------

ResultadoFormula avaliar_expressao(const char *expr_original,
                                   TabelaVerdade *tabela) {
    int usa_p = usa_variavel(expr_original, 'p', 'P');
    int usa_q = usa_variavel(expr_original, 'q', 'Q');
    int usa_r = usa_variavel(expr_original, 'r', 'R');

    int n_var = usa_p + usa_q + usa_r;
    if (n_var == 0) {
        // sem variáveis: vamos tratar como se tivesse 1 variável
        n_var = 1;
    }

    int n_linhas = 1 << n_var; // 2^n_var (máximo 8)

    if (tabela) {
        tabela->n_variaveis = n_var;
        tabela->n_linhas    = n_linhas;
    }

    int todas_verdadeiras = 1;
    int todas_falsas      = 1;

    for (int linha = 0; linha < n_linhas; linha++) {
        int p = 0, q = 0, r = 0;

        // Mapeia bits da linha nas variáveis que realmente aparecem
        int bit_index = 0;
        if (usa_p) {
            p = (linha >> bit_index) & 1;
            bit_index++;
        }
        if (usa_q) {
            q = (linha >> bit_index) & 1;
            bit_index++;
        }
        if (usa_r) {
            r = (linha >> bit_index) & 1;
            bit_index++;
        }

        int valor = avaliar_formula(expr_original, p, q, r);

        if (tabela) {
            tabela->valores_p[linha] = p;
            tabela->valores_q[linha] = q;
            tabela->valores_r[linha] = r;
            tabela->resultado[linha] = valor;
        }

        if (valor) {
            todas_falsas = 0;
        } else {
            todas_verdadeiras = 0;
        }
    }

    if (todas_verdadeiras) return RESULT_TAUTOLOGIA;
    if (todas_falsas)      return RESULT_CONTRADICAO;
    return RESULT_CONTINGENCIA;
}

// ------------- IMPRESSÃO SIMPLES DA TABELA VERDADE ----------------

void imprimir_tabela(const TabelaVerdade *tabela) {
    if (!tabela) return;

    printf("Tabela verdade (%d variaveis, %d linhas):\n",
           tabela->n_variaveis, tabela->n_linhas);
    printf("p q r | F\n");
    printf("-------------\n");

    for (int i = 0; i < tabela->n_linhas; i++) {
        printf("%d %d %d | %d\n",
               tabela->valores_p[i],
               tabela->valores_q[i],
               tabela->valores_r[i],
               tabela->resultado[i]);
    }
}

#ifdef TESTE_TAUTOLOGIA
// Pequeno main de teste manual (opcional)
int main(void) {
    const char *exprs[] = {
        "p v ~p",
        "~(p ^ ~p)",
        "p -> p",
        "p <-> p",
        "~~p -> p",
        "p -> (p v q)",
        "[(p v q) ^ ~p] -> q",
        "[(p -> q) ^ (q -> r)] -> (p -> r)",
        NULL
    };

    for (int i = 0; exprs[i] != NULL; i++) {
        printf("Expressao: %s\n", exprs[i]);
        TabelaVerdade tab;
        ResultadoFormula res = avaliar_expressao(exprs[i], &tab);
        imprimir_tabela(&tab);

        if (res == RESULT_TAUTOLOGIA)
            printf("=> TAUTOLOGIA\n\n");
        else if (res == RESULT_CONTRADICAO)
            printf("=> CONTRADICAO\n\n");
        else
            printf("=> CONTINGENCIA\n\n");
    }

    return 0;
}
#endif
