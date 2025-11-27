#ifndef KNAPSACK_H
#define KNAPSACK_H

typedef struct {
    int id;
    int peso;
    int valor;
    int prazo;
    int vertice;
} Pacote;

typedef struct {
    Pacote* itens;
    int quantidade;
    int peso_total;
    int valor_total;
} Selecionados;

Selecionados selecionar_pacotes(Pacote* pacotes, int n, int capacidade);
void liberar_selecionados(Selecionados* s);

#endif
