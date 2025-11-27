#ifndef DIJKSTRA_H
#define DIJKSTRA_H

typedef struct {
    int* caminho;
    int tamanho;
    int distancia;
} Caminho;

Caminho dijkstra(int n, int** matriz, int origem, int destino);
void liberar_caminho(Caminho* c);
void imprimir_caminho(Caminho* c, char nomes[][50], int origem, int destino);

#endif
