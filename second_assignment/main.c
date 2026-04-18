#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "domain.h"

int main() {
    // == 1. Inicialização Estruturas ==
    PageTableEntry page_table[NUM_PAGES]; // Tabela de Páginas para 128 páginas
    Frame physical_memory[NUM_FRAMES]; // Memória Física com 8 frames
    int global_time = 0; // Relógio lógico para o algoritmo LRU

    // == 2. Inicialização das estruturas ==
    for (int i = 0; i < NUM_PAGES; i++) {
        page_table[i].frame_number = -1; // -1 indica que a página não está na memória física
        page_table[i].valid_bit = 0; // 0 indica que a página não está na memória principal
        page_table[i].access_time = 0; // 0 indica que não houve acesso ainda
    }

    for (int i = 0; i < NUM_FRAMES; i++) {
        physical_memory[i].occupied = 0; // Frame livre
        physical_memory[i].process_id = -1; // Nenhum processo usando o frame
        physical_memory[i].page_number = -1; // Nenhuma página armazenada
        memset(physical_memory[i].data, 0, sizeof(physical_memory[i].data)); // Inicializa os dados como string vazia
    }

    // == 3. Leitura do arquivo de instruções ==

    // Declaração do array de instruções e contador
    Instruction instructions[MAX_INSTRUCTIONS];
    int total_instructions = 0;

    // Abertura do arquivo de log
    FILE *file = fopen("instructions.txt", "r");

    if (file == NULL) {
        printf("Erro: Não foi possível abrir o arquivo instructions.txt\n");
        return 1; // Encerra o programa com erro
    }

    // Leitura do arquivo e salvamento no array
    // O fscanf tenta ler dois inteiros por linha até o final do arquivo (EOF)
    while (fscanf(file, "%d %d", 
                  &instructions[total_instructions].process_id, 
                  &instructions[total_instructions].virtual_address) != EOF) {
        
        total_instructions++;
        
        // Verifica limite do array para evitar overflow
        if (total_instructions >= MAX_INSTRUCTIONS) {
            printf("Aviso: Limite maximo de instrucoes atingido.\n");
            break;
        }
    }

    // Fecha o arquivo após a leitura
    fclose(file);

    printf("Sucesso! %d instrucoes carregadas na memoria.\n", total_instructions);
    printf("===================================================\n");

    // 4. Iteração sobre o array (Core do Simulador)
    for (int i = 0; i < total_instructions; i++) {
        int current_pid = instructions[i].process_id;
        int current_virtual_address = instructions[i].virtual_address;

        global_time++; // Incrementa o relógio lógico a cada instrução processada

        printf("\n[NOVA INSTRUCAO] Processo: %d | Endereco Virtual: %d\n", current_pid, current_virtual_address);
        
        // ==========================================================
        // Lógica da MMU para cada instrução:
        
        // 1. Calcular o número da página
        int page_number = get_page_number(current_virtual_address);

        // Verificação de segurança
        if (page_number >= NUM_PAGES) {
            printf("[ERRO] Endereco %d fora dos limites da memoria virtual (1MB)\n", current_virtual_address);
            continue; // Pula para a próxima instrução
        }

        // 2. Calcular o deslocamento/offset
        int offset = get_offset(current_virtual_address);

        // 3. Consultar a Tabela de Páginas do processo atual para verificar se a página está na memória física
        // 4. Tratar Page Fault ou Hit
        printf("[MMU] Traduzindo: Pagina %d, Deslocamento %d\n", page_number, offset);

        // Verificar se a página está na memória física (valid_bit == 1)
        if (page_table[page_number].valid_bit == 1) {
            // == Page Hit ==
            int frame_number = page_table[page_number].frame_number;
            int physical_address = frame_number * PAGE_SIZE + offset; // Cálculo do endereço físico

            // Atualiza o tempo de acesso para o LRU
            page_table[page_number].access_time = global_time;

            printf("[MMU] SUCESSO (Hit): Pagina %d ja esta no Frame %d.\n", page_number, frame_number);
            printf("[MMU] Endereco Fisico gerado: %d\n", physical_address);
            printf("[OUT] Apresentando conteudo do Frame %d...\n", frame_number);
            printf("[OUT] Conteudo do Frame %d: %s\n", frame_number, physical_memory[frame_number].data);
        } else {
            // == Page Fault ==
            printf("[MMU] ALERTA: Falta de Pagina (Page Fault) para a Pagina %d!\n", page_number);

            // Primeiro, tenta encontrar um frame livre na memória física
            int free_frame_index = find_free_frame(physical_memory);

            // Cenário 1: Frame livre encontrado
            if (free_frame_index != -1) {
                printf("[MMU] Encontrado Frame livre: Frame %d. Carregando pagina %d...\n", free_frame_index, page_number);
                // Atualiza a Tabela de Páginas para refletir a nova página carregada
                page_table[page_number].frame_number = free_frame_index;
                page_table[page_number].valid_bit = 1; // Página agora está na memória
                page_table[page_number].access_time = global_time; // Atualiza o tempo de acesso

                // Atualiza a Memória Física para refletir o novo frame ocupado
                physical_memory[free_frame_index].occupied = 1;
                physical_memory[free_frame_index].process_id = current_pid;
                physical_memory[free_frame_index].page_number = page_number;

                // Escrevendo no frame dados simulando o que veio do disco
                sprintf(physical_memory[free_frame_index].data, "Bloco de Dados [Proc %d | Pag %d]", current_pid, page_number);

                printf("[OUT] Conteudo carregado no Frame %d: %s\n", free_frame_index, physical_memory[free_frame_index].data);
            } else {
                // Cenário 2: Nenhum frame livre, precisa aplicar o algoritmo LRU para substituição de página
                printf("[MMU] Nenhum Frame livre encontrado. Aplicando algoritmo LRU para substituição de pagina...\n");

                // Busca frame para substituição usando o algoritmo LRU
                int frame_to_replace = find_lru_frame(physical_memory, page_table);

                // Se houver um frame para substituir, realiza a substituição
                if (frame_to_replace != -1) {
                    // Página que será substituída (vítima do LRU)
                    int victim_page = physical_memory[frame_to_replace].page_number;

                    printf("[MMU] Substituindo pagina (vítima) %d do Frame %d (Processo %d) por pagina %d do Processo %d.\n", 
                           victim_page, frame_to_replace, physical_memory[frame_to_replace].process_id, 
                           page_number, current_pid);

                    // Atualiza a Tabela de Páginas para a página que está sendo substituída
                    page_table[victim_page].valid_bit = 0; // Página sai da memória
                    page_table[victim_page].frame_number = -1; // Remove referência ao frame

                    // Carrega a nova página no frame escolhido para substituição
                    page_table[page_number].frame_number = frame_to_replace;
                    page_table[page_number].valid_bit = 1; // Página agora está na memória
                    page_table[page_number].access_time = global_time; // Atualiza o tempo de acesso

                    // Atualiza a Memória Física para refletir o novo frame ocupado
                    physical_memory[frame_to_replace].occupied = 1;
                    physical_memory[frame_to_replace].process_id = current_pid; // Atualiza o processo que está usando o frame
                    physical_memory[frame_to_replace].page_number = page_number; // Atualiza a página que está armazenada no frame

                    // Sobrescrevendo o frame
                    sprintf(physical_memory[frame_to_replace].data, "Bloco de Dados [Proc %d | Pag %d]", current_pid, page_number);

                    printf("[OUT] Conteudo atualizado no Frame %d: %s\n", frame_to_replace, physical_memory[frame_to_replace].data);
                } else {
                    printf("[ERRO] Impossivel encontrar um frame para substituição. Isso não deveria acontecer nesse cenário.\n");
                }
            }
        }

        // ==========================================================
    }

    return 0;
}
