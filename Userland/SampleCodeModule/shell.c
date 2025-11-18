// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "shell.h"
#include "color.h"
#include "inputParser.h"
#include "sharedStructs.h"
#include "shellCommands.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "syscalls.h"
#include "test_functions.h"
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
#define CHECK_MAN_FONT                                                         \
  "Escriba \"man font-size\" para ver las dimensiones validas\n"

#define CAT_GENERAL "General"
#define CAT_INFO "Informacion"
#define CAT_TESTS "Tests"
#define CAT_PROCESOS "Procesos"
#define CAT_IO "Entrada/Salida"

#define COLOR_SECTION MAGENTA
#define COLOR_USAGE TURQUOISE
#define COLOR_DESC SILVER

#define RETURN_IF_INVALID(idx)                                                 \
  if ((idx) == -1) {                                                           \
    printfc(RED, INVALID_COMMAND);                                             \
    return;                                                                    \
  }

typedef struct {
  const char *name;
  const char *short_desc;
  entry_point_t f;
  const char *category;
  const char *usage;
  const char *details;
  const char *parameters;
  const char *notes;
  const char *example;
} Command;

static void help(int argc, char **argv);
static void man(int argc, char **argv);
static void printInfoReg(int argc, char **argv);
static void time(int argc, char **argv);
static void div(int argc, char **argv);
static void fontSize(int argc, char **argv);
static void printMem(int argc, char **argv);
static int getCommandIndex(char *command);
static void create_piped_processes(input_parser_t *parser);
static void create_single_process(input_parser_t *parser);
static void print_block(const char *text);
static void print_section(const char *title, const char *text);
static const char *command_category(const Command *cmd);
static Color category_color(const char *category);

