#ifndef STRING_METADATA_H
#define STRING_METADATA_H

#include <stddef.h>

#include "utils/vector/vector.h"

/**
 * @brief Quote types used for string metadata.
 */

enum quote_type
{
    Q_NOT_QUOTED,
    Q_SINGLE_QUOTE,
    Q_DOUBLE_QUOTE
};

/**
 * @brief A segment of a string with metadata.
 *
 * @param str Segment content
 * @param quote Quote type applied to this segment
 * @param escaped Non-zero if the segment was escaped
 */

struct string_segment
{
    char *str;
    enum quote_type quote;
    int escaped;
};

/**
 * @brief Metadata string type (vector of string_segment*).
 */

typedef vector *m_string;

/**
 * @brief Creates a string segment.
 * @param str Segment buffer
 * @param str_size Number of characters to use from str
 * @param type Quote type for the segment
 * @param escaped Non-zero if escaped
 * @return Newly allocated segment, or NULL on failure
 */

struct string_segment *create_segment(char *str, size_t str_size,
                                      enum quote_type type, int escaped);

/**
 * @brief Frees a string segment.
 * @param seg Segment to free
 */

void free_segment(struct string_segment *seg);

/**
 * @brief Frees a metadata string and its segments.
 * @param str Metadata string to free
 */

void free_m_string(m_string str);

/**
 * @brief Copies a metadata string.
 * @param in Metadata string to copy
 * @return New metadata string copy
 */

m_string copy_m_string(m_string in);

/**
 * @brief Returns the raw concatenated string (without metadata).
 * @param str Metadata string
 * @return Pointer to statically allocated raw string
 */

char *m_string_get_raw(m_string str);

m_string m_string_from_raw(char *str);

#endif /* ! STRING_METADATA_H */
