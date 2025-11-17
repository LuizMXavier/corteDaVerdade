#ifndef JOGO_H
#define JOGO_H

#include "tautologia.h"

// Estrutura de um nível do jogo
typedef struct {
    const char *codigo;             // "F1", "M3", "D5"...
    const char *formula;            // fórmula-alvo (usa v, ^, ~, ->, <->)
    const char *variaveis_texto;    // descrição das variáveis em linguagem natural
    const char *frase_tautologica;  // Dica em frase natural (aparece após 1º erro)
    ResultadoFormula esperado;      // resultado correto (tautologia, etc.)

    // Sacola de itens (símbolos mostrados na tela) – até 8 itens
    const char *itens[8];
    int num_itens;
} Nivel;

// Quantidade total de níveis (15 no momento)
extern const int NUM_NIVEIS;

// Array com todos os níveis do jogo
extern const Nivel NIVEIS[];

// Função principal do jogo
void jogo_corte_da_verdade(void);

#endif // JOGO_H
