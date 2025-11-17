#include <stdio.h>
#include <locale.h>     // <- para acentuação / locale
#include "screen.h"
#include "keyboard.h"
#include "jogo.h"

int main(void) {
    // Diz pro programa usar a locale padrão do sistema (pt_BR.UTF-8, en_US.UTF-8, etc.)
    setlocale(LC_ALL, "");

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
