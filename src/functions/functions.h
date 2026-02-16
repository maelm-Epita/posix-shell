#ifndef FUNCTIONS_H
#define FUNCTIONS_H

struct function
{
    struct ast_command *command;
};

void free_function(void *f);
struct function *create_function(struct ast_command *ast_command);

#endif /* ! FUNCTIONS_H */
