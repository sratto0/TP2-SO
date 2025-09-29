#include "memoryManager.h"

typedef struct block_t{
    size_t size;       
    int is_free;       
    struct block_t * next; 
} block_t;

typedef struct MemoryManagerCDT{ 
    void * heap_start;
    uint64_t total_size;
    uint64_t used_size;
    block_t * first_block;
} MemoryManagerCDT;

MemoryManagerCDT memory_manager;
memory_info_t memory_info;

void memory_init(void * start, uint64_t size){
    memory_manager.heap_start = start;
    memory_manager.total_size = size;
    memory_manager.used_size = 0;
    
    memory_manager.first_block = (block_t *)start;
    memory_manager.first_block->size = size - sizeof(block_t); 
    memory_manager.first_block->is_free = 1;                    
    memory_manager.first_block->next = NULL;   

    memory_info.size = size;
    memory_info.used = 0;
    memory_info.free = size;
}

void * memory_alloc(size_t size){
    if (size == 0) return NULL;
    
    // Alinear tamaño
    size = (size + 7) & ~7;
    
    printf("Buscando %zu bytes\n", size);
    printf("Primer bloque: %p, size: %zu, is_free: %d\n", 
          memory_manager.first_block, 
           memory_manager.first_block->size, 
           memory_manager.first_block->is_free);
    
    block_t *current = (block_t *)memory_manager.first_block;
    block_t *best_fit = NULL;
    size_t best_size = (size_t)-1;  // El más grande posible
    
    // Primera pasada: encontrar el mejor bloque (el más pequeño que sirva)
    while (current != NULL) {
         printf("Revisando bloque: %p, size: %zu, is_free: %d\n", 
               current, current->size, current->is_free);
        if (current->is_free && current->size >= size) {
            printf("Bloque válido encontrado!\n");
            if (current->size < best_size) {
                best_fit = current;
                best_size = current->size;
            }
        }
        current = current->next;
    }
    
    // Si no encontramos ningún bloque
    if (best_fit == NULL) {
         printf("No se encontró bloque válido\n");
        return NULL;
    }

    printf("Usando bloque: %p con size: %zu\n", best_fit, best_fit->size);
    
    // Dividir el bloque si es necesario
    if (best_fit->size > size + sizeof(block_t) + 16) {  // Mínimo 16 bytes sobrantes
        block_t *remainder = (block_t *)((uint8_t *)best_fit + sizeof(block_t) + size);
        remainder->size = best_fit->size - size - sizeof(block_t);
        remainder->is_free = 1;
        remainder->next = best_fit->next;
        
        best_fit->size = size;
        best_fit->next = remainder;
    }
    
    // Marcar como usado
    best_fit->is_free = 0;
    memory_manager.used_size += best_fit->size + sizeof(block_t);
    
    // Actualizar estadísticas
    memory_info.used = memory_manager.used_size;
    memory_info.free = memory_manager.total_size - memory_manager.used_size;
    
    return (void *)((uint8_t *)best_fit + sizeof(block_t));
}


void memory_free(void* ptr){
    if (ptr == NULL) return;
    
    // Obtener el bloque desde el puntero (retroceder al header)
    block_t *block_to_free = (block_t *)((uint8_t *)ptr - sizeof(block_t));
    
    // Validar que el puntero esté dentro del heap
    if ((uint8_t *)block_to_free < (uint8_t *)memory_manager.heap_start || 
        (uint8_t *)block_to_free >= (uint8_t *)memory_manager.heap_start + memory_manager.total_size) {
        return; // Puntero inválido
    }
    
    // Validar que el bloque no esté ya libre
    if (block_to_free->is_free) {
        return; // Ya está libre
    }
    
    // Marcar como libre
    block_to_free->is_free = 1;
    memory_manager.used_size -= block_to_free->size + sizeof(block_t);
    
    // Fusionar con bloques libres adyacentes
    block_t *current = (block_t *)memory_manager.first_block;
    
    while (current != NULL && current->next != NULL) {
        // Si el bloque actual y el siguiente están libres y son contiguos
        if (current->is_free && current->next->is_free) {
            uint8_t *current_end = (uint8_t *)current + sizeof(block_t) + current->size;
            if (current_end == (uint8_t *)current->next) {
                // Fusionar: expandir el bloque actual
                current->size += sizeof(block_t) + current->next->size;
                current->next = current->next->next;
                continue; // Revisar otra vez desde el mismo bloque
            }
        }
        current = current->next;
    }
    
    // Actualizar estadísticas
    memory_info.used = memory_manager.used_size;
    memory_info.free = memory_manager.total_size - memory_manager.used_size;
}

memory_info_t memory_get_info(){
    memory_info.size = memory_manager.total_size;
    memory_info.used = memory_manager.used_size;
    memory_info.free = memory_manager.total_size - memory_manager.used_size;
    return memory_info;
}