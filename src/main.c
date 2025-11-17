#include <stdio.h>
#include "screen.h"
#include "keyboard.h"
#include "jogo.h"

int main(void) {
    // Inicializa CLI-LIB
    keyboardInit();
    screenInit(1); // 1 = desenha bordas automaticamente

    // Roda o jogo
    jogo_corte_da_verdade();

    // Finaliza CLI-LIB
    screenDestroy();
    keyboardDestroy();

    return 0;
}
