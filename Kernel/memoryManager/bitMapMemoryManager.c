// #include "memoryManager.h"
// #include <stdint.h>

// #define BLOCK_SIZE 64

// typedef struct {
//     uint8_t *bitmap;        // 0=libre, 1=usado, 2=header (primero de N bloques)
//     void *heap_start;
//     uint64_t total_blocks;
//     uint64_t used_blocks;
// } simple_mm_t;

// static simple_mm_t mm;
// static memory_info_t info;

// void memory_init(void *start, uint64_t size) {
//     mm.heap_start = start;
//     mm.total_blocks = size / BLOCK_SIZE;
//     mm.used_blocks = 0;
//     mm.bitmap = (uint8_t *)start;
    
//     // Inicializar todo libre
//     for (uint64_t i = 0; i < mm.total_blocks; i++) {
//         mm.bitmap[i] = 0;
//     }
    
//     info.size = mm.total_blocks * BLOCK_SIZE;
//     info.used = 0;
//     info.free = info.size;
// }

// void *memory_alloc(uint64_t size) {
//     if (size == 0) return NULL;
    
//     // Bloques necesarios (con espacio para guardar el tamaño)
//     uint64_t blocks = (size + sizeof(uint32_t) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
//     if (blocks > mm.total_blocks - mm.used_blocks) return NULL;
    
//     // Buscar espacio consecutivo
//     for (uint64_t i = 0; i <= mm.total_blocks - blocks; i++) {
        
//         // Ver si desde i hay N bloques libres
//         uint64_t j;
//         for (j = 0; j < blocks; j++) {
//             if (mm.bitmap[i + j] != 0) break;  // Ocupado, siguiente
//         }
        
//         if (j == blocks) {  // Encontramos!
//             // Marcar primer bloque como header
//             mm.bitmap[i] = 2;
            
//             // Marcar resto como usado
//             for (uint64_t k = 1; k < blocks; k++) {
//                 mm.bitmap[i + k] = 1;
//             }
            
//             mm.used_blocks += blocks;
            
//             // Guardar cantidad de bloques al inicio
//             void *ptr = (void *)((uint8_t *)mm.heap_start + i * BLOCK_SIZE);
//             *(uint32_t *)ptr = blocks;
            
//             // Actualizar info
//             info.used = mm.used_blocks * BLOCK_SIZE;
//             info.free = (mm.total_blocks - mm.used_blocks) * BLOCK_SIZE;
            
//             // Retornar después del header
//             return (void *)((uint8_t *)ptr + sizeof(uint32_t));
//         }
//     }
    
//     return NULL;  // No hay espacio
// }

// void memory_free(void *ptr) {
//     if (ptr == NULL) return;
    
//     // Calcular índice del bloque
//     void *block_start = (void *)((uint8_t *)ptr - sizeof(uint32_t));
//     uint64_t offset = (uint8_t *)block_start - (uint8_t *)mm.heap_start;
//     uint64_t index = offset / BLOCK_SIZE;
    
//     if (index >= mm.total_blocks) return;
//     if (mm.bitmap[index] != 2) return;  // No es header, error
    
//     // Leer cantidad de bloques
//     uint32_t blocks = *(uint32_t *)block_start;
    
//     if (blocks == 0 || index + blocks > mm.total_blocks) return;
    
//     // Liberar todos los bloques
//     mm.bitmap[index] = 0;
//     for (uint64_t i = 1; i < blocks; i++) {
//         mm.bitmap[index + i] = 0;
//     }
    
//     mm.used_blocks -= blocks;
    
//     // Actualizar info
//     info.used = mm.used_blocks * BLOCK_SIZE;
//     info.free = (mm.total_blocks - mm.used_blocks) * BLOCK_SIZE;
// }

