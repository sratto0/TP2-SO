// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include <memory.h>
#include <stdint.h>
#include <video.h>

#define ZERO_EXCEPTION_ID 0
#define OPCODE_EXCEPTION_ID 6
#define GENERAL_PROTECTION_ID 13
#define PAGE_FAULT_ID 14

static void printError(const char *msg, uint64_t rip, uint64_t *rsp);

static const char *ZERO_EXCEPTION_DESCRIPTION = "Division por cero";
static const char *OPCODE_EXCEPTION_DESCRIPTION = "Operacion invalida";
static const char *GENERAL_PROTECTION_DESCRIPTION = "Proteccion general";
static const char *PAGE_FAULT_DESCRIPTION = "Fallo de pagina";

void exceptionDispatcher(int ex, uint64_t *rip, uint64_t *rsp) {
  const char *msg = "Excepcion desconocida";

  switch (ex) {
  case ZERO_EXCEPTION_ID:
    msg = ZERO_EXCEPTION_DESCRIPTION;
    break;
  case OPCODE_EXCEPTION_ID:
    msg = OPCODE_EXCEPTION_DESCRIPTION;
    break;
  case GENERAL_PROTECTION_ID:
    msg = GENERAL_PROTECTION_DESCRIPTION;
    break;
  case PAGE_FAULT_ID:
    msg = PAGE_FAULT_DESCRIPTION;
    break;
  default:
    break;
  }

  printError(msg, *rip, rsp);
}

static void printError(const char *msg, uint64_t rip, uint64_t *rsp) {
  setFontColor(ERROR_COLOR);
  printf("Error: %s\n\n", msg);
  dump(rip, *rsp, rsp - 4);
  setFontColor((Color)DEFAULT_COLOR);
}