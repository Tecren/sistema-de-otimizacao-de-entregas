#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include "knapsack.h"
#include "guloso.h"
#include "dijkstra.h"

#define INF 1000000000
#define MAX_VERTICES 50

#define COR_RESET "\033[0m"
#define COR_VERMELHO "\033[31m"
#define COR_VERDE "\033[32m"
#define COR_AZUL "\033[34m"
#define COR_CIANO "\033[36m"
#define COR_AMARELO "\033[33m"
#define COR_NEGRITO "\033[1m"

static int** alocar_matriz(int n) {
    int** m = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        m[i] = (int*)malloc(n * sizeof(int));
        for (int j = 0; j < n; j++) m[i][j] = -1;
        m[i][i] = 0;
    }
    return m;
}

static void liberar_matriz(int** m, int n) {
    if (!m) return;
    for (int i = 0; i < n; i++) free(m[i]);
    free(m);
}

static void imprimir_grafo(int** grafo, int vertices, char nomes[][50]) {
    printf("\n%s%s=== GRAFO CARREGADO ===%s\n", COR_NEGRITO, COR_CIANO, COR_RESET);
    for (int i = 0; i < vertices; i++) {
        printf("%s%d (%s)%s: ", COR_AZUL, i, nomes[i], COR_RESET);
        int primeiro = 1;
        for (int j = 0; j < vertices; j++) {
            if (i != j && grafo[i][j] >= 0) {
                if (!primeiro) printf(", ");
                printf("(%d,%d)", j, grafo[i][j]);
                primeiro = 0;
            }
        }
        if (primeiro) printf("(sem conexoes)");
        printf("\n");
    }
}

static void imprimir_pacotes(Pacote* v, int n) {
    for (int i = 0; i < n; i++) {
        printf("  [%d] Peso: %d kg | Valor: R$ %d | Prazo: %d dias | Destino: vertice %d\n", 
               v[i].id, v[i].peso, v[i].valor, v[i].prazo, v[i].vertice);
    }
}

static int validar_inteiro_positivo(const char* mensagem, ...) {
    char buffer[256];
    va_list args;
    va_start(args, mensagem);
    vsnprintf(buffer, sizeof(buffer), mensagem, args);
    va_end(args);
    
    int valor;
    while (1) {
        printf("%s", buffer);
        int resultado = scanf("%d", &valor);
        
        if (resultado != 1) {
            printf("%sErro: entrada invalida. Digite apenas numeros inteiros.%s\n", COR_VERMELHO, COR_RESET);
            while (getchar() != '\n');
            continue;
        }
        
        if (valor <= 0) {
            printf("%sErro: o valor deve ser maior que zero (valor informado: %d).%s\n", COR_VERMELHO, valor, COR_RESET);
            continue;
        }
        
        return valor;
    }
}

static int validar_inteiro_nao_negativo(const char* mensagem) {
    int valor;
    while (1) {
        printf("%s", mensagem);
        int resultado = scanf("%d", &valor);
        
        if (resultado != 1) {
            printf("%sErro: entrada invalida. Digite apenas numeros inteiros.%s\n", COR_VERMELHO, COR_RESET);
            while (getchar() != '\n');
            continue;
        }
        
        if (valor < 0) {
            printf("%sErro: o valor nao pode ser negativo (valor informado: %d).%s\n", COR_VERMELHO, valor, COR_RESET);
            continue;
        }
        
        return valor;
    }
}

static int validar_vertice(const char* mensagem, int max_vertices) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), mensagem, max_vertices - 1);
    
    int valor;
    while (1) {
        printf("%s", buffer);
        int resultado = scanf("%d", &valor);
        
        if (resultado != 1) {
            printf("%sErro: entrada invalida. Digite apenas numeros inteiros.%s\n", COR_VERMELHO, COR_RESET);
            while (getchar() != '\n');
            continue;
        }
        
        if (valor < 0) {
            printf("%sErro: o vertice nao pode ser negativo (valor informado: %d).%s\n", COR_VERMELHO, valor, COR_RESET);
            continue;
        }
        
        if (valor >= max_vertices) {
            printf("%sErro: vertice fora do intervalo valido. Valores aceitos: 0 a %d (valor informado: %d).%s\n", 
                   COR_VERMELHO, max_vertices - 1, valor, COR_RESET);
            continue;
        }
        
        return valor;
    }
}

