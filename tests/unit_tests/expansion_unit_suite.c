#include "utils/vector/vector_extra.h"
#define _POSIX_C_SOURCE 200809L
#include <criterion/criterion.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expansion/expansion.h"
#include "utils/hash_map/hash_map.h"
#include "variables/variables.h"

struct shell_state state;

void setup_expansion(void)
{
    state.last_exit_code = 0;
    state.args = NULL;
    state.variables = hash_map_init();
}

m_string create_input(char *in)
{
    struct string_segment *seg =
        create_segment(in, strlen(in), Q_NOT_QUOTED, 0);
    m_string str = vector_init(sizeof(struct string_segment *));
    vector_append(str, &seg);
    return str;
}

int add_variable_dup(char *key, char *value, int exported,
                     struct hash_map *variables)
{
    char *_key = strdup(key);
    char *_value = strdup(value);
    return add_variable(_key, _value, exported, variables);
}

TestSuite(expansion_tests, .init = setup_expansion);

Test(expansion_tests, add_variable_simple)
{
    add_variable_dup("TEST_VAR", "valeur123", 1, state.variables);

    m_string input = create_input("$TEST_VAR");
    vector *output = expand(input, &state, NULL);
    char *actual = vector_get_pointer(output, 0);
    cr_assert(output->size == 1);
    cr_assert_str_eq(actual, "valeur123");
    vector_free_pointer(output);
    free_m_string(input);
    hash_map_free(state.variables, free_variable);
}

Test(expansion_tests, envp_conversion)
{
    char *fake_envp[] = { "HOME=/home/utilisateur", "PATH=/bin:/usr/bin",
                          "VIDE=", NULL };

    cr_assert_eq(envp_to_variables(fake_envp, state.variables), 0);

    m_string input1 = create_input("$HOME");
    vector *out1 = expand(input1, &state, NULL);
    char *actual1 = vector_get_pointer(out1, 0);
    cr_assert(out1->size == 1);
    cr_assert_str_eq(actual1, "/home/utilisateur");
    vector_free_pointer(out1);
    free_m_string(input1);

    m_string input2 = create_input("$VIDE");
    vector *out2 = expand(input2, &state, NULL);
    cr_assert(out2->size == 0);
    vector_free_pointer(out2);
    free_m_string(input2);

    hash_map_free(state.variables, free_variable);
}

Test(expansion_tests, expand_simple)
{
    add_variable_dup("UTILISATEUR", "testuser", 1, state.variables);

    m_string in = create_input("Bonjour $UTILISATEUR !");
    vector *out = expand(in, &state, NULL);
    char *actual = vector_get_pointer(out, 1);
    cr_assert(out->size == 3);
    cr_assert_str_eq(actual, "testuser");
    vector_free_pointer(out);
    free_m_string(in);
    hash_map_free(state.variables, free_variable);
}

Test(expansion_tests, expand_braces)
{
    add_variable_dup("ACTION", "cod", 1, state.variables);

    m_string in = create_input("Je suis en train de ${ACTION}er");
    vector *out = expand(in, &state, NULL);
    char *actual = vector_get_pointer(out, 5);
    cr_assert(out->size == 6);
    cr_assert_str_eq(actual, "coder");
    vector_free_pointer(out);
    free_m_string(in);
    hash_map_free(state.variables, free_variable);
}

Test(expansion_tests, expand_undefined)
{
    m_string in = create_input("Voici $RIEN :");
    vector *out = expand(in, &state, NULL);
    cr_assert(out->size == 2);
    vector_free_pointer(out);
    free_m_string(in);
    hash_map_free(state.variables, free_variable);
}

Test(expansion_tests, expand_multiple)
{
    add_variable_dup("A", "1", 1, state.variables);
    add_variable_dup("B", "2", 1, state.variables);

    m_string in = create_input("$A + $B = 3");
    vector *out = expand(in, &state, NULL);
    char *actual1 = vector_get_pointer(out, 0);
    char *actual2 = vector_get_pointer(out, 2);
    cr_assert(out->size == 5);
    cr_assert_str_eq(actual1, "1");
    cr_assert_str_eq(actual2, "2");
    vector_free_pointer(out);
    free_m_string(in);
    hash_map_free(state.variables, free_variable);
}

Test(expansion_tests, expand_special_exit_code)
{
    state.last_exit_code = 42;

    m_string in = create_input("Code de sortie : $?");
    vector *out = expand(in, &state, NULL);
    char *actual = vector_get_pointer(out, 4);
    cr_assert(out->size == 5);
    cr_assert_str_eq(actual, "42");
    vector_free_pointer(out);
    free_m_string(in);
    hash_map_free(state.variables, free_variable);
}

Test(expansion_tests, expand_special_pid)
{
    m_string in = create_input("$$");
    vector *out = expand(in, &state, NULL);
    char *actual = vector_get_pointer(out, 0);

    cr_assert(out->size == 1);
    cr_assert_neq(strstr(actual, "$$"), actual, "$$ devrait être etendu");
    vector_free_pointer(out);
    free_m_string(in);
    hash_map_free(state.variables, free_variable);
}

Test(expansion_tests, expand_no_expansion)
{
    char *input = "Juste une chaîne normale";
    m_string in = create_input(input);
    vector *out = expand(in, &state, NULL);
    cr_assert(out->size == 4);
    vector_free_pointer(out);
    free_m_string(in);
    hash_map_free(state.variables, free_variable);
}

Test(expansion_tests, expand_escaping)
{
    add_variable_dup("VAR", "etendu", 1, state.variables);

    m_string in = create_input("Ceci est \\$VAR");
    vector *out = expand(in, &state, NULL);
    char *actual = vector_get_pointer(out, 2);
    cr_assert(out->size == 3);
    cr_assert_str_eq(actual, "\\etendu");
    vector_free_pointer(out);
    free_m_string(in);
    hash_map_free(state.variables, free_variable);
}

Test(expansion_tests, expand_complex_mix)
{
    add_variable_dup("HOME", "/root", 1, state.variables);
    add_variable_dup("USER", "admin", 1, state.variables);

    m_string in = create_input("Le dossier de ${USER} est $HOME/dir");
    vector *out = expand(in, &state, NULL);
    char *actual1 = vector_get_pointer(out, 3);
    char *actual2 = vector_get_pointer(out, 5);
    cr_assert(out->size == 6);
    cr_assert_str_eq(actual1, "admin");
    cr_assert_str_eq(actual2, "/root/dir");
    vector_free_pointer(out);
    free_m_string(in);
    hash_map_free(state.variables, free_variable);
}
