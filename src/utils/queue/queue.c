#include "queue.h"

#include <stdlib.h>

static struct queue_item *create_queue_item(void *data)
{
    struct queue_item *i = malloc(sizeof(struct queue_item));
    if (i == NULL)
        return NULL;
    i->data = data;
    i->next = NULL;
    return i;
}

queue *queue_init(void)
{
    queue *q = malloc(sizeof(queue));
    if (q == NULL)
        return NULL;
    q->head = NULL;
    q->tail = NULL;
    return q;
}

void queue_free(queue *q)
{
    struct queue_item *curr = q->head;
    while (curr != NULL)
    {
        struct queue_item *next = curr->next;
        free(curr);
        curr = next;
    }
    free(q);
}

int queue_push(queue *q, void *data)
{
    struct queue_item *i = create_queue_item(data);
    if (i == NULL)
        return -1;

    if (q->head == NULL)
    {
        q->head = i;
        q->tail = i;
    }
    else
    {
        q->tail->next = i;
        q->tail = i;
    }
    return 0;
}

int queue_push_front(queue *q, void *data)
{
    struct queue_item *i = create_queue_item(data);
    if (i == NULL)
        return -1;

    if (q->head == NULL)
    {
        q->head = i;
        q->tail = i;
    }
    else
    {
        i->next = q->head;
        q->head = i;
    }
    return 0;
}

void *queue_pop(queue *q)
{
    if (q->head == NULL)
        return NULL;

    struct queue_item *item = q->head;
    void *data = item->data;

    q->head = item->next;
    if (q->head == NULL)
        q->tail = NULL;

    free(item);

    return data;
}

void *queue_peek(queue *q)
{
    if (q->head == NULL)
        return NULL;
    return q->head->data;
}

int queue_isempty(queue *q)
{
    return q->head == NULL;
}
