#include <stdio.h>
#include <string.h>
#include "conveyor.h"

// Rótulos para exibição no terminal
static const char *QUALITY_LABELS[NUM_QUALITY_LEVELS] = {
    "Ruim", "Neutro", "Bom"
};

static const char *TYPE_LABELS[NUM_PRODUCT_TYPES] = {
    "Eletronico", "Alimenticio", "Fragil"
};

// Zera todas as filas e contadores da esteira
// Chamada uma única vez pelo processo pai antes dos forks
void conveyor_init(Conveyor *conveyor) {
    memset(conveyor, 0, sizeof(Conveyor));
    // memset já coloca head=0, tail=0, count=0 em todas as filas e zera todos os contadores de estatísticas
}

// Insere um produto na fila correspondente à sua qualidade
// Retorna 0 em sucesso, -1 se a fila estiver cheia
int conveyor_enqueue(Conveyor *conveyor, Product product) {
    Queue *queue = &conveyor->queues[product.quality];
    int is_full = (queue->count == QUEUE_CAPACITY);

    // Se a fila estiver cheia, retorna -1
    if (is_full) {
        return -1;
    }

    queue->items[queue->tail] = product; // Insere o produto na fila
    queue->tail  = (queue->tail + 1) % QUEUE_CAPACITY; // Avança o tail de forma circular
    queue->count++; // Incrementa a quantidade de produtos na fila

    return 0;
}

// Remove e devolve o próximo produto da fila da qualidade indicada
// Retorna 0 em sucesso, -1 se a fila estiver vazia
int conveyor_dequeue(Conveyor *conveyor, Quality quality, Product *out) {
    Queue *queue = &conveyor->queues[quality];
    int is_empty = (queue->count == 0);

    // Se a fila estiver vazia, retorna -1
    if (is_empty) {
        return -1;
    }

    *out = queue->items[queue->head]; // Devolve o próximo produto da fila
    queue->head  = (queue->head + 1) % QUEUE_CAPACITY; // Avança o head de forma circular
    queue->count--; // Decrementa a quantidade de produtos na fila

    return 0;
}

// Calcula a diferença em milissegundos entre dois instantes
double timespec_diff_ms(struct timespec start, struct timespec end) {
    double diff_sec  = (double)(end.tv_sec  - start.tv_sec);
    double diff_nsec = (double)(end.tv_nsec - start.tv_nsec);
    return diff_sec * 1000.0 + diff_nsec / 1.0e6;
}

// Imprime o relatório final da simulação
// Chamada pelo processo pai após todos os filhos terminarem
void conveyor_print_stats(const Conveyor *conveyor) {
    int total_produced = conveyor->total_produced;
    int total_consumed = conveyor->total_consumed;

    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║         ESTATÍSTICAS DA SIMULAÇÃO        ║\n");
    printf("╠══════════════════════════════════════════╣\n");

    printf("║  Total produzido:  %4d                  ║\n", total_produced);
    printf("║  Total consumido:  %4d                  ║\n", total_consumed);

    // Produtos ainda na esteira ao fim da simulação
    int restantes = total_produced - total_consumed;
    printf("║  Na esteira (nao consumidos): %4d        ║\n", restantes);

    printf("╠══════════════════════════════════════════╣\n");
    printf("║  Por qualidade:                          ║\n");

    for (int i = 0; i < NUM_QUALITY_LEVELS; i++) {
        int produced_by_quality = conveyor->produced_by_quality[i];
        int consumed_by_quality = conveyor->consumed_by_quality[i];
        double percentage = (total_produced > 0) ? ((double)produced_by_quality * 100.0 / total_produced) : 0.0;

        printf("║    %-8s  produzidos: %3d  consumidos: %3d  (%5.1f%%)  ║\n",
               QUALITY_LABELS[i], produced_by_quality, consumed_by_quality, percentage);
    }

    printf("╠══════════════════════════════════════════╣\n");
    printf("║  Por tipo de produto:                    ║\n");

    for (int i = 0; i < NUM_PRODUCT_TYPES; i++) {
        int produced_by_type = conveyor->produced_by_type[i];
        int consumed_by_type = conveyor->consumed_by_type[i];
        double percentage = (total_produced > 0) ? ((double)produced_by_type * 100.0 / total_produced) : 0.0;

        printf("║    %-12s  produzidos: %3d  consumidos: %3d  (%5.1f%%)  ║\n",
               TYPE_LABELS[i], produced_by_type, consumed_by_type, percentage);
    }

    printf("╠══════════════════════════════════════════╣\n");

    // Tempo médio de permanência na esteira
    double avg_time = (total_consumed > 0)
                      ? (conveyor->total_time_in_conveyor_ms / total_consumed)
                      : 0.0;
                      
    printf("║  Tempo medio na esteira: %8.2f ms      ║\n", avg_time);
    printf("║  Tempo total acumulado:  %8.2f ms      ║\n",
           conveyor->total_time_in_conveyor_ms);

    printf("╚══════════════════════════════════════════╝\n\n");
}
