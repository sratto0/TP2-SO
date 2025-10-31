#ifndef _STDLIB_H
#define _STDLIB_H
#include <stdint.h>
#include "syscalls.h"

/**
 * @brief Convierte un arreglo de caracteres en numero entero en formato decimal
 * @param s: Cadena de caracteres
 * @return Numero entero en formato decimal 
 */
int atoi(char* s);

/**
 * @brief Convierte una cadena de caracteres en numero entero en formato decimal
 * @param s: Cadena de caracteres
 * @param end: Puntero al final de la cadena de caracteres
 * @return Numero entero en formato decimal 
 */
int strtoi(char* s, char ** end);

/**
 * @brief Convierte una cadena de caracteres en numero entero en formato hexadecimal
 * @param s: Cadena de caracteres
 * @param end: Puntero al final de la cadena de caracteres
 * @return Numero entero en formato hexadecimal 
 */
int strtoh(char* s, char ** end);

/**
 * @brief Convierte un numero en una base en una cadena de caracteres
 * @param n: Numero que se desea convertir
 * @param buffer: Buffer para ir guardando la conversion
 * @param base: Base del numero que se desea convertir
 * @return Longitud del buffer (numero que se convirtio en cadena de caracteres)
 */
int itoa(uint64_t n, char* buffer, int base);

void * my_malloc(uint64_t size);

void my_free(void * ptr);

int64_t my_create_process(entry_point_t main, char ** argv, char * name, int * file_descriptors);

int64_t my_wait_pid(int64_t pid, int * exit_code);

uint64_t my_exit(int exit_code);

uint64_t my_yield();

int64_t my_getpid();

int64_t my_block_process(int64_t pid);

int64_t my_unblock_process(int64_t pid);

int64_t my_nice(int64_t pid, uint8_t priority);

int64_t my_kill(int64_t pid);

#endif
