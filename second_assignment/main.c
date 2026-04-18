#include <stdio.h>
#include <stdlib.h>

#include "domain.h"

int main() {
    // 1. Declaração do array de instruções e contador
    Instruction instructions[MAX_INSTRUCTIONS];
    int total_instructions = 0;

    // 2. Abertura do arquivo de log
    FILE *file = fopen("instructions.txt", "r");

    if (file == NULL) {
        printf("Erro: Não foi possível abrir o arquivo instructions.txt\n");
        return 1; // Encerra o programa com erro
    }

    // 3. Leitura do arquivo e salvamento no array
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

        printf("\n[NOVA INSTRUCAO] Processo: %d | Endereco Virtual: %d\n", current_pid, current_virtual_address);
        
        // ==========================================================
        // AQUI ENTRA A LÓGICA DA SUA MMU!
        // 
        // 1. Calcular o número da página (end_virtual_atual / 8192)
        // 2. Calcular o deslocamento/offset (end_virtual_atual % 8192)
        // 3. Consultar a Tabela de Páginas do 'pid_atual'
        // 4. Tratar Page Fault ou Hit
        // 5. Atualizar o contador de tempo do LRU
        // ==========================================================
    }

    return 0;
}
