// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com


#include "./include/shell.h"
#include "../../SharedLibraries/sharedStructs.h"
#include "./include/inputParser.h"
#include "./include/man.h"
#include "./include/shellCommands.h"
#include "./include/syscalls.h"
#include "./include/test_functions.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <libasm.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Enum para la cantidad de argumentos recibidos */
typedef enum { NO_PARAMS = 0, SINGLE_PARAM, DUAL_PARAM } functionType;

#define QTY_BYTES 32 /* Cantidad de bytes de respuesta del printmem */
#define DEFAULT_FONT_SIZE 1
#define MIN_FONT_SIZE 1
#define MAX_FONT_SIZE 3

#define WELCOME "Bienvenido a Nuestro SO!\n"
#define INVALID_COMMAND "Comando invalido!\n"
#define WRONG_PARAMS "La cantidad de parametros ingresada es invalida\n"
#define INVALID_FONT_SIZE "Dimension invalida de fuente\n"
#define CHECK_MAN "Escriba \"man %s\" para ver como funciona el comando\n"
#define CHECK_MAN_FONT                                                         \
  "Escriba \"man font-size\" para ver las dimensiones validas\n"

typedef struct {
  char *name;        // Nombre del comando
  char *description; // Descripcion del comando (para help)
  entry_point_t f;
} Command;

static void help(int argc, char **argv);
static void man(int argc, char **argv);
static void printInfoReg(int argc, char **argv);
static void time(int argc, char **argv);
static void div(int argc, char **argv);
static void fontSize(int argc, char **argv);
static void printMem(int argc, char **argv);
static int getCommandIndex(char *command);
static void create_single_process(input_parser_t *parser);

const static Command commands[] = {
    {"help", "Listado de comandos", (entry_point_t)help},
    {"man", "Manual de uso de los comandos", (entry_point_t)man},
    {"inforeg", "Informacion de los registos capturados",
     (entry_point_t)printInfoReg},
    {"time", "Despliega la hora actual UTC - 3", (entry_point_t)time},
    {"div", "Division entera de dos numeros naturales", (entry_point_t)div},
    {"kaboom", "Ejecuta una excepcion de Invalid Opcode",
     (entry_point_t)kaboom},
    {"font-size", "Cambia dimensiones de la fuente", (entry_point_t)fontSize},
    {"printmem", "Vuelco de memoria de 32 bytes desde direccion dada",
     (entry_point_t)printMem},
    {"clear", "Limpia toda la pantalla", (entry_point_t)clear},
    {"test-mm", "Corre el test del memory manager", (entry_point_t)test_mm},
    {"test-processes", "Corre el test de procesos",
     (entry_point_t)test_processes},
    {"test-prio", "Corre el test de prioridades", (entry_point_t)test_prio},
    {"test-sync", "Corre el test de sincronizacion", (entry_point_t)test_sync},
    {"ps", "Muestra informacion de los procesos", (entry_point_t)cmd_ps},
    {"mem", "Muestra informacion del uso de memoria", (entry_point_t)cmd_mem},
    {"loop",
     "Imprime su ID con un saludo cada una determinada cantidad de segundos",
     (entry_point_t)cmd_loop},
    {"kill", "Mata un proceso dado su ID", (entry_point_t)cmd_kill},
    {"nice",
     "Cambia la prioridad de un proceso dado su ID y la nueva prioridad",
     (entry_point_t)cmd_nice},
    {"block", "Bloquea o desbloquea un proceso dado su ID",
     (entry_point_t)cmd_block},
    {"unblock", "Desbloquea un proceso dado su ID", (entry_point_t)cmd_unblock},
    {"cat", "Imprime el stdin tal como lo recibe", (entry_point_t)cmd_cat},
    {"wc", "Cuenta la cantidad de líneas del input", (entry_point_t)cmd_wc},
    {"filter", "Filtra las vocales del input", (entry_point_t)cmd_filter}};

void run_shell() {
  puts(WELCOME);
  while (1) {
    putchar('>');
    char raw_input[MAX_CHARS] = {0};
    scanf("%S", raw_input);
    input_parser_t *parser = parse_input(raw_input);
    if (parser == NULL) {
      printErr(INVALID_COMMAND);
      continue;
    }

    if (parser->qty_shell_programs == 1) {
      create_single_process(parser);
    } 
    free_parser(parser);
  }
}

static void create_single_process(input_parser_t *parser) {
  shell_program_t *program = parser->shell_programs[0];
  int idx = getCommandIndex(program->name);
  if (idx == -1) {
    printErr(INVALID_COMMAND);
    return;
  }
  if (parser->background) {
    // VER!!
  } else {
    int16_t pid = my_create_process((entry_point_t)commands[idx].f,
                                    program->params, program->name, NULL);
    my_wait_pid(pid, NULL);
  }
}

/**
 * @brief  Devuelve el indice del vector de comandos dado su nombre
 * @param  command: Nombre del comando a buscar
 * @return  Indice del comando
 */
static int getCommandIndex(char *command) {
  int idx = 0;
  for (; idx < QTY_COMMANDS; idx++) {
    if (!strcmp(commands[idx].name, command))
      return idx;
  }
  return -1;
}

static void help(int argc, char **argv) {
  if (argc != 1) {
    printErr(WRONG_PARAMS);
    return;
  }
  for (int i = 0; i < QTY_COMMANDS; i++)
    printf("%s: %s\r\n", commands[i].name, commands[i].description);
}

static void div(int argc, char **argv) {
  if (argc != 3) {
    printErr(WRONG_PARAMS);
    return;
  }
  char *num = argv[1];
  char *div = argv[2];
  printf("%s/%s=%d\r\n", num, div, atoi(num) / atoi(div));
}

static void time(int argc, char **argv) {
  uint32_t secs = getSeconds();
  uint32_t h = secs / 3600, m = secs % 3600 / 60, s = secs % 3600 % 60;
  printf("%2d:%2d:%2d\r\n", h, m, s);
}

static void fontSize(int argc, char **argv) {
  if (argc != 2) {
    printErr(WRONG_PARAMS);
    return;
  }
  int s = atoi(argv[1]);
  if (s >= MIN_FONT_SIZE && s <= MAX_FONT_SIZE)
    setFontSize((uint8_t)s);
  else {
    printErr(INVALID_FONT_SIZE);
    puts(CHECK_MAN_FONT);
  }
}

static void printMem(int argc, char **argv) {
  if (argc != 2) {
    printErr(WRONG_PARAMS);
    return;
  }
  uint8_t resp[QTY_BYTES];
  char *end;
  getMemory(strtoh(argv[1], &end), resp);
  for (int i = 0; i < QTY_BYTES; i++) {
    printf("0x%2x ", resp[i]);
    if (i % 4 == 3)
      putchar('\n');
  }
}

static char *_regNames[] = {"RIP", "RSP", "RAX", "RBX", "RCX", "RDX",
                            "RBP", "RDI", "RSI", "R8",  "R9",  "R10",
                            "R11", "R12", "R13", "R14", "R15"};
static void printInfoReg(int argc, char **argv) {
  int len = sizeof(_regNames) / sizeof(char *);
  uint64_t regSnapshot[len];
  getInfoReg(regSnapshot);
  for (int i = 0; i < len; i++)
    printf("%s: 0x%x\n", _regNames[i], regSnapshot[i]);
}

static void man(int argc, char **argv) {
  if (argc != 2) {
    printErr(WRONG_PARAMS);
    return;
  }
  int idx = getCommandIndex(argv[1]);
  if (idx != -1)
    printf("%s\n", usages[idx]);
  else
    printErr(INVALID_COMMAND);
}