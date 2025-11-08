#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "color.h"
#include <stdint.h>

#include "../../../SharedLibraries/sharedStructs.h"




/**
 * @brief Escribe a partir del descriptor recibido un caracter
 * @param fd: FileDescriptor (STDOUT | STDERR)
 * @param c: Caracter a escribir
 */
void write(fd_t fd, char * buffer, uint64_t len);

/**
 * @brief Lee un byte a partir del descriptor recibido
 * @param fd: FileDescriptor (STDIN | KBDIN)
 * @return Byte leido
 */
uint8_t read(fd_t fd, char * destination_buffer, uint64_t len);


/**
 * @brief Devuelve la hora expresada en segundos
 * @return Hora expresada en segundos
 */
uint32_t getSeconds();

/**
 * @brief Pone todos los pixeles de la pantalla en negro y limpia el buffer de
 * video
 */
void clear(void);

/**
 * @brief
 * @param regarr: Vector donde se llena la informacion de los registros
 * @return Puntero a la informacion de los registros
 */
uint64_t *getInfoReg(uint64_t *regarr);

/**
 * @brief Cambia el tamaño de la fuente
 * @param size: (1|2|3)
 */
void setFontSize(uint8_t size);

/**
 * @brief Devuelve las dimensiones de la pantalla
 * @return 32 bits menos significativos el ancho, 32 el alto
 */
uint32_t getScreenResolution();

/**
 * @brief  Dibuja un rectangulo
 * @param  x: Origen en x
 * @param  y: Origen en y
 * @param  width: Ancho
 * @param  height: Alto
 * @param  color: Color de relleno
 */
void drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
              Color color);

/**
 * @brief Devuelve la cantidad de ticks actuales
 * @return Cantidad de ticks
 */
uint64_t getTicks();

/**
 * @brief Llena un vector con 32 bytes de informacion a partir de una direccion
 * de memoria en hexa
 * @param pos: Direccion de memoria a partir de la cual se llena el vector
 * @param vec: Vector en el cual se llena la informacion
 */
void getMemory(uint64_t pos, uint8_t *vec);

/**
 * @brief Ejecuta una excepcion de Invalid Opcode Exception
 */
void kaboom();

/**
 * @brief Establece un color de fuente
 * @param r: Color rojo
 * @param g: Color verde
 * @param b: Color azul
 */
void setFontColor(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Retorna el color de fuente que se esta usando actualmente
 * @return Color
 */
Color getFontColor();

/**
 * @brief Asigna memoria dinamica
 * @param size: Cantidad de bytes a asignar
 * @return Puntero a la memoria asignada
 */

void *sys_malloc(uint64_t size);

void sys_free(void *ptr);

int64_t sys_create_process(entry_point_t main, char **argv, char *name,
                           fd_t *file_descriptors);

uint64_t sys_exit_process(int64_t exit_code);

uint64_t sys_yield();

int64_t sys_getpid();

int64_t sys_block_process(int64_t pid);

int64_t sys_unblock_process(int64_t pid);

int64_t sys_set_priority(int64_t pid, uint8_t priority);

process_info_t *sys_get_processes_info();

int64_t sys_kill_process(int64_t pid);

int64_t sys_wait_pid(int64_t pid, int *ret);

uint64_t sys_total_cpu_ticks();

int64_t sys_sem_open(char *name, uint64_t initialValue);

int64_t sys_sem_wait(char *name);

int64_t sys_sem_post(char *name);

int64_t sys_sem_close(char *name);

memory_info_t *sys_memory_get_info();

void sys_sleep(uint64_t seconds);

int sys_create_pipe(fd_t fds[2]);

int sys_write_pipe(fd_t fd, const char * buffer, int size);

int sys_read_pipe(fd_t fd, char * buffer, int size);

void sys_destroy_pipe(fd_t fd);

void sys_adopt_child(int64_t pid);


#endif
