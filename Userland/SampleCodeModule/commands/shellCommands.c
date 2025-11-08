// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "../include/shellCommands.h"
#include "../include/stdio.h"
#include "../include/stdlib.h"
#include <stddef.h>

int cmd_ps(int argc, char **argv) {
  if (argc != 1) {
    printErr(WRONG_PARAM_0);
    return -1;
  }
  process_info_t *info = my_get_processes_info();
  if (info == NULL) {
    printErr("ps: no data\n");
    return -1;
  }

  int count = 0;
  for (process_info_t *p = info; p->pid != NO_PID; p++) {
    count++;
  }

  printf("Los %d procesos en ejecucion son:\n\n", count);

  for (process_info_t *p = info; p->pid != NO_PID; p++) {
    const char *estado = (p->state == PROC_RUNNING || p->state == PROC_READY)
                             ? "Activo"
                         : (p->state == PROC_BLOCKED) ? "Bloqueado"
                                                      : "Terminado";
    const char *ground = p->foreground ? "Foreground" : "Background";

    printf("PID: %d\n", (int)p->pid);
    printf("Name: %s\n", p->name);
    printf("Estado: %s\n", estado);
    printf("Priority: %d\n", p->priority);
    printf("Ground: %s\n", ground);
    printf("StackPos: %d\n", (unsigned long long)p->stack_pointer);
    printf("StackBase: %d\n", (unsigned long long)p->stack_base);
    printf("RIP: %d\n", (unsigned long long)p->rip);
    printf("\n");
  }
  my_free(info);
  return 0;
}

int cmd_mem(int argc, char **argv) {
  if (argc != 1) {
    printErr(WRONG_PARAM_0);
    return -1;
  }
  memory_info_t *info = my_memory_get_info();
  if (info == NULL) {
    printErr("mem: no data\n");
    return -1;
  }

  printf("Total Memory: %d bytes\n", (unsigned long long)info->size);
  printf("Used Memory:  %d bytes\n", (unsigned long long)info->used);
  printf("Free Memory:  %d bytes\n", (unsigned long long)info->free);
  return 0;
}

int cmd_loop(int argc, char *argv[]) {
  if (argc != 2) {
    printErr("Necesita un argumento numerico que representa la cantidad de "
           "segundos\n");
    return -1;
  }
  int seconds = atoi(argv[1]);
  if (seconds <= 0) {
    printErr("El argumento debe ser un numero positivo\n");
    return -1;
  }
  int64_t pid = my_getpid();
  while (1) {
    printf("Buenas, soy el proceso %d y voy a dormir por %d segundos en un "
           "loop infinito\n",
           (int)pid, seconds);
    my_sleep(seconds);
  }
  return 0;
}

int cmd_kill(int argc, char **argv) {
  if (argc != 2) {
    printErr(WRONG_PARAM_1);
    return -1;
  }
  int pid = atoi(argv[1]);
  if (pid <= 1) {
    printErr("El PID debe ser un numero mayor a 1\n");
    return -1;
  }
  process_info_t *info = my_get_processes_info();
  if (info != NULL) {
    int found = 0;
    for (process_info_t *p = info; p->pid != NO_PID; p++) {
      if (p->pid == pid) {
        found = 1;
        break;
      }
    }
    if (!found) {
      printErr("No existe un proceso con ese PID\n");
      return -1;
    }
  }
  int result = my_kill(pid);

  if (result == -1) {
    printErr("Error al matar el proceso. Verifique que el PID sea correcto.\n");
    return -1;
  } else if (result == 0) {
    printf("Proceso %d matado exitosamente\n", pid);
    return 0;
  } else {
    printErr("my_kill devolvió código inesperado\n");
    return -1;
  }
}

int cmd_nice(int argc, char **argv) {
  if (argc != 3) {
    printErr(WRONG_PARAM_2);
    return -1;
  }
  int pid = atoi(argv[1]);
  int new_priority = atoi(argv[2]);
  if (pid <= 0 || new_priority < 0) {
    printErr("El PID debe ser un numero positivo y la prioridad un numero no "
           "negativo\n");
    return -1;
  }
  int result = my_nice(pid, new_priority);
  if (result == -1) {
    printErr(
        "Error al cambiar la prioridad. Verifique que el PID sea correcto.\n");
    return -1;
  }
  printf("Prioridad del proceso %d cambiada a %d exitosamente\n", pid,
         new_priority);
  return 0;
}