// memory_info_t memory_get_info(void) {
//     info.size = mm.total_blocks * BLOCK_SIZE;
//     info.used = mm.used_blocks * BLOCK_SIZE;
//     info.free = (mm.total_blocks - mm.used_blocks) * BLOCK_SIZE;
//     return info;
// }












// #include "memoryManager.h"

// #define FREE 0
// #define USED 1
// #define HEADER 2

// #define MAXBLOCKSINMEMORY 1024
// typedef struct MemoryManagerCDT{
//     void * bitmapStart;
//     uint64_t cantBlocks;
//     uint64_t blocksUsed;
//     uint8_t bitmapState[MAXBLOCKSINMEMORY]; 
// } MemoryManagerCDT;

// static void * firstMemoryPlace;

// static MemoryManagerADT getMM();

// MemoryManagerADT memory_init(void * const restrict memoryForMemoryManager, uint64_t managedMemory){
//     if(managedMemory < sizeof(MemoryManagerCDT) + BLOCKSIZE){
//         return NULL; 
//     }
//     firstMemoryPlace = memoryForMemoryManager;
//     MemoryManagerADT memoryManager = (MemoryManagerADT) firstMemoryPlace;

//     memoryManager->cantBlocks = (managedMemory - sizeof(MemoryManagerCDT)) / BLOCKSIZE; 

//     memoryManager->bitmapStart = (void *)((uint8_t *)memoryForMemoryManager + sizeof(MemoryManagerCDT) + memoryManager->cantBlocks);
    
//     memoryManager->blocksUsed = 0;

//     for(uint64_t i = 0; i < memoryManager->cantBlocks; i++){
//         memoryManager->bitmapState[i] = FREE; 
//     }

//     return memoryManager;
// }

// void * memory_alloc(const size_t memoryToAllocate){
//     MemoryManagerADT memoryManager = getMM();
//     if(memoryToAllocate + memoryManager->blocksUsed > memoryManager->cantBlocks){
//         return NULL;
//     }
//     uint64_t flag = 0;
//     uint64_t j = 0;
//     for(uint64_t i = 0; i < memoryManager->cantBlocks; i++){
//         flag = 0;
//         if(memoryManager->bitmapState[i] == FREE){
//             for(j = 0; j < memoryToAllocate && !flag; j++){
//                 if(memoryManager->bitmapState[i + j] == USED){
//                     flag = 1;
//                 }
//             }
//             if(!flag){
//                 memoryManager->bitmapState[i] = HEADER;
//                 for(uint64_t k = 1; k < j; k++){
//                     memoryManager->bitmapState[i + k] = USED;
//                 }
//                 memoryManager->blocksUsed += j;
//                 return (void *) ((uint8_t *)memoryManager->bitmapStart + i * BLOCKSIZE);
//             }  
//         }
//         if(flag){
//             i += j - 1; 
//         }
//     }
//     return NULL;
// }

// void memory_free(void * const restrict memoryToFree){
//     MemoryManagerADT memoryManager = getMM();    
//     uint64_t i = (uint8_t *) memoryToFree - (uint8_t *) memoryManager->bitmapStart;

//     if(i >= memoryManager->cantBlocks || memoryManager->bitmapState[i] != HEADER){
//         return;
//     }

//     memoryManager->bitmapState[i] = FREE;
//     memoryManager->blocksUsed--;

//     for(i += 1 ; i < memoryManager->cantBlocks && memoryManager->bitmapState[i] == USED ; i++){
//         memoryManager->bitmapState[i] = FREE;
//         memoryManager->blocksUsed--;
//     }
    
// }

// memory_info_t memory_get_info(){
//     MemoryManagerADT memoryManager = getMM();
//     memory_info_t info;
//     info.size = memoryManager->cantBlocks;
//     info.used = memoryManager->blocksUsed;
//     info.free = memoryManager->cantBlocks - memoryManager->blocksUsed;
//     return info;
// }

// static MemoryManagerADT getMM(){
//     return (MemoryManagerADT) firstMemoryPlace;
// }