#ifndef TAUTOLOGIA_H
#define TAUTOLOGIA_H

// Resultado global da fórmula
typedef enum {
    RESULT_TAUTOLOGIA,
    RESULT_CONTRADICAO,
    RESULT_CONTINGENCIA
} ResultadoFormula;

// Estrutura com a tabela verdade (até 3 variáveis: p, q, r)
typedef struct {
    int n_variaveis;       // 1, 2 ou 3
    int n_linhas;          // 2^n_variaveis (máx. 8)
    int valores_p[8];      // valores de p em cada linha (0 ou 1)
    int valores_q[8];      // valores de q em cada linha (0 ou 1)
    int valores_r[8];      // valores de r em cada linha (0 ou 1)
    int resultado[8];      // resultado da fórmula em cada linha (0 ou 1)
} TabelaVerdade;

/**
 * Sintaxe aceita na expressão:
 *
 *  - Variáveis: p, q, r (maiúsculas ou minúsculas)
 *  - Conectivos:
 *        v   : disjunção (OU)
 *        ^   : conjunção (E)
 *        ~   : negação
 *        ->  : implicação
 *        <-> : bicondicional
 *  - Agrupamento: parênteses () ou colchetes [].
 *
 * Exemplos:
 *      "p v ~p"
 *      "~(p ^ ~p)"
 *      "p -> (p v q)"
 *      "[(p v q) ^ ~p] -> q"
 *
 * Retorno:
 *  - RESULT_TAUTOLOGIA     : fórmula sempre 1
 *  - RESULT_CONTRADICAO    : fórmula sempre 0
 *  - RESULT_CONTINGENCIA   : mistura de 0 e 1
 *
 * Se `tabela != NULL`, também preenche a TabelaVerdade.
 */
ResultadoFormula avaliar_expressao(const char *expr, TabelaVerdade *tabela);

/**
 * Imprime no stdout a tabela verdade armazenada em `tabela`.
 */
void imprimir_tabela(const TabelaVerdade *tabela);

#endif // TAUTOLOGIA_H
