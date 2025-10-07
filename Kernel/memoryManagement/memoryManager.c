// #include "memoryManager.h"
// #define BLOCK_HEADER_SIZE sizeof(block_t)
// #define ALIGN4(x) (((((x) - 1) >> 2) << 2) + 4)

// typedef struct block_t{
//     uint64_t size;       
//     int is_free;       
//     struct block_t * next; 
// } block_t;

// typedef struct MemoryManagerCDT{ 
//     void * heap_start;
//     uint64_t total_size;
//     uint64_t used_size;
//     block_t * first_block;
// } MemoryManagerCDT;

// MemoryManagerCDT memory_manager;
// memory_info_t memory_info;

// void memory_init(void * start, uint64_t size){
//     memory_manager.heap_start = start;
//     memory_manager.total_size = size;
//     memory_manager.used_size = 0;
    
//     memory_manager.first_block = (block_t *)start;
//     memory_manager.first_block->size = size - sizeof(block_t); 
//     memory_manager.first_block->is_free = 1;                    
//     memory_manager.first_block->next = NULL;   

//     memory_info.size = size;
//     memory_info.used = 0;
//     memory_info.free = size;
// }

// // void * memory_alloc(uint64_t size){
// //     if (size == 0) return NULL;

// //     // Alinear tamaño a 8 bytes
// //     size = (size + 7) & ~7;

// //     block_t *current = memory_manager.first_block;
// //     uint8_t *heap_end = (uint8_t *)memory_manager.heap_start + memory_manager.total_size;

// //     // First-fit
// //     while (current != NULL) {
// //         if (current->is_free && current->size >= size) {

// //             // ¿Podemos dividir? Hace falta al menos un header + 8 bytes para el remanente
// //             if (current->size >= size + sizeof(block_t) + 8) {
// //                 block_t *remainder = (block_t *)((uint8_t *)current + sizeof(block_t) + size);

// //                 // Validar que el remainder COMPLETO entre en el heap
// //                 uint8_t *remainder_end = (uint8_t *)remainder + sizeof(block_t) + (current->size - size - sizeof(block_t));
// //                 if (remainder_end <= heap_end) {
// //                     remainder->size = current->size - size - sizeof(block_t);
// //                     remainder->is_free = 1;
// //                     remainder->next = current->next;

// //                     current->size = size;
// //                     current->next = remainder;
// //                 }
// //                 // si no entra el remainder completo, usamos todo el bloque sin dividir
// //             }

// //             // Marcar como usado
// //             current->is_free = 0;
// //             memory_manager.used_size += current->size + sizeof(block_t);

// //             // Actualizar estadísticas
// //             memory_info.used = memory_manager.used_size;
// //             memory_info.free = memory_manager.total_size - memory_manager.used_size;

// //             return (void *)((uint8_t *)current + sizeof(block_t));
// //         }
// //         current = current->next;
// //     }

// //     // No hay bloque válido
// //     return NULL;
// // }


// void *memory_alloc(uint64_t size) {
//     size = ALIGN4(size);

//     block_t *curr = NULL;

//     while (curr != NULL) {
//         if (curr->is_free && curr->size >= size) {
//             // Si el bloque es suficientemente grande para dividir
//             if (curr->size >= size + BLOCK_HEADER_SIZE + 4) {
//                 // Dividimos el bloque en uno asignado y otro libre
//                 block_t *new_block = (block_t *)((uint8_t *)curr + BLOCK_HEADER_SIZE + size);
//                 new_block->size = curr->size - size - BLOCK_HEADER_SIZE;
//                 new_block->is_free = 1;
//                 new_block->next = curr->next;

//                 curr->size = size;
//                 curr->next = new_block;
//             }

//             curr->is_free = 0;
//             memory_info.used += curr->size + BLOCK_HEADER_SIZE;

//             // Retornamos la dirección después de la cabecera (donde empieza el bloque útil)
//             return (void *)((uint8_t *)curr + BLOCK_HEADER_SIZE);
//         }

//         curr = curr->next;
//     }

// }
// // void memory_free(void* ptr){
// //     if (ptr == NULL) return;
    
// //     // Obtener el bloque desde el puntero (retroceder al header)
// //     block_t *block_to_free = (block_t *)((uint8_t *)ptr - sizeof(block_t));
    
