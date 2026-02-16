#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>

struct queue_item
{
    void *data;
    struct queue_item *next;
};

/**
 ** @brief              Generic queue datatype
 ** @param head         The head of the queue
 ** @param tail         The tail of the queue
 */
typedef struct
{
    struct queue_item *head;
    struct queue_item *tail;
} queue;

/**
 ** @brief              Creates an empty queue
 ** @param void
 ** @return             A pointer to the created queue on success, NULL on
 * failure
 */
queue *queue_init(void);

/**
 ** @brief              Frees a queue and all data inside of it
 ** @param q            The queue to free
 ** @return             void
 */
void queue_free(queue *q);

/**
 ** @brief              Pushes data into the queue's tail
 ** @param q            The queue to push into
 ** @param data         The data to push
 ** @return             Returns 0 on success, -1 on failure
 ** @safety             This function takes ownership of data,
                        meaning it does not copy the value, and it is the
 queue's job to free the data
 */
int queue_push(queue *q, void *data);

int queue_push_front(queue *q, void *data);

/**
 ** @brief              Pops data from the queue's head
 ** @param q            The queue to pop from
 ** @return             Returns a pointer to the head's data if the queue was
 * not empty, NULL if it was
 ** @safety             This function returns ownership of the data
 */
void *queue_pop(queue *q);

/**
 ** @brief              Peeks data from the queue's head (returns it without
 * removing it)
 ** @param q            The queue to peek from
 ** @return             Returns a pointer to the head's data if the queue was
 * not empty, NULL if it was
 ** @safety             This function does not give ownership of the data, the
 * queue still owns the data
 */
void *queue_peek(queue *q);

/**
 ** @brief              Checks if a queue is empty
 ** @param q            The queue to check
 ** @return             Returns 1 if the queue is empty, 0 if the queue is not
 * empty
 */
int queue_isempty(queue *q);

#endif /* ! QUEUE_H */
