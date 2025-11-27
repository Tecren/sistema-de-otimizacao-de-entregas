#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "dijkstra.h"

#define INF 1000000000

Caminho dijkstra(int n, int** matriz, int origem, int destino) {
    int* dist = (int*)malloc(n * sizeof(int));
    int* visitado = (int*)calloc(n, sizeof(int));
    int* anterior = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        anterior[i] = -1;
    }
    dist[origem] = 0;

    for (int k = 0; k < n; k++) {
        int u = -1;
        int menor = INF;
        for (int i = 0; i < n; i++) {
            if (!visitado[i] && dist[i] < menor) {
                menor = dist[i];
                u = i;
            }
        }
        if (u == -1) break;
        visitado[u] = 1;
        for (int v = 0; v < n; v++) {
            int peso = matriz[u][v];
            if (peso >= 0 && !visitado[v]) {
                if (dist[u] + peso < dist[v]) {
                    dist[v] = dist[u] + peso;
                    anterior[v] = u;
                }
            }
        }
    }

    Caminho c;
    c.caminho = NULL;
    c.tamanho = 0;
    c.distancia = dist[destino];

    if (dist[destino] >= INF) {
        free(dist);
        free(visitado);
        free(anterior);
        return c;
    }

    int capacidade = n;
    int* temp = (int*)malloc(capacidade * sizeof(int));
    int tam = 0;
    for (int v = destino; v != -1; v = anterior[v]) {
        temp[tam++] = v;
    }
    int* caminho = (int*)malloc(tam * sizeof(int));
    for (int i = 0; i < tam; i++) caminho[i] = temp[tam - 1 - i];
    free(temp);

    c.caminho = caminho;
    c.tamanho = tam;

    free(dist);
    free(visitado);
    free(anterior);
    return c;
}

void liberar_caminho(Caminho* c) {
    if (!c) return;
    if (c->caminho) free(c->caminho);
    c->caminho = NULL;
    c->tamanho = 0;
    c->distancia = 0;
}

void imprimir_caminho(Caminho* c, char nomes[][50], int origem, int destino) {
    if (!c || c->tamanho == 0) {
        printf("Caminho inexistente\n");
        return;
    }
    printf("  Partindo de %s...\n", nomes[origem]);
    printf("  Caminho ate %s: ", nomes[destino]);
    for (int i = 0; i < c->tamanho; i++) {
        if (i > 0) printf(" -> ");
        printf("%s", nomes[c->caminho[i]]);
    }
    printf("\n  Distancia: %d km\n", c->distancia);
}