const static Command commands[] = {
    {"help", "Lista todos los comandos disponibles", (entry_point_t)help,
     CAT_GENERAL, "help",
     "Muestra una tabla con todos los comandos agrupados por categoria.",
     "No recibe parametros.", "Use 'man <comando>' para ver mas detalles.",
     "help"},
    {"man", "Describe un comando en detalle", (entry_point_t)man, CAT_GENERAL,
     "man <comando>",
     "Imprime descripcion, parametros, notas y ejemplos del comando "
     "solicitado.",
     "<comando>  Nombre tal como figura en 'help'.",
     "Si el comando no existe se informa un error.", "man ps"},
    {"clear", "Limpia la pantalla y resetea el cursor", (entry_point_t)clear,
     CAT_GENERAL, "clear",
     "Borra el contenido de la consola y posiciona el cursor en la esquina "
     "superior.",
     "No recibe parametros.", NULL, "clear"},
    {"font-size", "Cambia el tamano de la fuente", (entry_point_t)fontSize,
     CAT_GENERAL, "font-size <1|2|3>",
     "Actualiza el tamano de la fuente usada por la terminal.",
     "<1|2|3>  Nivel de zoom (1=pequeno, 3=grande).",
     "El valor permanece hasta seleccionar otro.", "font-size 2"},
    {"div", "Divide dos numeros naturales", (entry_point_t)div, CAT_GENERAL,
     "div <dividendo> <divisor>",
     "Realiza una division entera e imprime solo el cociente.",
     "<dividendo>  Entero natural.\n<divisor>    Entero natural distinto de "
     "cero.",
     "No maneja signos ni valores fraccionarios.", "div 10 3"},
    {"kaboom", "Dispara una excepcion de opcode invalido",
     (entry_point_t)kaboom, CAT_GENERAL, "kaboom",
     "Ejecuta un opcode invalido para probar el manejador de excepciones.",
     "No recibe parametros.", "Provoca un dump de registros y vuelve al shell.",
     "kaboom"},
    {"inforeg", "Muestra el ultimo snapshot de registros",
     (entry_point_t)printInfoReg, CAT_INFO, "inforeg",
     "Imprime el estado guardado de los registros generales y de pila.",
     "No recibe parametros.",
     "El snapshot se captura al presionar Ctrl+S en el teclado.", "inforeg"},
    {"time", "Despliega la hora actual UTC-3", (entry_point_t)time, CAT_INFO,
     "time", "Lee el RTC y muestra la hora local en formato hh:mm:ss.",
     "No recibe parametros.", NULL, "time"},
    {"printmem", "Vuelca 32 bytes consecutivos de memoria",
     (entry_point_t)printMem, CAT_INFO, "printmem <direccion_hex>",
     "Lee y muestra 32 bytes a partir de la direccion fisica indicada.",
     "<direccion_hex>  Direccion en hexadecimal (sin 0x).",
     "Util para depurar contenido de memoria.", "printmem F00"},
    {"ps", "Lista PID, estado, prioridad y stack", (entry_point_t)cmd_ps,
     CAT_INFO, "ps",
     "Muestra para cada proceso su PID, nombre, estado, prioridad, stack y si "
     "está en foreground/background.",
     "No recibe parametros.",
     "Los procesos marcados como foreground aparecen como 'Foreground'.", "ps"},
    {"mem", "Muestra el uso de memoria", (entry_point_t)cmd_mem, CAT_INFO,
     "mem",
     "Consulta al administrador de memoria y muestra total, usado y libre.",
     "No recibe parametros.", NULL, "mem"},
    {"test-mm", "Ejecuta el test del memory manager", (entry_point_t)test_mm,
     CAT_TESTS, "test-mm <max_mem>",
     "Reserva y libera bloques aleatorios hasta alcanzar el limite indicado, "
     "verificando que no haya corrupcion.",
     "<max_mem>  Cantidad maxima de bytes a reservar antes de liberar.",
     "Use Ctrl+C para detener el test y volver a la shell.", "test-mm 4096"},
    {"test-processes", "Estresa al scheduler creando multiples procesos",
     (entry_point_t)test_processes, CAT_TESTS, "test-processes <cantidad>",
     "Crea la cantidad indicada de procesos y los va matando, bloqueando y "
     "desbloqueando al azar.",
     "<cantidad>  Cantidad de procesos que el test crea por ronda "
     "(1..MAX_PROCESSES-1).",
     "Corre en bucle infinito: detenelo con Ctrl+C cuando termines de observar "
     "el comportamiento.",
     "test-processes 10"},
    {"test-prio", "Muestra como impactan las prioridades en el scheduler",
     (entry_point_t)test_prio, CAT_TESTS, "test-prio <vueltas>",
     "Corre tres escenarios: (1) todos con igual prioridad, (2) prioridad "
     "ajustada antes de arrancar y (3) prioridad modificada mientras los "
     "procesos estan bloqueados.",
     "<vueltas>  Iteraciones que cada proceso ejecuta antes de finalizar.",
     "Usar valores a partir de 100000000 permite ver claramente como se "
     "ordenan segun la prioridad.",
     "test-prio 100000000"},
    {"test-sync", "Testea sincronizacion con y sin semaforos",
     (entry_point_t)test_sync, CAT_TESTS, "test-sync <n> <usar_sem>",
     "Lanza pares de procesos que incrementan y decrementan una variable "
     "compartida.",
     "<n>        Iteraciones por proceso.\n<usar_sem>  1 para usar "
     "semaforos, 0 para dejar la condicion de carrera.",
     "Con usar_sem=1 el valor final deberia ser 0.", "test-sync 1000 1"},
    {"mvar", "Simula productores y consumidores sobre una variable compartida",
     (entry_point_t)cmd_mvar, CAT_TESTS, "mvar <writers> <readers>",
     "Crea procesos escritores y lectores que se sincronizan con semaforos "
     "para compartir una MVar.",
     "<writers>  Cantidad de escritores (1-6).\n<readers>  Cantidad de "
     "lectores (1-6).",
     "Oculta el cursor mientras corre; usar Ctrl+C para abortar la demo.",
     "mvar 2 2"},
    {"loop", "Imprime su PID y duerme en un bucle infinito",
     (entry_point_t)cmd_loop, CAT_PROCESOS, "loop <segundos>",
     "Muestra el PID y duerme la cantidad indicada de segundos en un loop.",
     "<segundos>  Intervalo en segundos (entero positivo).",
     "Detenelo con Ctrl+C desde la shell.", "loop 2"},
    {"kill", "Mata los procesos indicados por PID", (entry_point_t)cmd_kill,
     CAT_PROCESOS, "kill <pid> <pid> ...",
     "Solicita al kernel que termine el proceso con el PID indicado.",
     "<pid>  Identificador del proceso (>1).",
     "No se puede matar init (0) ni la shell (1).", "kill 7"},
    {"nice", "Cambia la prioridad de un proceso", (entry_point_t)cmd_nice,
     CAT_PROCESOS, "nice <pid> <prioridad>",
     "Actualiza la prioridad que el scheduler utiliza para el proceso.",
     "<pid>        Identificador del proceso.\n<prioridad>  Nuevo valor "
     "(0-5).",
     "La nueva prioridad se respeta cuando el proceso vuelva a correr.",
     "nice 8 3"},
    {"block", "Detiene temporalmente un proceso", (entry_point_t)cmd_block,
     CAT_PROCESOS, "block <pid>",
     "Quita al proceso de la cola de listos y lo marca como BLOQUEADO.",
     "<pid>  Identificador del proceso.",
     "El proceso permanece detenido hasta ejecutar 'unblock <pid>'.",
     "block 6"},
    {"unblock", "Devuelve un proceso bloqueado a READY",
     (entry_point_t)cmd_unblock, CAT_PROCESOS, "unblock <pid>",
     "Reincorpora un proceso bloqueado a la cola de listos.",
     "<pid>  Identificador del proceso (debe estar bloqueado).",
     "Complemento de 'block': reanuda el proceso pausado.", "unblock 6"},
    {"cat", "Reproduce stdin en pantalla", (entry_point_t)cmd_cat, CAT_IO,
     "cat",
     "Lee desde la entrada estandar (teclado o pipe) y muestra el flujo por "
     "pantalla"
     " sin realizar modificaciones hasta recibir la senal de fin de archivo "
     "(Ctrl+D).",
     "No recibe parametros.",
     "Util para testear redirecciones y pipes. "
     "Permite verificar la comunicacion entre procesos.",
     "cat"},
    {"wc", "Cuenta lineas del texto ingresado", (entry_point_t)cmd_wc, CAT_IO,
     "wc", "Lee lineas desde stdin y al final informa cuantas se recibieron.",
     "No recibe parametros.", "Se detiene con Ctrl+D.", "wc"},
    {"filter", "Filtra vocales del texto ingresado", (entry_point_t)cmd_filter,
     CAT_IO, "filter",
     "Lee caracteres desde stdin y reimprime solo los que no son vocales.",
     "No recibe parametros.", "Termina con Ctrl+D.", "filter"}};