int cmd_block(int argc, char **argv) {
  if (argc != 2) {
    printErr(WRONG_PARAM_1);
    return -1;
  }
  int pid = atoi(argv[1]);
  if (pid <= 1) {
    printErr("El PID debe ser un numero mayor a 1\n");
    return -1;
  }
  process_info_t *info = my_get_processes_info();
  if (info != NULL) {
    int found = 0;
    for (process_info_t *p = info; p->pid != NO_PID; p++) {
      if (p->pid == pid) {
        found = 1;
        break;
      }
    }
    if (!found) {
      printErr("No existe un proceso con ese PID\n");
      return -1;
    }
  }

  int result = my_block_process(pid);
  if (result == -1) {
    printErr("Error al bloquear/desbloquear el proceso. Verifique que el PID sea correcto.\n");
    return -1;
  } else if (result == 0) {
    printf("Proceso %d bloqueado/desbloqueado exitosamente\n", pid);
    return 0;
  } else {
    printErr("my_block devolvió código inesperado\n");
    return -1;
  }
}

int cmd_unblock(int argc, char **argv) {
  if (argc != 2) {
    printErr(WRONG_PARAM_1);
    return -1;
  }
  int pid = atoi(argv[1]);
  if (pid <= 1) {
    printErr("El PID debe ser un numero mayor a 1\n");
    return -1;
  }
  process_info_t *info = my_get_processes_info();
  if (info != NULL) {
    int found = 0;
    for (process_info_t *p = info; p->pid != NO_PID; p++) {
      if (p->pid == pid) {
        found = 1;
        break;
      }
    }
    if (!found) {
      printErr("No existe un proceso con ese PID\n");
      return -1;
    }
  }

  int result = my_unblock_process(pid);
  if (result == -1) {
    printErr("Error al desbloquear el proceso. Verifique que el PID sea correcto.\n");
    return -1;
  } else if (result == 0) {
    printf("Proceso %d desbloqueado exitosamente\n", pid);
    return 0;
  } else {
    printErr("my_unblock devolvió código inesperado\n");
    return -1;
  }
}

int cmd_cat(int argc, char **argv) {
  if (argc != 1) {
    printErr(WRONG_PARAM_0);
    return -1;
  }

  printf("cat: leyendo desde stdin (Ctrl+D para terminar)\n");

  char c;
  while (1) {
    c = getchar();

    if (c == -1 || c == 0x04) {
      printf("\n");
      break;
    }

    if (c == '\n') {
      printf("\n");
      continue;
    }

    if (c == '\b') {
      printf("\b \b");
      continue;
    }

    printf("%c", c);
  }

  return 0;
}

int cmd_filter(int argc, char **argv) {
  if (argc != 1) {
    printErr(WRONG_PARAM_0);
    return -1;
  }

  printf("filter: escriba texto (Ctrl+D para terminar)\n");

  while (1) {
    char c = getchar();

    if (c == -1 || c == 0x04) {
      printf("\n");
      break;
    }

    if (c == '\n') {
      printf("\n");
      continue;
    }

    if (c == '\b') {
      printf("\b \b");
      continue;
    }

    if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' && c != 'A' &&
        c != 'E' && c != 'I' && c != 'O' && c != 'U') {
      printf("%c", c);
    }
  }

  return 0;
}

int cmd_wc(int argc, char **argv) {
  if (argc != 1) {
    printErr(WRONG_PARAM_0);
    return -1;
  }

  printf("wc: escriba texto (Ctrl+D para terminar)\n");

  int line_count = 0;
  int has_content = 0;

  while (1) {
    char c = getchar();

    if (c == -1 || c == 0x04) {
      if (has_content) {
        line_count++;
      }
      printf("\n");
      break;
    }

    if (c == '\n') {
      line_count++;
      has_content = 0;
      printf("\n");
      continue;
    }

    if (c == '\b') {
      printf("\b \b");
      continue;
    }

    if (c >= 32 && c <= 126) {
      printf("%c", c);
      has_content = 1;
    }
  }

  printf("Cantidad de lineas: %d\n", line_count);
  return 0;
}
