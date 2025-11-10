#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H
#include <stdint.h>

#define MAX_CHAINED_PROGRAMS 2
#define MAX_PARAM_LEN 32
#define MAX_COMMAND_LEN 32

typedef struct shell_program {
  char *name;
  char **params;
} shell_program_t;

typedef struct input_parser {
  shell_program_t **shell_programs;
  uint8_t qty_shell_programs;
  uint8_t background;
} input_parser_t;

input_parser_t * parse_input(char * input);
void free_parser(input_parser_t * parser);
shell_program_t *get_shell_program(input_parser_t *parser, int index);

#endif // INPUT_PARSER_H