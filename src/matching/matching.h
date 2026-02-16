#ifndef MATCHING_H
#define MATCHING_H

/**
 * @brief Checks if a word matches a shell pattern.
 * @param word The input word to test
 * @param pattern The pattern to match against
 * @return 1 if it matches, 0 otherwise
 */

int pattern_matches(char *word, char *pattern);

#endif /* ! MATCHING_H */
