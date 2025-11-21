#ifndef RANKING_H
#define RANKING_H

// Tamanho máximo do nome do réu no ranking
#define NOME_MAX 32

// Carrega ranking do arquivo (se existir)
void carregar_rank(void);

// Salva o ranking atual em arquivo
void salvar_rank(void);

// Atualiza o ranking com o nome e pontuação informados
void atualizar_rank(const char *nome, int pontos);

// Mostra a tela de ranking
void mostrar_rank(void);

#endif // RANKING_H
