#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <time.h>
#include <semaphore.h>

#define NUM_PRODUCERS       3   // número de produtores
#define NUM_CONSUMERS       3   // um consumidor por nível de qualidade
#define QUEUE_CAPACITY      50  // capacidade de cada fila de qualidade
#define NUM_QUALITY_LEVELS  3   // número de níveis de qualidade
#define NUM_PRODUCT_TYPES   3   // número de tipos de produto
#define GENERATED_PRODUCTS  10  // número de produtos gerados por produtor

// Nomes dos objetos de memória compartilhada e semáforos
#define SHM_NAME        "/conveyor_shm" // Shared memory
#define SEM_MUTEX_NAME  "/conveyor_mutex"
#define SEM_EMPTY_BAD   "/conveyor_empty_bad"
#define SEM_EMPTY_MED   "/conveyor_empty_med"
#define SEM_EMPTY_GOOD  "/conveyor_empty_good"
#define SEM_FULL_BAD    "/conveyor_full_bad"
#define SEM_FULL_MED    "/conveyor_full_med"
#define SEM_FULL_GOOD   "/conveyor_full_good"

// Enums

// Nível de qualidade do produto
typedef enum {
    QUALITY_BAD    = 0,  /* ruim   */
    QUALITY_MEDIUM = 1,  /* neutro */
    QUALITY_GOOD   = 2   /* bom    */
} Quality;

// Categoria do produto
typedef enum {
    TYPE_ELECTRONIC  = 0,  /* eletrônico  */
    TYPE_FOOD        = 1,  /* alimentício */
    TYPE_FRAGILE     = 2   /* frágil      */
} ProductType;

// Struct do produto

typedef struct {
    int         id;
    char        name[32];
    ProductType type;
    Quality     quality;
    int         producer_id;           // ID do processo produtor que gerou o produto
    double      production_time_ms;    // tempo que o produtor esperou antes de inserir na esteira (ms)
    struct timespec timestamp_in;      // momento de entrada na esteira
    struct timespec timestamp_out;     // momento de saída da esteira  
} Product;

// Fila circular para uma única qualidade de produto

typedef struct {
    Product items[QUEUE_CAPACITY];
    int     head;   // índice do próximo item a ser consumido
    int     tail;   // índice do próximo slot livre          
    int     count;  // quantidade de itens atualmente na fila
} Queue;

// Esteira — memória compartilhada principal
// Contém uma fila por nível de qualidade,
// além dos contadores de estatísticas da esteira

typedef struct {
    // Três filas independentes: [RUIM, NEUTRO, BOM]
    Queue queues[NUM_QUALITY_LEVELS];

    // Estatísticas globais
    int    total_produced;                          // total de produtos inseridos na esteira    
    int    total_consumed;                          // total de produtos retirados da esteira    
    int    produced_by_quality[NUM_QUALITY_LEVELS]; // contagem por qualidade (produzidos)       
    int    consumed_by_quality[NUM_QUALITY_LEVELS]; // contagem por qualidade (consumidos)       
    int    produced_by_type[NUM_QUALITY_LEVELS];    // contagem por tipo de produto (produzidos) 
    int    consumed_by_type[NUM_QUALITY_LEVELS];    // contagem por tipo de produto (consumidos) 
    double total_time_in_conveyor_ms;               // soma dos tempos de permanência na esteira 
    int    simulation_done;                         // flag: 1 quando todos os produtores finalizaram
} Conveyor; // Estrutura principal da esteira

// Protótipos de funções da esteira

// Inicializa todas as filas da esteira (head, tail, count = 0) e zera todos os contadores de estatísticas
// Deve ser chamada uma única vez pelo processo pai antes de fazer fork
void conveyor_init(Conveyor *conveyor);

// Insere um produto na fila correspondente à sua qualidade.
// Retorna 0 em sucesso, -1 se a fila estiver cheia.
int conveyor_enqueue(Conveyor *conveyor, Product product);

// Remove e retorna o próximo produto da fila de qualidade indicada.
// Retorna 0 em sucesso, -1 se a fila estiver vazia.
int conveyor_dequeue(Conveyor *conveyor, Quality quality, Product *out);

// Calcula a diferença em milissegundos entre dois timespec.
// Utilizado para medir o tempo que um produto ficou na esteira.
double timespec_diff_ms(struct timespec start, struct timespec end);

// Imprime no terminal as estatísticas finais da simulação.
// Deve ser chamada pelo processo pai após todos os filhos terminarem.
void conveyor_print_stats(const Conveyor *conveyor);

#endif // CONVEYOR_H
