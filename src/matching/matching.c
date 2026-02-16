#include "matching.h"

static int match_range(char c, const char *p, int *consumed)
{
    int i = 0;
    int neg = 0;
    int ok = 0;

    if (p[i] == '!' || p[i] == '^')
    {
        neg = 1;
        i++;
    }
    if (p[i] == ']')
    {
        ok |= (c == ']');
        i++;
    }

    while (p[i] && p[i] != ']')
    {
        if (p[i + 1] == '-' && p[i + 2] && p[i + 2] != ']')
        {
            char a = p[i];
            char b = p[i + 2];
            if (a > b)
            {
                char t = a;
                a = b;
                b = t;
            }
            if (c >= a && c <= b)
                ok = 1;
            i += 3;
        }
        else
        {
            if (c == p[i])
                ok = 1;
            i++;
        }
    }

    if (p[i] == ']')
        i++;

    *consumed = i;
    return neg ? !ok : ok;
}

static int match_here(const char *w, const char *p)
{
    if (*p == '\0')
        return *w == '\0';

    if (*p == '*')
    {
        while (*p == '*')
            p++;
        if (*p == '\0')
            return 1;
        for (const char *ww = w; *ww; ww++)
            if (match_here(ww, p))
                return 1;
        return match_here(w + (w[0] ? 1 : 0), p);
    }

    if (*w == '\0')
        return 0;

    if (*p == '?')
        return match_here(w + 1, p + 1);

    if (*p == '[')
    {
        int consumed = 0;
        int ok = match_range(*w, p + 1, &consumed);
        if (consumed == 0)
            return (*w == '[') && match_here(w + 1, p + 1);
        return ok && match_here(w + 1, p + 1 + consumed);
    }

    return (*w == *p) && match_here(w + 1, p + 1);
}

int pattern_matches(char *word, char *pattern)
{
    if (!word || !pattern)
        return 0;
    return match_here(word, pattern);
}
