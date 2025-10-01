include ../../Makefile.inc

SOURCES=test_mm.c test_prio.c test_sync.c test_processes.c syscall.c
OBJECTS=$(SOURCES:.c=.o)
UTIL=test_util.o
INCLUDE=../include

all: $(UTIL) $(OBJECTS)
	cp $(OBJECTS) $(UTIL) ../

%.o: %.c $(UTIL)
	$(GCC) $(GCCFLAGS) -g -I$(INCLUDE) -I. -c $< -o $@

$(UTIL): test_util.c
	$(GCC) $(GCCFLAGS) -g -I$(INCLUDE) -I. -c $< -o $@

clean:
# 	rm -rf asm/*.o *.o
	rm -rf *.o
	rm -f ../test_*.o ../test_util.o  # Limpiar archivos copiados al directorio padre

.PHONY: all clean