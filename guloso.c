#include <stdlib.h>
#include "guloso.h"

static int cmp(const void* a, const void* b) {
    const Pacote* pa = (const Pacote*)a;
    const Pacote* pb = (const Pacote*)b;
    if (pa->prazo != pb->prazo) return pa->prazo - pb->prazo;
    if (pa->valor != pb->valor) return pb->valor - pa->valor;
    return pa->id - pb->id;
}

void ordenar_por_prazo(Pacote* itens, int quantidade) {
    if (!itens || quantidade <= 1) return;
    qsort(itens, quantidade, sizeof(Pacote), cmp);
}
