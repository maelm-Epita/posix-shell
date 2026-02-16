#include "string_helpers.h"

int string_to_int(char *string)
{
    int res = 0;
    for (int i = 0; string[i] != 0; i++)
    {
        if (string[i] > '9' || string[i] < '0')
            return -1;
        res *= 10;
        res += string[i] - '0';
    }
    return res;
}

int string_to_int_plus_minus(char *string, int *out)
{
    int res = 0;
    if (string[0] == '+' || string[0] == '-')
        res = string_to_int(string + 1);
    else
        res = string_to_int(string);
    if (res == -1)
        return -1;
    if (string[0] == '-')
        res *= -1;
    *out = res;
    return 0;
}