void run_shell() {
  puts(WELCOME);
  while (1) {
    printfc(PINK, ">");
    char raw_input[MAX_CHARS] = {0};
    scanf("%S", raw_input);
    input_parser_t *parser = parse_input(raw_input);
    if (parser == NULL) {
      printErr(INVALID_COMMAND);
      continue;
    }

    if (parser->qty_shell_programs == 1) {
      create_single_process(parser);
    } else if (parser->qty_shell_programs == 2) {
      create_piped_processes(parser);
    }
    free_parser(parser);
  }
}

static void create_single_process(input_parser_t *parser) {
  shell_program_t *program = parser->shell_programs[0];
  int idx = getCommandIndex(program->name);
  RETURN_IF_INVALID(idx);

  if (parser->background) {
    fd_t fds[2] = {DEV_NULL_FD, STDOUT};
    my_create_process((entry_point_t)commands[idx].f, program->params,
                      program->name, fds);
  } else {
    fd_t fds[2] = {STDIN, STDOUT}; // Usar descriptores estándar para foreground
    int64_t pid = my_create_process((entry_point_t)commands[idx].f,
                                    program->params, program->name, fds);
    my_wait_pid(pid, NULL);
  }
}

static void create_piped_processes(input_parser_t *parser) {
  shell_program_t *left_program = get_shell_program(parser, 0);
  int first_idx = getCommandIndex(left_program->name);
  RETURN_IF_INVALID(first_idx);

  shell_program_t *right_program = get_shell_program(parser, 1);
  int second_idx = getCommandIndex(right_program->name);
  RETURN_IF_INVALID(second_idx);

  fd_t pipe_fds[2];
  if (my_pipe_create(pipe_fds) == -1) {
    printErr("No se pudo crear el pipe\n");
    return;
  }

  fd_t left_fds[2] = {STDIN, pipe_fds[1]};
  fd_t right_fds[2] = {pipe_fds[0], STDOUT};

  int left_pid =
      my_create_process((entry_point_t)commands[first_idx].f,
                        left_program->params, left_program->name, left_fds);
  int right_pid =
      my_create_process((entry_point_t)commands[second_idx].f,
                        right_program->params, right_program->name, right_fds);

  if (left_pid == -1 || right_pid == -1) {
    printErr("No se pudo crear uno de los procesos\n");
    my_destroy_pipe(pipe_fds[0]);
    return;
  }

  if (parser->background) {
    my_adopt_child(left_pid);
    my_adopt_child(right_pid);
  } else {
    my_wait_pid(left_pid, NULL);
    my_wait_pid(right_pid, NULL);
    my_destroy_pipe(pipe_fds[0]);
  }
}

