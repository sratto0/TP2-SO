#ifndef _MAN_H
#define _MAN_H
#include <shell.h>
/* Matriz con una breve explicacion de uso de cada uno de los comandos */
char *usages[QTY_COMMANDS] = {
    "Uso: help - muestra todos los comandos que existen en la terminal. No "
    "recibe parametros",

    "Uso: man [COMANDO] - explica el funcionamiento de un comando enviado como "
    "parametro",

    "Uso: inforeg - muestra informacion de los registros en un momento "
    "arbitrario de ejecucion del sistema. No recibe parametros",

    "Uso: time - despliega la hora actual. No recibe parametros",

    "Uso: div [OP1] [OP2] - hace la division entera de dos numeros naturales "
    "que recibe por parametro\n"
    "Ejemplo: div 10 5",

    "Uso: kaboom - arroja una excepcion de invalid opcode. No recibe "
    "parametros",

    "Uso: font-size [1|2|3] - cambia la medida de la fuente. Para eso se deben "
    "enviar por parametro el numero 1, 2 o 3\n"
    "Ejemplo: font-size 2",

    "Uso: printmem [DIR] - imprime los primeros 32 bytes de memoria a partir "
    "de una direccion de memoria enviada como parametro\n"
    "Ejemplo: printmem 10ff8c",

    "Uso: clear - limpia la pantalla. No recibe parametros",

    "Uso: test-mm - corre el test del memory manager. No recibe parametros",

    "Uso: test-processes - corre el test de procesos. No recibe parametros",

    "Uso: test-prio - corre el test de prioridades. No recibe parametros",

    "Uso: test-sync - corre el test de sincronizacion. No recibe parametros",

    "Uso: ps - muestra informacion de los procesos. No recibe parametros",

    "Uso: mem - muestra informacion del uso de memoria. No recibe parametros",

    "Uso: loop [SEGUNDOS] - imprime su ID con un saludo cada una determinada "
    "cantidad de segundos\n",

    "Uso: kill [PID] - mata un proceso dado su ID\n",

    "Uso: nice [PID] [NUEVA_PRIO] - cambia la prioridad de un proceso dado su "
    "ID y la nueva prioridad\n",

    "Uso: block [PID] - bloquea o desbloquea un proceso dado su ID\n",

    "Uso: unblock [PID] - desbloquea un proceso dado su ID\n",

    "Uso: cat - imprime el stdin tal como lo recibe. No recibe parametros",

    "Uso: filter - filtra las vocales del input. No recibe parametros",

    "Uso: wc - cuenta la cantidad de lineas del input. No recibe parametros"};
#endif