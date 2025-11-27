# Sistema de Otimização de Entregas

Programa em C de um otimizador de entregas usando Programação Dinâmica, Algoritmo Guloso e Dijkstra.

Sistema desenvolvido para a diciplina de Projeto e Otimização de Algoritmos - UNESC.

Aluno Guilherme C. Machado
Professor Allan F. Favaro

---

### Funcionalidades

1. **Executar exemplo pronto** - Demonstração completa com dados pré-configurados.
2. **Escolher entre mapas pré-definidos** - Quatro mapas (grafos) prontos.
3. **Inserir grafo manualmente** - Criação customizada de grafo por vertices e arestas.
4. **Sair** - Encerrar o programa

---

### Arquivos

- `main.c` - Interface, menu, entrada/saída, integração.
- `knapsack.c/h` - Algoritmo Knapsack (PD).
- `guloso.c/h` - Algoritmo guloso.
- `dijkstra.c/h` - Algoritmo de Dijkstra.

---

### Compilação e Execução

```bash
gcc main.c knapsack.c guloso.c dijkstra.c -o entregas

./entregas
```

---