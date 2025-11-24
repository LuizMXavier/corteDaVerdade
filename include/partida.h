#ifndef PARTIDA_H
#define PARTIDA_H

#include <stddef.h>

// Pede o nome do réu. 
// Retorna 1 = ok, 0 = cancelado (ESC).
int pedir_nome_reu(char *nome, size_t tam);

// Executa uma partida completa (todos os níveis + fases bônus).
// Retorna a pontuação acumulada, mesmo se o jogador apertar ESC no meio.
int executar_partida(const char *nome_reu);

#endif // PARTIDA_H