static void executar_fluxo(Pacote* pacotes, int n, int capacidade, int** grafo, int vertices, char nomes[][50]) {
    printf("\n%s%s=== RESUMO DOS PACOTES ===%s\n", COR_NEGRITO, COR_CIANO, COR_RESET);
    printf("Total de pacotes disponiveis: %d\n", n);
    imprimir_pacotes(pacotes, n);
    
    printf("\n%s%s=== FASE 1: SELECAO POR CAPACIDADE (KNAPSACK) ===%s\n", COR_NEGRITO, COR_AZUL, COR_RESET);
    printf("Capacidade do caminhao: %d kg\n", capacidade);
    
    Selecionados s = selecionar_pacotes(pacotes, n, capacidade);
    if (s.quantidade == 0) {
        printf("%sNenhum pacote selecionado.%s\n", COR_VERMELHO, COR_RESET);
        return;
    }
    
    printf("%sPacotes selecionados: %d%s\n", COR_VERDE, s.quantidade, COR_RESET);
    printf("Peso total: %d kg\n", s.peso_total);
    printf("Valor total: R$ %d\n", s.valor_total);
    imprimir_pacotes(s.itens, s.quantidade);

    printf("\n%s%s=== FASE 2: ORDENACAO POR PRAZO (ALGORITMO GULOSO) ===%s\n", COR_NEGRITO, COR_AZUL, COR_RESET);
    ordenar_por_prazo(s.itens, s.quantidade);
    printf("Ordem de entrega (prioridade: menor prazo):\n");
    imprimir_pacotes(s.itens, s.quantidade);

    printf("\n%s%s=== FASE 3: CALCULO DE ROTAS (DIJKSTRA) ===%s\n", COR_NEGRITO, COR_AZUL, COR_RESET);
    int atual = 0;
    int* rota = (int*)malloc(vertices * s.quantidade * sizeof(int));
    int tam_rota = 0;
    int distancia_total = 0;
    rota[tam_rota++] = atual;
    
    for (int i = 0; i < s.quantidade; i++) {
        int alvo = s.itens[i].vertice;
        printf("\nEntrega %d/%d - Pacote ID %d\n", i + 1, s.quantidade, s.itens[i].id);
        
        Caminho c = dijkstra(vertices, grafo, atual, alvo);
        if (c.tamanho == 0 || c.distancia >= INF) {
            printf("%sErro: Caminho inexistente entre %s e vertice %d.%s\n", 
                   COR_VERMELHO, nomes[atual], alvo, COR_RESET);
            liberar_caminho(&c);
            free(rota);
            liberar_selecionados(&s);
            return;
        }
        
        imprimir_caminho(&c, nomes, atual, alvo);
        
        for (int k = 1; k < c.tamanho; k++) {
            rota[tam_rota++] = c.caminho[k];
        }
        distancia_total += c.distancia;
        atual = alvo;
        liberar_caminho(&c);
    }
    
    printf("\n%s%s=== RESUMO FINAL ===%s\n", COR_NEGRITO, COR_VERDE, COR_RESET);
    printf("Caminho completo: ");
    for (int i = 0; i < tam_rota; i++) {
        if (i) printf(" -> ");
        printf("%s", nomes[rota[i]]);
    }
    printf("\nDistancia total percorrida: %s%d km%s\n", COR_NEGRITO, distancia_total, COR_RESET);
    printf("Peso total transportado: %d kg\n", s.peso_total);
    printf("Valor total entregue: R$ %d\n", s.valor_total);
    printf("Numero de entregas: %d\n", s.quantidade);

    free(rota);
    liberar_selecionados(&s);
}

