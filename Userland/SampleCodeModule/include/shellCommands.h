#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#define WRONG_PARAM_0 "Este comando no recibe parametros.\n"
#define WRONG_PARAM_1                                                          \
  "Cantidad de parametros incorrecta. Se espera 1 parametro.\n"
#define WRONG_PARAM_2                                                          \
  "Cantidad de parametros incorrecta. Se esperan 2 parametros.\n"

int cmd_ps(int argc, char **argv);
int cmd_mem(int argc, char **argv);
int cmd_loop(int argc, char *argv[]);
int cmd_kill(int argc, char **argv);
int cmd_nice(int argc, char **argv);
int cmd_block(int argc, char **argv);
int cmd_unblock(int argc, char **argv);
int cmd_cat(int argc, char **argv);
int cmd_wc(int argc, char **argv);
int cmd_filter(int argc, char **argv);
int cmd_mvar(int argc, char *argv[]);

#endif
