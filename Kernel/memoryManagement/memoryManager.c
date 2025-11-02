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

// static void *heap_start = NULL;
// static uint32_t heap_size = 0;
// static block_t *free_list = NULL;
// static memory_info_t mem_info;


// void memory_init(void * start, uint64_t size){

//     heap_start = start;
//     heap_size = size;

    
//     mem_info.size = size;
//     mem_info.used = 0;

//     free_list = (block_t *)start;
//     free_list->size = size - BLOCK_HEADER_SIZE; // solo la parte de datos
//     free_list->is_free = 1;
//     free_list->next = NULL;
// }


// void *memory_alloc(uint64_t size) {
//     size = ALIGN4(size);

//     block_t *curr = free_list;

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

//     return NULL;
// }


// void memory_free(void* ptr) {
//     if (ptr == NULL) return;

//     block_t *block_to_free = (block_t *)((uint8_t *)ptr - BLOCK_HEADER_SIZE);
//     block_to_free->is_free = 1;
//     memory_info.used -= block_to_free->size + BLOCK_HEADER_SIZE;

//     // Coalescing de bloques libres adyacentes
//     block_t *curr = free_list;
//     while (curr != NULL && curr->next != NULL) {
//         if (curr->is_free && curr->next->is_free && (uint8_t *)curr + BLOCK_HEADER_SIZE + curr->size == (uint8_t *)curr->next) {
//             curr->size += BLOCK_HEADER_SIZE + curr->next->size;
//             curr->next = curr->next->next;
//         } else {
//             curr = curr->next;
//         }
//     }
// }


// memory_info_t *mem_dump() {
//     memory_info.free = memory_info.size - memory_info.used;
//     return &memory_info;
// }