static void carregar_mapa_1(int** grafo, int* vertices, char nomes[][50]) {
    *vertices = 8;
    strcpy(nomes[0], "Deposito");
    strcpy(nomes[1], "Centro");
    strcpy(nomes[2], "Bairro Norte");
    strcpy(nomes[3], "Bairro Sul");
    strcpy(nomes[4], "Zona Industrial");
    strcpy(nomes[5], "Shopping");
    strcpy(nomes[6], "Aeroporto");
    strcpy(nomes[7], "Universidade");
    
    grafo[0][1] = grafo[1][0] = 5;
    grafo[0][2] = grafo[2][0] = 3;
    grafo[1][2] = grafo[2][1] = 2;
    grafo[1][3] = grafo[3][1] = 7;
    grafo[2][3] = grafo[3][2] = 10;
    grafo[2][4] = grafo[4][2] = 12;
    grafo[3][4] = grafo[4][3] = 3;
    grafo[3][5] = grafo[5][3] = 8;
    grafo[4][5] = grafo[5][4] = 4;
    grafo[4][6] = grafo[6][4] = 6;
    grafo[5][6] = grafo[6][5] = 5;
    grafo[5][7] = grafo[7][5] = 9;
    grafo[6][7] = grafo[7][6] = 7;
}

static void carregar_mapa_2(int** grafo, int* vertices, char nomes[][50]) {
    *vertices = 10;
    strcpy(nomes[0], "Deposito Central");
    strcpy(nomes[1], "Praca Principal");
    strcpy(nomes[2], "Zona Oeste");
    strcpy(nomes[3], "Zona Leste");
    strcpy(nomes[4], "Parque Central");
    strcpy(nomes[5], "Terminal Rodoviario");
    strcpy(nomes[6], "Hospital");
    strcpy(nomes[7], "Escola");
    strcpy(nomes[8], "Mercado Municipal");
    strcpy(nomes[9], "Estadio");
    
    grafo[0][1] = grafo[1][0] = 4;
    grafo[0][2] = grafo[2][0] = 6;
    grafo[1][3] = grafo[3][1] = 5;
    grafo[1][4] = grafo[4][1] = 3;
    grafo[2][4] = grafo[4][2] = 8;
    grafo[2][5] = grafo[5][2] = 7;
    grafo[3][6] = grafo[6][3] = 9;
    grafo[3][7] = grafo[7][3] = 4;
    grafo[4][6] = grafo[6][4] = 6;
    grafo[4][8] = grafo[8][4] = 5;
    grafo[5][8] = grafo[8][5] = 10;
    grafo[6][9] = grafo[9][6] = 3;
    grafo[7][9] = grafo[9][7] = 8;
    grafo[8][9] = grafo[9][8] = 4;
    grafo[1][2] = grafo[2][1] = 7;
}

