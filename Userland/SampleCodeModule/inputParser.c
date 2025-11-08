// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "inputParser.h"
#include "stdlib.h"
#include "string.h"
#include <stddef.h>

#define PIPE_CHAR '|'
#define BACKGROUND_CHAR '&'

static shell_program_t *extract_command_info(char **command_line);
static void skip_whitespace(char **str);
static void free_program_memory(shell_program_t *prog);
static int allocate_argument(shell_program_t *cmd, int arg_index);
static int tokenize_arguments(shell_program_t *cmd, char **line_ptr);


input_parser_t *parse_input(char *command_line) {
  input_parser_t *result = (input_parser_t *)my_malloc(sizeof(input_parser_t));
  if (!result) {
    return NULL;
  }

  result->shell_programs = (shell_program_t **)my_malloc(
      sizeof(shell_program_t *) * MAX_CHAINED_PROGRAMS);

  if (!result->shell_programs) {
    my_free(result);
    return NULL;
  }

  int i;
  for (i = 0; i < MAX_CHAINED_PROGRAMS; i++) {
    result->shell_programs[i] = NULL;
  }
  result->qty_shell_programs = 1;
  result->background = 0;

  skip_whitespace(&command_line);

  if (*command_line == '\0' || *command_line == '\n') {
    result->qty_shell_programs = 0;
    return result;
  }

  shell_program_t *cmd_left = extract_command_info(&command_line);
  shell_program_t *cmd_right = NULL;

  if (*command_line == PIPE_CHAR) {
    command_line++;
    skip_whitespace(&command_line);
    cmd_right = extract_command_info(&command_line);
    if (cmd_right) {
      result->qty_shell_programs++;
    }
  }

  if (*command_line == BACKGROUND_CHAR) {
    result->background = 1;
  }

  result->shell_programs[0] = cmd_left;
  result->shell_programs[1] = cmd_right;

  return result;
}

static void skip_whitespace(char **str) {
  while (**str == ' ') {
    (*str)++;
  }
}

static shell_program_t *extract_command_info(char **line_ptr) {
  shell_program_t *cmd = (shell_program_t *)my_malloc(sizeof(shell_program_t));
  if (!cmd) {
    return NULL;
  }

  cmd->params = (char **)my_malloc(sizeof(char *) * (MAX_PARAM_LEN + 2));
  if (!cmd->params) {
    my_free(cmd);
    return NULL;
  }

  int j;
  for (j = 0; j < (MAX_PARAM_LEN + 2); j++) {
    cmd->params[j] = NULL;
  }

  cmd->name = (char *)my_malloc(MAX_PARAM_LEN);
  if (!cmd->name) {
    my_free(cmd->params);
    my_free(cmd);
    return NULL;
  }

  int bytes_copied = strcpychar_n(cmd->name, *line_ptr, ' ', MAX_COMMAND_LEN);
  *line_ptr += bytes_copied;
  skip_whitespace(line_ptr);

  if (!allocate_argument(cmd, 0)) {
    free_program_memory(cmd);
    return NULL;
  }
  strcpy(cmd->params[0], cmd->name);

  int total_args = tokenize_arguments(cmd, line_ptr);
  if (total_args < 0) {
    free_program_memory(cmd);
    return NULL;
  }

  return cmd;
}

static int allocate_argument(shell_program_t *cmd, int arg_index) {
  cmd->params[arg_index] = my_malloc(MAX_PARAM_LEN);
  return (cmd->params[arg_index] != NULL);
}

static int tokenize_arguments(shell_program_t *cmd, char **line_ptr) {
  int arg_count = 1;
  int bytes_read = 1;

  while (bytes_read > 0 && **line_ptr != PIPE_CHAR &&
         **line_ptr != BACKGROUND_CHAR && **line_ptr != '\0' &&
         **line_ptr != '\n') {

    if (!allocate_argument(cmd, arg_count)) {
      return -1;
    }

    bytes_read =
        strcpychar_n(cmd->params[arg_count], *line_ptr, ' ', MAX_PARAM_LEN);

    if (bytes_read > 0) {
      arg_count++;
      *line_ptr += bytes_read;
      skip_whitespace(line_ptr);
    } else {
      my_free(cmd->params[arg_count]);
      cmd->params[arg_count] = NULL;
    }
  }

  cmd->params[arg_count] = NULL;
  return arg_count;
}

static void free_program_memory(shell_program_t *prog) {
  if (!prog) {
    return;
  }

  int i = 0;
  while (prog->params && prog->params[i] != NULL) {
    my_free(prog->params[i]);
    i++;
  }

  if (prog->params) {
    my_free(prog->params);
  }
  if (prog->name) {
    my_free(prog->name);
  }
  my_free(prog);
}

void free_parser(input_parser_t *parser) {
  if (!parser) {
    return;
  }

  if (parser->shell_programs) {
    int i;
    for (i = 0; i < MAX_CHAINED_PROGRAMS; i++) {
      if (parser->shell_programs[i] != NULL) {
        free_program_memory(parser->shell_programs[i]);
      }
    }
    my_free(parser->shell_programs);
  }
  my_free(parser);
}

shell_program_t *get_shell_program(input_parser_t *parser, int index) {
    if (parser == NULL || parser->shell_programs == NULL) {
        return NULL;
    }
    
    if (index < 0 || index >= MAX_CHAINED_PROGRAMS) {
        return NULL;
    }
    
    return parser->shell_programs[index];
}
