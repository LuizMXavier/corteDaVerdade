#include <stdio.h>
#include <string.h>

#include "screen.h"
#include "ui.h"

// ---------------------------------------------------------
// Utilidades de desenho / layout
// (copiadas de jogo.c original, sem alteração de lógica)
// ---------------------------------------------------------

void draw_centered(int y, const char *text) {
    int len = (int)strlen(text);
    int x = (MAXX - len) / 2;
    if (x < MINX) x = MINX;
    screenGotoxy(x, y);
    printf("%s", text);
}

void draw_horizontal_bar(int y, int width) {
    screenGotoxy(SCRSTARTX + 2, y);
    for (int i = 0; i < width; i++) {
        putchar('-');
    }
}

void draw_wrapped_text(int x, int y, int width, const char *text) {
    int col = 0;
    screenGotoxy(x, y);
    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        if (c == '\n') {
            y++;
            col = 0;
            screenGotoxy(x, y);
            continue;
        }
        if (col >= width) {
            y++;
            col = 0;
            screenGotoxy(x, y);
            continue;
        }
        putchar(c);
        col++;
    }
}