static void carregar_mapa_3(int** grafo, int* vertices, char nomes[][50]) {
    *vertices = 15;
    strcpy(nomes[0], "Centro de Distribuicao");
    strcpy(nomes[1], "Centro Historico");
    strcpy(nomes[2], "Bairro Comercial");
    strcpy(nomes[3], "Zona Industrial");
    strcpy(nomes[4], "Bairro Residencial A");
    strcpy(nomes[5], "Bairro Residencial B");
    strcpy(nomes[6], "Shopping Center");
    strcpy(nomes[7], "Universidade");
    strcpy(nomes[8], "Hospital Regional");
    strcpy(nomes[9], "Aeroporto");
    strcpy(nomes[10], "Estacao de Trem");
    strcpy(nomes[11], "Parque Municipal");
    strcpy(nomes[12], "Zona Portuaria");
    strcpy(nomes[13], "Estadio Municipal");
    strcpy(nomes[14], "Terminal de Onibus");
    
    grafo[0][1] = grafo[1][0] = 8;
    grafo[0][2] = grafo[2][0] = 5;
    grafo[0][3] = grafo[3][0] = 12;
    grafo[1][2] = grafo[2][1] = 4;
    grafo[1][4] = grafo[4][1] = 7;
    grafo[1][10] = grafo[10][1] = 6;
    grafo[2][5] = grafo[5][2] = 9;
    grafo[2][6] = grafo[6][2] = 3;
    grafo[3][7] = grafo[7][3] = 10;
    grafo[3][12] = grafo[12][3] = 15;
    grafo[4][5] = grafo[5][4] = 5;
    grafo[4][11] = grafo[11][4] = 8;
    grafo[5][6] = grafo[6][5] = 6;
    grafo[5][8] = grafo[8][5] = 11;
    grafo[6][8] = grafo[8][6] = 7;
    grafo[6][14] = grafo[14][6] = 4;
    grafo[7][9] = grafo[9][7] = 18;
    grafo[7][13] = grafo[13][7] = 14;
    grafo[8][9] = grafo[9][8] = 20;
    grafo[8][14] = grafo[14][8] = 9;
    grafo[9][12] = grafo[12][9] = 25;
    grafo[10][11] = grafo[11][10] = 5;
    grafo[10][14] = grafo[14][10] = 7;
    grafo[11][13] = grafo[13][11] = 12;
    grafo[12][13] = grafo[13][12] = 16;
    grafo[13][14] = grafo[14][13] = 10;
}

static void carregar_mapa_4(int** grafo, int* vertices, char nomes[][50]) {
    *vertices = 20;
    strcpy(nomes[0], "CD Principal");
    strcpy(nomes[1], "Centro Financeiro");
    strcpy(nomes[2], "Avenida Paulista");
    strcpy(nomes[3], "Zona Sul");
    strcpy(nomes[4], "Zona Norte");
    strcpy(nomes[5], "Zona Leste");
    strcpy(nomes[6], "Zona Oeste");
    strcpy(nomes[7], "Alphaville");
    strcpy(nomes[8], "Brooklin");
    strcpy(nomes[9], "Morumbi");
    strcpy(nomes[10], "Vila Olimpia");
    strcpy(nomes[11], "Itaim Bibi");
    strcpy(nomes[12], "Pinheiros");
    strcpy(nomes[13], "Congonhas");
    strcpy(nomes[14], "Guarulhos");
    strcpy(nomes[15], "ABC Paulista");
    strcpy(nomes[16], "Osasco");
    strcpy(nomes[17], "Tatuape");
    strcpy(nomes[18], "Santana");
    strcpy(nomes[19], "Interlagos");
    
    grafo[0][1] = grafo[1][0] = 6;
    grafo[0][2] = grafo[2][0] = 8;
    grafo[0][4] = grafo[4][0] = 15;
    grafo[1][2] = grafo[2][1] = 3;
    grafo[1][10] = grafo[10][1] = 5;
    grafo[1][11] = grafo[11][1] = 4;
    grafo[2][3] = grafo[3][2] = 7;
    grafo[2][12] = grafo[12][2] = 6;
    grafo[3][8] = grafo[8][3] = 9;
    grafo[3][13] = grafo[13][3] = 12;
    grafo[3][19] = grafo[19][3] = 18;
    grafo[4][14] = grafo[14][4] = 22;
    grafo[4][17] = grafo[17][4] = 11;
    grafo[4][18] = grafo[18][4] = 8;
    grafo[5][14] = grafo[14][5] = 20;
    grafo[5][15] = grafo[15][5] = 14;
    grafo[5][17] = grafo[17][5] = 13;
    grafo[6][7] = grafo[7][6] = 19;
    grafo[6][12] = grafo[12][6] = 7;
    grafo[6][16] = grafo[16][6] = 10;
    grafo[7][9] = grafo[9][7] = 8;
    grafo[7][16] = grafo[16][7] = 12;
    grafo[8][10] = grafo[10][8] = 4;
    grafo[8][13] = grafo[13][8] = 6;
    grafo[9][10] = grafo[10][9] = 5;
    grafo[9][12] = grafo[12][9] = 9;
    grafo[10][11] = grafo[11][10] = 2;
    grafo[11][12] = grafo[12][11] = 3;
    grafo[13][15] = grafo[15][13] = 16;
    grafo[13][19] = grafo[19][13] = 11;
    grafo[14][18] = grafo[18][14] = 17;
    grafo[15][19] = grafo[19][15] = 20;
    grafo[16][18] = grafo[18][16] = 15;
    grafo[17][18] = grafo[18][17] = 7;
}

