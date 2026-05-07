#ifndef WORD_COUNTER_H
#define WORD_COUNTER_H

#include <pthread.h>

#define MAX_WORD_LENGTH 64
#define LINE_BLOCK_SIZE 10000

#define CHAR_TO_INDEX(c) ((c) - 'a')

extern int adjacency_matrix[26][26];

typedef struct
{
    char **words;
    int count;
} word_block_t;

typedef struct
{
    int *total_count;
    char ***result_words;
    int *result_count;
    double *timing_count;
    pthread_mutex_t *result_mutex;
} consumer_args_t;

typedef struct
{
    word_block_t *block;
    consumer_args_t *args;
} task_data_t;

void init_qwerty_layout(void);
int can_type_word(const char *word);
void process_block(void *arg);

#endif
