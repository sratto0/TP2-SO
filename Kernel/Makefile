include Makefile.inc

KERNEL=kernel.bin

ifdef MM
	MM_FLAG := -D$(MM)
else
	MM_FLAG :=
endif


SOURCES=$(wildcard *.c) $(wildcard memoryManagement/*.c) $(wildcard process/*.c) $(wildcard semaphores/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -I../SharedLibraries -c $< -o $@ $(MM_FLAG)

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o *.o ./memoryManagement/*.o *.bin ./process/*.o ./semaphores/*.o

.PHONY: all clean

# include Makefile.inc

# KERNEL=kernel.bin
# # Use all C sources except the original memoryManager.c; we will use buddyMemoryManager.c instead
# SOURCES=$(filter-out memoryManager/memoryManager.c, $(wildcard *.c memoryManager/*.c processes/*.c)) \
# 		$(wildcard processes/*.c)

# # SOURCES=$(filter-out ./memoryManager/memoryManager.c,$(wildcard *.c)) \
# # 		$(wildcard processes/*.c)
# SOURCES_ASM=$(wildcard asm/*.asm)
# OBJECTS=$(SOURCES:.c=.o)
# OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
# LOADERSRC=loader.asm

# LOADEROBJECT=$(LOADERSRC:.asm=.o)
# STATICLIBS=

# all: $(KERNEL)

# $(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
# 	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)
# 	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)

# %.o: %.c
# 	$(GCC) $(GCCFLAGS) -DUSE_BUDDY_MM -I./include -c $< -o $@

# %.o : %.asm
# 	$(ASM) $(ASMFLAGS) $< -o $@

# $(LOADEROBJECT):
# 	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

# clean:
# 	rm -rf asm/*.o *.o *.bin

# .PHONY: all clean