static void carregar_exemplo_completo() {
    printf("\n%s%s=== EXEMPLO PRONTO ===%s\n", COR_NEGRITO, COR_CIANO, COR_RESET);
    
    int n_pacotes = 6;
    Pacote* pacotes = (Pacote*)malloc(n_pacotes * sizeof(Pacote));
    pacotes[0] = (Pacote){1, 5, 150, 2, 3};
    pacotes[1] = (Pacote){2, 3, 120, 1, 5};
    pacotes[2] = (Pacote){3, 2, 80, 3, 4};
    pacotes[3] = (Pacote){4, 6, 200, 4, 6};
    pacotes[4] = (Pacote){5, 4, 100, 1, 2};
    pacotes[5] = (Pacote){6, 2, 90, 2, 7};
    int capacidade = 12;

    int v = 8;
    int** g = alocar_matriz(MAX_VERTICES);
    char nomes[MAX_VERTICES][50];
    carregar_mapa_1(g, &v, nomes);
    
    imprimir_grafo(g, v, nomes);
    executar_fluxo(pacotes, n_pacotes, capacidade, g, v, nomes);

    liberar_matriz(g, MAX_VERTICES);
    free(pacotes);
}

static void escolher_mapa_predefinido() {
    printf("\n%s%s=== MAPAS PRE-DEFINIDOS ===%s\n", COR_NEGRITO, COR_CIANO, COR_RESET);
    printf("1 - Mapa 1 (8 vertices - Cidade Pequena)\n");
    printf("2 - Mapa 2 (10 vertices - Cidade Media)\n");
    printf("3 - Mapa 3 (15 vertices - Cidade Grande)\n");
    printf("4 - Mapa 4 (20 vertices - Metropole)\n");
    printf("Escolha um mapa: ");
    
    int escolha;
    while (1) {
        if (scanf("%d", &escolha) != 1) {
            printf("%sErro: entrada invalida. Digite apenas numeros inteiros.%s\n", COR_VERMELHO, COR_RESET);
            while (getchar() != '\n');
            printf("Escolha um mapa (1-4): ");
            continue;
        }
        
        if (escolha < 1 || escolha > 4) {
            printf("%sErro: opcao invalida. Escolha entre 1 e 4 (valor informado: %d).%s\n", COR_VERMELHO, escolha, COR_RESET);
            printf("Escolha um mapa (1-4): ");
            continue;
        }
        
        break;
    }
    
    int v;
    int** g = alocar_matriz(MAX_VERTICES);
    char nomes[MAX_VERTICES][50];
    
    if (escolha == 1) {
        carregar_mapa_1(g, &v, nomes);
    } else if (escolha == 2) {
        carregar_mapa_2(g, &v, nomes);
    } else if (escolha == 3) {
        carregar_mapa_3(g, &v, nomes);
    } else {
        carregar_mapa_4(g, &v, nomes);
    }
    
    imprimir_grafo(g, v, nomes);

    int n_pacotes = validar_inteiro_positivo("\nQuantidade de pacotes: ");
    Pacote* pacotes = (Pacote*)malloc(n_pacotes * sizeof(Pacote));
    
    for (int i = 0; i < n_pacotes; i++) {
        pacotes[i].id = i + 1;
        printf("\n--- Pacote %d ---\n", i + 1);
        pacotes[i].peso = validar_inteiro_positivo("Peso (kg): ");
        pacotes[i].valor = validar_inteiro_positivo("Valor (R$): ");
        pacotes[i].prazo = validar_inteiro_positivo("Prazo (dias): ");
        pacotes[i].vertice = validar_vertice("Vertice de destino: ", v);
    }
    
    int capacidade = validar_inteiro_positivo("\nCapacidade do caminhao (kg): ");
    
    executar_fluxo(pacotes, n_pacotes, capacidade, g, v, nomes);
    
    liberar_matriz(g, MAX_VERTICES);
    free(pacotes);
}

