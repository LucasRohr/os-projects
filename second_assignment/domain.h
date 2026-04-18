// Limite máximo de instruções para array estático de leitura do log
#define MAX_INSTRUCTIONS 1000

// Estrutura para armazenar a instrução extraída do log (arquivo txt)
typedef struct {
    int process_id;
    int virtual_address;
} Instruction;

// Representa uma entrada na Tabela de Páginas (1024 KB / 8 KB = 128 páginas)
typedef struct {
    int frame_number; // Número do frame na memória física
    int valid_bit; // 1 se estiver na RAM, 0 se estiver no disco
    int access_time; // Último tempo de acesso para implementar algoritmo LRU de subtituição de páginas (em caso de page fault)
} PageTableEntry;

// Representa a Memória Física (64 KB / 8 KB = 8 frames)
typedef struct {
    int occupied; // 1 ocupado, 0 livre
    int process_id; // ID do processo leve que está usando o frame (se ocupado)
    int page_number; // Número da página que está armazenada no frame (se ocupado)
    char data[8192]; // 8KB de dados por frame
} Frame;