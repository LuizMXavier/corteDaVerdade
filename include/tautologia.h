#ifndef TAUTOLOGIA_H
#define TAUTOLOGIA_H

typedef enum {
    RESULT_TAUTOLOGIA,
    RESULT_CONTRADICAO,
    RESULT_CONTINGENCIA
} ResultadoFormula;

typedef struct {
    int n_variaveis;     
    int n_linhas;       
    int valores_p[8];      
    int valores_q[8];     
    int valores_r[8];      
    int resultado[8];     
} TabelaVerdade;

ResultadoFormula avaliar_expressao(const char *expr, TabelaVerdade *tabela);

void imprimir_tabela(const TabelaVerdade *tabela);

#endif 