static const char *command_category(const Command *cmd) {
  if (cmd->category != NULL && cmd->category[0] != '\0') {
    return cmd->category;
  }
  return CAT_GENERAL;
}

static Color category_color(const char *category) {
  (void)category;
  return COLOR_SECTION;
}

static void print_block(const char *text) {
  if (text == NULL || *text == '\0') {
    return;
  }

  const char *cursor = text;
  while (*cursor != '\0') {
    printf("    ");

    while (*cursor != '\0' && *cursor != '\n') {
      putchar(*cursor);
      cursor++;
    }
    putchar('\n');

    if (*cursor == '\0') {
      break;
    }
    cursor++;
  }
}

static void print_section(const char *title, const char *text) {
  if (text == NULL || *text == '\0') {
    return;
  }
  printfc(COLOR_SECTION, "%s:\n", title);
  print_block(text);
  putchar('\n');
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
  printfc((Color){0, 255, 255}, "Comandos disponibles:\n\n");

  const char *current_category = NULL;
  for (int i = 0; i < QTY_COMMANDS; i++) {
    const char *category = command_category(&commands[i]);
    if (current_category == NULL || strcmp(current_category, category) != 0) {
      if (current_category != NULL) {
        putchar('\n');
      }
      current_category = category;
      printfc(category_color(current_category), "[%s]\n", current_category);
    }
    printfc(COLOR_USAGE, "  %s", commands[i].usage);
    if (commands[i].short_desc != NULL) {
      printfc(COLOR_DESC, " - %s", commands[i].short_desc);
    }
    putchar('\n');
  }

  printf("\nUse 'man <comando>' para obtener mas detalles.\n");
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
  if (idx == -1) {
    printErr(INVALID_COMMAND);
    return;
  }

  const Command *cmd = &commands[idx];
  const char *category = command_category(cmd);

  printfc(COLOR_SECTION, "Comando:   ");
  printf("%s\n\n", cmd->name);
  printfc(COLOR_SECTION, "Categoria: ");
  printf("%s\n\n", category);
  printfc(COLOR_SECTION, "Uso:       ");
  printf("%s\n\n", cmd->usage);

  print_section("Descripcion",
                (cmd->details != NULL) ? cmd->details : cmd->short_desc);
  print_section("Parametros", cmd->parameters);
  print_section("Notas", cmd->notes);
  print_section("Ejemplo", cmd->example);
}