// //     // Validar que el puntero esté dentro del heap
// //     if ((uint8_t *)block_to_free < (uint8_t *)memory_manager.heap_start || 
// //         (uint8_t *)block_to_free >= (uint8_t *)memory_manager.heap_start + memory_manager.total_size) {
// //         return; // Puntero inválido
// //     }
    
// //     // Validar que el bloque no esté ya libre
// //     if (block_to_free->is_free) {
// //         return; // Ya está libre
// //     }
    
// //     // Marcar como libre
// //     block_to_free->is_free = 1;
// //     memory_manager.used_size -= block_to_free->size + sizeof(block_t);
    
// //     // Fusionar con bloques libres adyacentes
// //     block_t *current = (block_t *)memory_manager.first_block;
    
// //     while (current != NULL && current->next != NULL) {
// //         // Si el bloque actual y el siguiente están libres y son contiguos
// //         if (current->is_free && current->next->is_free) {
// //             uint8_t *current_end = (uint8_t *)current + sizeof(block_t) + current->size;
// //             if (current_end == (uint8_t *)current->next) {
// //                 // Fusionar: expandir el bloque actual
// //                 current->size += sizeof(block_t) + current->next->size;
// //                 current->next = current->next->next;
// //                 continue; // Revisar otra vez desde el mismo bloque
// //             }
// //         }
// //         current = current->next;
// //     }
    
// //     // Actualizar estadísticas
// //     memory_info.used = memory_manager.used_size;
// //     memory_info.free = memory_manager.total_size - memory_manager.used_size;
// // }

// void memory_free(void* ptr){
//     if (ptr == NULL) return;
    
//     // Obtener el bloque desde el puntero (retroceder al header)
//     block_t *block_to_free = (block_t *)((uint8_t *)ptr - sizeof(block_t));
    
//     // Validar que el puntero esté dentro del heap
//     if ((uint8_t *)block_to_free < (uint8_t *)memory_manager.heap_start || 
//         (uint8_t *)block_to_free >= (uint8_t *)memory_manager.heap_start + memory_manager.total_size) {
//         printf("FREE: Puntero fuera del heap: %p\n", block_to_free);
//         return; // Puntero inválido
//     }
    
//     // Validar que el bloque no esté ya libre
//     if (block_to_free->is_free) {
//         printf("FREE: Bloque ya libre en %p\n", block_to_free);
//         return; // Ya está libre
//     }
    
//     // Print antes de liberar
//     printf("FREE: Liberando bloque en %p, size=%zu, is_free=%d, next=%p\n", block_to_free, block_to_free->size, block_to_free->is_free, block_to_free->next);

//     // Marcar como libre
//     block_to_free->is_free = 1;
//     memory_manager.used_size -= block_to_free->size + sizeof(block_t);
    
//     // Fusionar con bloques libres adyacentes
//     block_t *current = (block_t *)memory_manager.first_block;
    
//     while (current != NULL && current->next != NULL) {   // <-- agregar llaves
//         printf("FREE: Revisando fusion entre %p (free=%d, size=%zu) y %p (free=%d, size=%zu)\n",
//             current, current->is_free, current->size,
//             current->next, current->next->is_free, current->next->size);

//         if (current->is_free && current->next->is_free) {
//             uint8_t *current_end = (uint8_t *)current + sizeof(block_t) + current->size;
//             if (current_end == (uint8_t *)current->next) {
//                 printf("FREE: Fusionando %p y %p\n", current, current->next);
//                 current->size += sizeof(block_t) + current->next->size;
//                 current->next = current->next->next;
//                 printf("FREE: Bloque fusionado en %p, nuevo size=%zu, next=%p\n", current, current->size, current->next);
//                 continue; // seguir intentando coalescer desde el mismo nodo
//             }
//         }
//         current = current->next;
//     }                                                    // <-- cerrar llaves
    
//     // Print final del estado del heap
//     printf("FREE: Estado final del heap:\n");
//     current = memory_manager.first_block;
//     while (current != NULL) {
//         printf("FREE: Bloque en %p, size=%zu, is_free=%d, next=%p\n", current, current->size, current->is_free, current->next);
//         current = current->next;
//     }
    
//     // Actualizar estadísticas
//     memory_info.used = memory_manager.used_size;
//     memory_info.free = memory_manager.total_size - memory_manager.used_size;
// }

// memory_info_t memory_get_info(){
//     memory_info.size = memory_manager.total_size;
//     memory_info.used = memory_manager.used_size;
//     memory_info.free = memory_manager.total_size - memory_manager.used_size;
//     return memory_info;
// }