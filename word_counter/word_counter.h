#ifndef WORD_COUNTER_H
#define WORD_COUNTER_H

#include <pthread.h>

#define MAX_WORD_LENGTH 64
#define LINE_BLOCK_SIZE 10000

#define CHAR_TO_INDEX(c) ((c) - 'a')

/**
 * @brief Structure for storing a block of words
 * @param words - array of word strings
 * @param count - number of words in the block
 */
typedef struct
{
    char **words;
    int count;
} word_block_t;

/**
 * @brief Structure containing shared data for worker threads
 * @param total_count - pointer to total count of typable words
 * @param result_words - pointer to array of result words
 * @param result_count - pointer to current number of results
 * @param timing_count - pointer to accumulated processing time
 * @param result_mutex - mutex for synchronizing access to results
 */
typedef struct
{
    int *total_count;
    char ***result_words;
    int *result_count;
    double *timing_count;
    pthread_mutex_t *result_mutex;
} consumer_args_t;

/**
 * @brief Structure representing a task for the thread pool
 * @param block - block of words to process
 * @param args - consumer arguments shared between threads
 */
typedef struct
{
    word_block_t *block;
    consumer_args_t *args;
} task_data_t;

/**
 * @brief Initializes the QWERTY keyboard adjacency matrix
 */
void init_qwerty_layout(void);

/**
 * @brief Checks if a word can be typed using adjacent QWERTY keys
 * @param word - pointer to the word string to check
 * @return 1 if the word can be typed, 0 otherwise
 */
int can_type_word(const char *word);

/**
 * @brief Processes a block of words in a worker thread
 * @param arg - pointer to task_data_t structure containing block and arguments
 */
void process_block(void *arg);

/**
 * @brief Processes a block of words in a worker thread
 * @param arg - pointer to task_data_t structure containing block and arguments
 */
unsigned int is_adjacent(char a, char b);

#endif
