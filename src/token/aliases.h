#ifndef ALIASES_H
#define ALIASES_H

#include "shell/shell.h"
#include "token.h"
#include "utils/vector/vector.h"

struct alias
{
    char *raw_input;
    vector *tokens;
};

vector *perform_alias(struct token *tok, struct shell_state *state);
struct alias *create_alias(char *raw_input);
void free_alias(void *alias);
void print_alias(struct alias *alias);

#endif /* ! ALIASES_H */