static void inserir_grafo_manual() {
    printf("\n%s%s=== INSERCAO MANUAL DE GRAFO ===%s\n", COR_NEGRITO, COR_CIANO, COR_RESET);
    
    int v;
    while (1) {
        v = validar_inteiro_positivo("Numero de vertices (max %d): ", MAX_VERTICES);
        if (v <= MAX_VERTICES) break;
        printf("%sErro: numero maximo de vertices e %d (valor informado: %d).%s\n", 
               COR_VERMELHO, MAX_VERTICES, v, COR_RESET);
    }
    
    int** g = alocar_matriz(MAX_VERTICES);
    char nomes[MAX_VERTICES][50];
    
    printf("\n%s--- Nomes dos vertices ---%s\n", COR_CIANO, COR_RESET);
    printf("Deseja nomear os vertices? (s/n): ");
    getchar();
    char resposta;
    scanf("%c", &resposta);
    getchar();
    
    if (resposta == 's' || resposta == 'S') {
        printf("(Pressione Enter para usar nome padrao 'Vertice X')\n");
        for (int i = 0; i < v; i++) {
            printf("Nome do vertice %d: ", i);
            if (fgets(nomes[i], 50, stdin) != NULL) {
                size_t len = strlen(nomes[i]);
                if (len > 0 && nomes[i][len - 1] == '\n') {
                    nomes[i][len - 1] = '\0';
                }
                if (strlen(nomes[i]) == 0) {
                    sprintf(nomes[i], "Vertice %d", i);
                }
            } else {
                sprintf(nomes[i], "Vertice %d", i);
            }
        }
    } else {
        for (int i = 0; i < v; i++) {
            sprintf(nomes[i], "Vertice %d", i);
        }
    }
    
    printf("\n%s--- Vertices disponiveis ---%s\n", COR_CIANO, COR_RESET);
    for (int i = 0; i < v; i++) {
        printf("  %d - %s\n", i, nomes[i]);
    }
    
    printf("\n%s--- Insercao de Arestas ---%s\n", COR_CIANO, COR_RESET);
    printf("Digite as conexoes entre vertices.\n");
    printf("Para encerrar, digite -1 na origem.\n\n");
    
    int arestas_inseridas = 0;
    while (1) {
        printf("%sAresta %d:%s\n", COR_VERDE, arestas_inseridas + 1, COR_RESET);
        
        int origem;
        printf("  Origem (-1 para encerrar): ");
        int resultado = scanf("%d", &origem);
        
        if (resultado != 1) {
            printf("%sErro: entrada invalida. Digite apenas numeros inteiros.%s\n", COR_VERMELHO, COR_RESET);
            while (getchar() != '\n');
            continue;
        }
        
        if (origem == -1) {
            break;
        }
        
        if (origem < 0 || origem >= v) {
            printf("%sErro: vertice de origem invalido. Use valores entre 0 e %d.%s\n", 
                   COR_VERMELHO, v - 1, COR_RESET);
            continue;
        }
        
        int destino;
        printf("  Destino: ");
        resultado = scanf("%d", &destino);
        
        if (resultado != 1) {
            printf("%sErro: entrada invalida. Digite apenas numeros inteiros.%s\n", COR_VERMELHO, COR_RESET);
            while (getchar() != '\n');
            continue;
        }
        
        if (destino < 0 || destino >= v) {
            printf("%sErro: vertice de destino invalido. Use valores entre 0 e %d.%s\n", 
                   COR_VERMELHO, v - 1, COR_RESET);
            continue;
        }
        
        if (origem == destino) {
            printf("%sErro: origem e destino devem ser diferentes.%s\n", COR_VERMELHO, COR_RESET);
            continue;
        }
        
        if (g[origem][destino] >= 0 && g[origem][destino] != 0) {
            printf("%sAviso: ja existe uma aresta entre %s e %s com peso %d.%s\n", 
                   COR_AMARELO, nomes[origem], nomes[destino], g[origem][destino], COR_RESET);
            printf("Deseja sobrescrever? (s/n): ");
            char resposta_sobre;
            scanf(" %c", &resposta_sobre);
            if (resposta_sobre != 's' && resposta_sobre != 'S') {
                continue;
            }
        }
        
        int peso = validar_inteiro_positivo("  Peso (distancia em km): ");
        
        g[origem][destino] = peso;
        g[destino][origem] = peso;
        
        printf("%sAresta adicionada: %s <-> %s (peso: %d km)%s\n\n", 
               COR_VERDE, nomes[origem], nomes[destino], peso, COR_RESET);
        arestas_inseridas++;
    }
    
    if (arestas_inseridas == 0) {
        printf("\n%sAviso: Nenhuma aresta foi inserida. O grafo esta desconectado.%s\n", 
               COR_AMARELO, COR_RESET);
    }
    
    imprimir_grafo(g, v, nomes);
    
    printf("\n%s--- Pacotes para Entrega ---%s\n", COR_CIANO, COR_RESET);
    int n_pacotes = validar_inteiro_positivo("Quantidade de pacotes: ");
    Pacote* pacotes = (Pacote*)malloc(n_pacotes * sizeof(Pacote));
    
    for (int i = 0; i < n_pacotes; i++) {
        pacotes[i].id = i + 1;
        printf("\n%s--- Pacote %d ---%s\n", COR_VERDE, i + 1, COR_RESET);
        pacotes[i].peso = validar_inteiro_positivo("  Peso (kg): ");
        pacotes[i].valor = validar_inteiro_positivo("  Valor (R$): ");
        pacotes[i].prazo = validar_inteiro_positivo("  Prazo (dias): ");
        pacotes[i].vertice = validar_vertice("  Vertice de destino (0-%d): ", v);
    }
    
    int capacidade = validar_inteiro_positivo("\nCapacidade do caminhao (kg): ");
    
    executar_fluxo(pacotes, n_pacotes, capacidade, g, v, nomes);
    
    liberar_matriz(g, MAX_VERTICES);
    free(pacotes);
}

