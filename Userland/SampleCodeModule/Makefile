include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard [^_]*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:asm/%.asm=obj/%.asm.o)
TEST_OBJECTS=test_mm.o test_prio.o test_sync.o test_processes.o test_util.o syscall.o
INCLUDE=./include

all: tests $(MODULE)

$(MODULE): _loader.c $(OBJECTS) $(OBJECTS_ASM) $(TEST_OBJECTS)
	$(GCC) $(GCCFLAGS) -I$(INCLUDE) -T sampleCodeModule.ld _loader.c $(OBJECTS_ASM) $(OBJECTS) $(TEST_OBJECTS) -o ../$(MODULE)
	$(GCC) $(GCCFLAGS) -I$(INCLUDE) -T sampleCodeModule.ld -Wl,--oformat=elf64-x86-64 _loader.c $(OBJECTS_ASM) $(OBJECTS) $(TEST_OBJECTS) -o ../0000-sampleCodeModule.elf

%.o: %.c
	mkdir -p obj
	$(GCC) $(GCCFLAGS) -I$(INCLUDE) -c $< -o $@

obj/%.asm.o: asm/%.asm
	mkdir -p obj
	$(ASM) $(ASMFLAGS) $< -o $@

tests: 
	cd tests && make all

clean:
	rm -rf *.o
	rm -rf obj
	cd tests && make clean && cd ..

.PHONY: all clean tests