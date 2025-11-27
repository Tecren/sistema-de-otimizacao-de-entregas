#include <stdio.h>
#include <stdlib.h>
#include "knapsack.h"

static int max(int a, int b) { return a > b ? a : b; }

Selecionados selecionar_pacotes(Pacote* pacotes, int n, int capacidade) {
    int i, w;
    int** dp = (int**)malloc((n + 1) * sizeof(int*));
    for (i = 0; i <= n; i++) dp[i] = (int*)calloc((capacidade + 1), sizeof(int));

    for (i = 1; i <= n; i++) {
        for (w = 1; w <= capacidade; w++) {
            if (pacotes[i - 1].peso <= w) {
                int pega = pacotes[i - 1].valor + dp[i - 1][w - pacotes[i - 1].peso];
                int deixa = dp[i - 1][w];
                dp[i][w] = max(pega, deixa);
            } else {
                dp[i][w] = dp[i - 1][w];
            }
        }
    }

    int valor_total = dp[n][capacidade];
    int peso_total = 0;
    int capacidade_restante = capacidade;
    int* escolhido = (int*)calloc(n, sizeof(int));
    int qtd = 0;

    for (i = n; i >= 1; i--) {
        if (dp[i][capacidade_restante] != dp[i - 1][capacidade_restante]) {
            escolhido[i - 1] = 1;
            capacidade_restante -= pacotes[i - 1].peso;
            peso_total += pacotes[i - 1].peso;
            qtd++;
        }
    }

    Pacote* itens = NULL;
    if (qtd > 0) itens = (Pacote*)malloc(qtd * sizeof(Pacote));
    int idx = 0;
    for (i = 0; i < n; i++) {
        if (escolhido[i]) itens[idx++] = pacotes[i];
    }

    for (i = 0; i <= n; i++) free(dp[i]);
    free(dp);
    free(escolhido);

    Selecionados s;
    s.itens = itens;
    s.quantidade = qtd;
    s.peso_total = peso_total;
    s.valor_total = valor_total;
    return s;
}

void liberar_selecionados(Selecionados* s) {
    if (!s) return;
    if (s->itens) free(s->itens);
    s->itens = NULL;
    s->quantidade = 0;
    s->peso_total = 0;
    s->valor_total = 0;
}
