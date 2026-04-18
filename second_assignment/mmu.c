#include "domain.h"

// Função para calcular o número da página a partir do endereço virtual
// Exemplo: 15000 / 8192 = 1 (Página 1)
int get_page_number(int virtual_address) {
    return virtual_address / PAGE_SIZE; // Divisão inteira para obter o número da página
}

// Função para calcular o deslocamento (offset) a partir do endereço virtual
// Exemplo: 15000 % 8192 = 6808 (Byte 6808 dentro da Página 1)
int get_offset(int virtual_address) {
    return virtual_address % PAGE_SIZE; // Resto da divisão para obter o offset dentro da página
}
