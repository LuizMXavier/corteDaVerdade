#include <stdio.h>
#include <locale.h>     
#include <stdlib.h>     
#include "screen.h"
#include "keyboard.h"
#include "jogo.h"

int main(void) {
    setlocale(LC_ALL, "");
    char *titulo_buffer = malloc(64 * sizeof(char));
    if (titulo_buffer != NULL) {
        snprintf(titulo_buffer, 64, "Corte da Verdade");
        free(titulo_buffer);
    }

    keyboardInit();
    screenInit(1);

    jogo_corte_da_verdade();
    
    screenDestroy();
    keyboardDestroy();

    return 0;
}
