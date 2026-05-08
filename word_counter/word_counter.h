#ifndef WORD_COUNTER_H
#define WORD_COUNTER_H

#include <pthread.h>

#define MAX_WORD_LENGTH 64
#define LINE_BLOCK_SIZE 10000

#define CHAR_TO_INDEX(c) ((c) - 'a')

typedef struct
{
    char **words;
    int count;
} word_block_t;

typedef struct
{
    word_block_t *block;
    int *total_count;
    char ***result_words;
    int *result_count;
    double *timing_count;
    pthread_mutex_t *result_mutex;
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
 * @brief Checks if two characters are adjacent on QWERTY keyboard
 * @param a first character ('a'..'z')
 * @param b second character ('a'..'z')
 * @return 1 if adjacent, 0 otherwise
 */
unsigned int is_adjacent(char a, char b);

/**
 * @brief Checks if a word can be typed using adjacent QWERTY keys
 * @param word pointer to null-terminated word string
 * @return 1 if typable, 0 otherwise
 */
int can_type_word(const char *word);

/**
 * @brief Filters typable words from input block
 * @param input pointer to input word block (not modified)
 * @return pointer to new word_block_t containing only typable words.
 *         Caller must free the returned block and its words using
 *         free_word_block().
 * @note Returns NULL on allocation failure.
 */
word_block_t* filter_typable_words(const word_block_t *input);

/**
 * @brief Frees memory allocated for a word block
 * @param block pointer to word_block_t to free
 */
void free_word_block(word_block_t *block);

#endif
