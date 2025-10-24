// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./include/shell.h"
#include <stdint.h>
#include "./include/syscalls.h"
#include "./include/man.h"
#include <libasm.h>
#include "./include/test_functions.h"
#include <stddef.h>

/* Enum para la cantidad de argumentos recibidos */
typedef enum { NO_PARAMS = 0, SINGLE_PARAM, DUAL_PARAM } functionType;

#define QTY_BYTES 32 /* Cantidad de bytes de respuesta del printmem */
#define DEFAULT_FONT_SIZE 1
#define MIN_FONT_SIZE 1
#define MAX_FONT_SIZE 3

#define WELCOME "Bienvenido a Cactiland OS!\n"
#define INVALID_COMMAND "Comando invalido!\n"
#define WRONG_PARAMS "La cantidad de parametros ingresada es invalida\n"
#define INVALID_FONT_SIZE "Dimension invalida de fuente\n"
#define CHECK_MAN "Escriba \"man %s\" para ver como funciona el comando\n"
#define CHECK_MAN_FONT "Escriba \"man font-size\" para ver las dimensiones validas\n"

typedef struct {
    char * name;                    // Nombre del comando
    char * description;             // Descripcion del comando (para help)
    union {                         // Puntero a la funcion
       int  (*f)(void);
       int  (*g)(char *);
       int  (*h)(char *, char *);
       void (*s)(char **);          // Variante que recibe char** (argv)
    };
    functionType ftype;             // Cantidad de argumentos del comando
} Command;

static void help();
static void man(char * command);
static void printInfoReg();
static void time();
static int div(char * num, char * div);
static void fontSize(char * size);
static void printMem(char * pos);
static int getCommandIndex(char * command);
static void mm_test(char ** max_memory);

static Command commands[QTY_COMMANDS];

void init() {
    commands[0]  = (Command){ "help",           "Listado de comandos",                                                                 .f = (void*)&help,        NO_PARAMS };
    commands[1]  = (Command){ "man",            "Manual de uso de los comandos",                                                       .g = (void*)&man,         SINGLE_PARAM };
    commands[2]  = (Command){ "inforeg",        "Informacion de los registos capturados",                                             .f = (void*)&printInfoReg, NO_PARAMS };
    commands[3]  = (Command){ "time",           "Despliega la hora actual UTC - 3",                                                   .f = (void*)&time,        NO_PARAMS };
    commands[4]  = (Command){ "div",            "Division entera de dos numeros naturales",                                           .h = (void*)&div,         DUAL_PARAM };
    commands[5]  = (Command){ "kaboom",         "Ejecuta una excepcion de Invalid Opcode",                                            .f = (void*)&kaboom,      NO_PARAMS };
    commands[6]  = (Command){ "font-size",      "Cambia dimensiones de la fuente",                                                    .g = (void*)&fontSize,    SINGLE_PARAM };
    commands[7]  = (Command){ "printmem",       "Vuelco de memoria de 32 bytes desde direccion dada",                                 .g = (void*)&printMem,    SINGLE_PARAM };
    commands[8]  = (Command){ "clear",          "Limpia toda la pantalla",                                                            .f = (void*)&clear,       NO_PARAMS };
    commands[9]  = (Command){ "test-mm",        "Corre el test del memory manager",                                                   .s = (void*)&mm_test,     SINGLE_PARAM };
    commands[10] = (Command){ "test-processes", "Corre el test de procesos",                                                          .f = (void*)&test_processes, NO_PARAMS };
    commands[11] = (Command){ "test-prio",      "Corre el test de prioridades",                                                       .f = (void*)&test_prio,   NO_PARAMS };
}

void run_shell() {
    init();
    int index;
    puts(WELCOME);
    while (1) {
        putchar('>');
        char command[MAX_CHARS] = {0};
        char arg1[MAX_CHARS] = {0};
        char arg2[MAX_CHARS] = {0};

        int qtyParams = scanf("%s %s %s", command, arg1, arg2);
        if (qtyParams <= 0) {
            continue;
        }

        index = getCommandIndex(command);
        if (index == -1) {
            if (command[0] != 0)
                printErr(INVALID_COMMAND);
            continue;
        }

        int funcParams = commands[index].ftype;
        if (qtyParams - 1 != funcParams) {
            printErr(WRONG_PARAMS);
            printf(CHECK_MAN, command);
            continue;
        }

        switch (commands[index].ftype) {
            case NO_PARAMS:
                commands[index].f();
                break;

            case SINGLE_PARAM: {
                // Si el comando fue registrado con .s (char**),
                // armamos argv con un único parámetro y lo pasamos.
                if (commands[index].s != NULL) {
                    char *argv1[2] = { arg1, NULL };
                    commands[index].s(argv1);
                } else {
                    commands[index].g(arg1);
                }
                break;
            }

            case DUAL_PARAM:
                commands[index].h(arg1, arg2);
                break;
        }
    }
}

/**
 * @brief  Devuelve el indice del vector de comandos dado su nombre
 * @param  command: Nombre del comando a buscar
 * @return  Indice del comando
 */
static int getCommandIndex(char * command) {
    int idx = 0;
    for (; idx < QTY_COMMANDS; idx++) {
        if (!strcmp(commands[idx].name, command))
            return idx;
    }
    return -1;
}

static void help() {
    for (int i = 0; i < QTY_COMMANDS; i++)
        printf("%s: %s\r\n", commands[i].name, commands[i].description);
}

static int div(char * num, char * div) {
    printf("%s/%s=%d\r\n", num, div, atoi(num)/atoi(div));
    return 1;
}

static void time() {
    uint32_t secs = getSeconds();
    uint32_t h = secs / 3600, m = secs % 3600 / 60, s = secs % 3600 % 60;
    printf("%2d:%2d:%2d\r\n", h, m, s);
}

static void fontSize(char * size) {
    int s = atoi(size);
    if (s >= MIN_FONT_SIZE && s <= MAX_FONT_SIZE)
        setFontSize((uint8_t)atoi(size));
    else {
        printErr(INVALID_FONT_SIZE);
        puts(CHECK_MAN_FONT);
    }
}

static void printMem(char * pos) {
    uint8_t resp[QTY_BYTES];
    char * end;
    getMemory(strtoh(pos, &end), resp);
    for (int i = 0; i < QTY_BYTES; i++) {
        printf("0x%2x ", resp[i]);
        if (i % 4 == 3)
            putchar('\n');
    }
}

static char * _regNames[] = {"RIP", "RSP", "RAX", "RBX", "RCX", "RDX", "RBP", "RDI", "RSI", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15"};
static void printInfoReg() {
    int len = sizeof(_regNames)/sizeof(char *);
    uint64_t regSnapshot[len];
    getInfoReg(regSnapshot);
    for (int i = 0; i < len; i++)
        printf("%s: 0x%x\n", _regNames[i], regSnapshot[i]);
}

static void man(char * command) {
    int idx = getCommandIndex(command);
    if (idx != -1)
        printf("%s\n", usages[idx]);
    else
        printErr(INVALID_COMMAND);
}

// Wrapper que adapta (char**) para test_mm(argc, argv)
static void mm_test(char ** max_memory) {
    if(atoi(max_memory[0]) <= 0) {
        printErr("El parametro debe ser un numero positivo\n");
        return;
    }
    test_mm(1, max_memory);
}