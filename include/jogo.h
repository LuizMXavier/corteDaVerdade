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



// Função principal do jogo
void jogo_corte_da_verdade(void);

// Função principal do jogo
void jogo_corte_da_verdade(void);

// Espera ENTER ou ESC (0 = ENTER, 1 = ESC)
int esperar_enter_or_esc(void);


#endif // JOGO_H
