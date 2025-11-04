#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

int cmd_ps(int argc, char **argv);
int cmd_mem(int argc, char **argv);
int cmd_loop(int argc, char *argv[]);
int cmd_kill(int argc, char **argv);
int cmd_nice(int argc, char **argv);
int cmd_block(int argc, char **argv);
int cmd_unblock(int argc, char **argv);
int cmd_cat(int argc, char **argv);

#endif