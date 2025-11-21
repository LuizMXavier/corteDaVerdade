#ifndef UI_H
#define UI_H

// Funções utilitárias de interface para o jogo Corte da Verdade.

void draw_centered(int y, const char *text);
void draw_horizontal_bar(int y, int width);
void draw_wrapped_text(int x, int y, int width, const char *text);

#endif // UI_H
