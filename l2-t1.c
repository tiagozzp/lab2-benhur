#include <stdio.h>
#include <stdlib.h>

typedef struct {

} estado_t;

void joga_onda (estado_t *est)
{
    laco() {
        processar_teclado(est);
        processar_tempo(est);
        apresenta(est);
    }
}

void joga_partida (estado_t *est)
{
    laco() {
        joga_onda(est);
    }
}

int main ()
{
    estado_t estado;
    inicializa_tela();
    inicializa_estado(&estado);
    while (!estado.terminou) {
        joga_partida(&estado);
    }
    desinicializa_tela();
}
