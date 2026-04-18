#include "domain.h"

// Função para encontrar um frame livre na memória física
// Retorno:
// - Sucesso: Índice do frame livre encontrado
// - Falha: -1 (indica que não há frames livres)
int find_free_frame(Frame physical_memory[]) {
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (physical_memory[i].occupied == 0) {
            return i; // Retorna o índice do frame livre
        }
    }

    return -1; // Retorna -1 se não houver frames livres
}

// Função para implementar o algoritmo LRU e encontrar um frame para substituição
// Retorno:
// - Sucesso: Índice do frame escolhido para substituição
// - Falha: -1 (indica que não há frames ocupados, o que não deveria acontecer nesse cenário)
int find_lru_frame(Frame physical_memory[], PageTableEntry page_table[]) {
    int frame_to_replace = -1; // Inicializa com -1 para indicar que ainda não foi encontrado um frame
    int oldest_access_time = INT_MAX; // Inicializa com o maior valor possível

    for (int i = 0; i < NUM_FRAMES; i++) {
        // Verifica fram ocupado
        if (physical_memory[i].occupied == 1) {
            int page_number = physical_memory[i].page_number;
            int access_time = page_table[page_number].access_time;

            // Verifica se página do frame tem acesso mais antigo para aplicar o LRU
            if (access_time < oldest_access_time) {
                oldest_access_time = access_time;
                frame_to_replace = i; // Atualiza o frame escolhido para substituição
            }
        }
    }

    return frame_to_replace;
}
