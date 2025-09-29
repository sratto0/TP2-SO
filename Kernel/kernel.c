// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <interrupts.h>
#include <video.h>
#include "memoryManager.h"

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void * const sampleCodeModuleAddress = (void*)0x400000;
static void * const sampleDataModuleAddress = (void*)0x500000;

typedef int (*EntryPoint)();


void clearBSS(void * bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void * getStackBase()
{
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void initializeKernelBinary()
{
	void * moduleAddresses[] = { sampleCodeModuleAddress, sampleDataModuleAddress };
	loadModules(&endOfKernelBinary, moduleAddresses);
	clearBSS(&bss, &endOfKernel - &bss);
}

void memory_test() {
    void *ptr1 = memory_alloc(100);
    void *ptr2 = memory_alloc(200);
    void *ptr3 = memory_alloc(50);
    
    // Si tienes funciones de print, puedes usarlas aquí para debug
    printf("ptr1: %p, ptr2: %p, ptr3: %p\n", ptr1, ptr2, ptr3);
    
    memory_info_t info = memory_get_info();
    printf("Used: %lu, Free: %lu\n", info.used, info.free);
    
    memory_free(ptr2);  // Liberar el del medio
    memory_free(ptr1);
    memory_free(ptr3);
}

int main()
{	
	load_idt();

    // Inicializar memory manager
    // Usar memoria después de los módulos cargados
    void * heap_start = (void*)0x600000;  // Después de sampleDataModuleAddress
    uint64_t heap_size = 1024 * 1024;    // 1MB
    
    memory_init(heap_start, heap_size);
    
    // Probar el memory manager
    memory_test();

	((EntryPoint)sampleCodeModuleAddress)();
	while(1) _hlt();
	return 0;
}

