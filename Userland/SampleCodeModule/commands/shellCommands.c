#include "../include/shellCommands.h"
#include "../include/stdlib.h"
#include "../include/stdio.h"
#include <stddef.h>



int cmd_ps(int argc, char **argv) {
    if (argc != 1) {
        printf("ps: usage: ps\n");
        return -1;
    }
    process_info_t *info = my_get_processes_info();
    if (info == NULL) {
        printf("ps: no data\n");
        return -1;
    }

    int count = 0;
    for (process_info_t *p = info; p->pid != NO_PID; p++) {
        count++;
    }

    printf("Los %d procesos en ejecucion son:\n\n", count);

    for (process_info_t *p = info; p->pid != NO_PID; p++) {
        const char *estado = (p->state == PROC_RUNNING || p->state == PROC_READY) ? "Activo" : 
                            (p->state == PROC_BLOCKED) ? "Bloqueado" : "Terminado";
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
    return 0;
}

int cmd_mem(int argc, char **argv) {
    if (argc != 1) {
        printf("mem: usage: mem\n");
        return -1;
    }
    memory_info_t *info = my_memory_get_info();
    if (info == NULL) {
        printf("mem: no data\n");
        return -1;
    }

    printf("Total Memory: %d bytes\n", (unsigned long long)info->size);
    printf("Used Memory:  %d bytes\n", (unsigned long long)info->used);
    printf("Free Memory:  %d bytes\n", (unsigned long long)info->free);
    return 0;
}

int cmd_loop(int argc, char *argv[]){
    if(argc != 2){
        printf("Necesita un argumento numerico que representa la cantidad de segundos\n");
        return -1;
    }
    int seconds = atoi(argv[1]);
    if(seconds <= 0){
        printf("El argumento debe ser un numero positivo\n");
        return -1;
    }
    int64_t pid = my_getpid();
    while (1) {
        printf("Buenas, soy el proceso %d y voy a dormir por %d segundos en un loop infinito\n", (int)pid, seconds);
        my_sleep(seconds);
    }
    return 0;
}

int cmd_kill(int argc, char **argv){
    if(argc != 2){
        printf("Necesita un argumento: el PID del proceso\n");
        return -1;
    }
    int pid = atoi(argv[1]);
    if(pid <= 1){
        printf("El PID debe ser un numero mayor a 1\n");
        return -1;
    }
    process_info_t *info = my_get_processes_info();
    if(info != NULL){
        int found = 0;
        for (process_info_t *p = info; p->pid != NO_PID; p++) {
            if(p->pid == pid){
                found = 1;
                break;
            }
        }
        if(!found){
            printf("No existe un proceso con PID %d\n", pid);
            return -1;
        }
    }else {
        printf("DEBUG: my_get_processes_info() devolvió NULL, intentaré matar igual\n");
    }

    printf("DEBUG: llamando a my_kill(%d)\n", pid);
    int result = my_kill(pid);
    printf("DEBUG: my_kill retornó %d\n", result);

    if(result == -1){
        printf("Error al matar el proceso. Verifique que el PID sea correcto.\n");
        return -1;
    } else if (result == 0) {
        printf("Proceso %d matado exitosamente\n", pid);
        return 0;
    } else {
        printf("my_kill devolvió código inesperado %d\n", result);
        return -1;
    }
}

int cmd_nice(int argc, char **argv){
    if(argc != 3){
        printf("Necesita dos argumentos: el PID del proceso y la nueva prioridad\n");
        return -1;
    }
    int pid = atoi(argv[1]);
    int new_priority = atoi(argv[2]);
    if(pid <= 0 || new_priority < 0){
        printf("El PID debe ser un numero positivo y la prioridad un numero no negativo\n");
        return -1;
    }
    int result = my_nice(pid, new_priority);
    if(result == -1){
        printf("Error al cambiar la prioridad. Verifique que el PID sea correcto.\n");
        return -1;
    }
    printf("Prioridad del proceso %d cambiada a %d exitosamente\n", pid, new_priority);
    return 0;
}