int main() {
    while (1) {
        printf("\n%s%s========================================%s\n", COR_NEGRITO, COR_AZUL, COR_RESET);
        printf("%s%s    OTIMIZACAO DE ENTREGAS URBANAS%s\n", COR_NEGRITO, COR_AZUL, COR_RESET);
        printf("%s%s========================================%s\n", COR_NEGRITO, COR_AZUL, COR_RESET);
        printf("1 - Executar exemplo pronto\n");
        printf("2 - Escolher entre mapas pre-definidos\n");
        printf("3 - Inserir grafo manualmente\n");
        printf("4 - Sair\n");
        printf("\nEscolha uma opcao: ");
        
        int op;
        int resultado = scanf("%d", &op);
        
        if (resultado != 1) {
            printf("%sErro: entrada invalida. Digite apenas numeros inteiros.%s\n", COR_VERMELHO, COR_RESET);
            while (getchar() != '\n');
            continue;
        }
        
        if (op < 1 || op > 4) {
            printf("%sErro: opcao invalida. Escolha entre 1 e 4 (valor informado: %d).%s\n", COR_VERMELHO, op, COR_RESET);
            continue;
        }
        
        switch (op) {
            case 1:
                carregar_exemplo_completo();
                break;
            case 2:
                escolher_mapa_predefinido();
                break;
            case 3:
                inserir_grafo_manual();
                break;
            case 4:
                printf("%sEncerrando programa.%s\n", COR_VERDE, COR_RESET);
                return 0;
        }
    }
}
