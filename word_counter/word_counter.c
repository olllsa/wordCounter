#include "word_counter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

int adjacency_matrix[26][26];

/**
 * @brief Initializes the QWERTY keyboard adjacency matrix
 *
 * Creates a 26x26 adjacency matrix where cell [i][j] = 1 if keys i and j
 * are adjacent on a standard QWERTY keyboard layout. Adjacency includes:
 * - Horizontal neighbors within the same row
 * - Vertical neighbors between rows
 * - Self-adjacency (diagonal set to 1)
 *
 * Row layout:
 *   Row 0: qwertyuiop
 *   Row 1: asdfghjkl
 *   Row 2: zxcvbnm
 */
void init_qwerty_layout(void)
{
    char *rows[] =
    {
        "qwertyuiop",
        "asdfghjkl",
        "zxcvbnm"
    };

    memset(adjacency_matrix, 0, sizeof(adjacency_matrix));

    // Set horizontal adjacencies within each row
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; rows[r][c+1] != '\0'; c++)
        {
            int idx1 = CHAR_TO_INDEX(rows[r][c]);
            int idx2 = CHAR_TO_INDEX(rows[r][c+1]);
            adjacency_matrix[idx1][idx2] = 1;
            adjacency_matrix[idx2][idx1] = 1;
        }
    }

    // Set vertical adjacencies between row 0 and row 1
    char *row1 = rows[0];
    char *row2 = rows[1];
    for (int c = 0; row1[c] != '\0' && row2[c] != '\0'; c++)
    {
        int idx1 = CHAR_TO_INDEX(row1[c]);
        int idx2 = CHAR_TO_INDEX(row2[c]);
        adjacency_matrix[idx1][idx2] = 1;
        adjacency_matrix[idx2][idx1] = 1;
    }

    // Set vertical adjacencies between row 1 and row 2
    char *row3 = rows[2];
    for (int c = 0; row2[c] != '\0' && row3[c] != '\0'; c++)
    {
        int idx1 = CHAR_TO_INDEX(row2[c]);
        int idx2 = CHAR_TO_INDEX(row3[c]);
        adjacency_matrix[idx1][idx2] = 1;
        adjacency_matrix[idx2][idx1] = 1;
    }

    // Set self-adjacency (each key is adjacent to itself)
    for (int i = 0; i < 26; i++)
    {
        adjacency_matrix[i][i] = 1;
    }
}

/**
 * @brief Checks if a word can be typed using adjacent QWERTY keys
 *
 * A word is typable if for every pair of consecutive letters, the corresponding
 * keys are adjacent on a QWERTY keyboard. Words of length 1 or less are rejected.
 * Only lowercase English letters 'a' to 'z' are supported.
 *
 * @param word - pointer to the null-terminated word string
 * @return 1 if the word can be typed on QWERTY using adjacent keys, 0 otherwise
 */
int can_type_word(const char *word)
{
    int len = strlen(word);

    if (len <= 1) return 0;

    for (int i = 1; i < len; i++)
    {
        int prev = CHAR_TO_INDEX(word[i-1]);
        int curr = CHAR_TO_INDEX(word[i]);

        if (prev < 0 || prev > 25 || curr < 0 || curr > 25)
        {
            return 0;
        }

        if (!adjacency_matrix[prev][curr])
        {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Processes a block of words in a worker thread
 *
 * This function is executed by worker threads from the thread pool.
 * It filters typable words from the given block, measures processing time,
 * and safely merges local results into the global result array using a mutex.
 * All allocated memory for the block and task is freed after processing.
 *
 * @param arg - pointer to task_data_t containing the word block and consumer arguments
 */
void process_block(void *arg)
{
    task_data_t *task_data = (task_data_t *)arg;

    word_block_t *block = task_data->block;
    consumer_args_t *args = task_data->args;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    char **local_results = malloc(block->count * sizeof(char*));

    int local_word_count = 0;

    for (int i = 0; i < block->count; i++)
    {
        if (can_type_word(block->words[i]))
        {
            local_results[local_word_count] = strdup(block->words[i]);
            local_word_count++;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    pthread_mutex_lock(args->result_mutex);
    if (local_word_count > 0)
    {
        int new_total = *(args->result_count) + local_word_count;
        char **new_result_words = realloc(*(args->result_words), new_total * sizeof(char*));
        if (new_result_words)
        {
            *(args->result_words) = new_result_words;
            for (int i = 0; i < local_word_count; i++)
            {
                (*(args->result_words))[*(args->result_count) + i] = local_results[i];
            }
            *(args->result_count) = new_total;
            *(args->total_count) += local_word_count;
            *(args->timing_count) += elapsed;
        }
    }
    else
    {
        free(local_results);
    }
    pthread_mutex_unlock(args->result_mutex);

    for (int i = 0; i < block->count; i++)
    {
        free(block->words[i]);
    }
    free(block->words);
    free(block);
    free(task_data);
}
