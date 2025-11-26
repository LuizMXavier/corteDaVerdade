#ifndef JOGO_H
#define JOGO_H

#include "tautologia.h"

typedef struct {
    const char *codigo;             
    const char *formula;           
    const char *variaveis_texto;    
    const char *frase_tautologica;  
    ResultadoFormula esperado;      

    const char *itens[16];
    int num_itens;
} Nivel;

void jogo_corte_da_verdade(void);

int esperar_enter_or_esc(void);

#endif 
