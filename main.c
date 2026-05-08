#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "thpool/thpool.h"
#include "word_counter/word_counter.h"

/**
 * @brief Worker function executed by each thread in the pool.
 *
 * Receives a task (task_data_t) containing a word block and pointers to shared data.
 *
 * @param arg Pointer to a task_data_t allocated in main before adding the task to the thread pool.
 *
 * @note The function assumes that the input word block and all its strings are allocated via malloc/strdup.
 * @note After calling filter_typable_words(), the strings from the original block are freed.
 * @note If realloc of the global result array fails, the filtered block is completely freed,
 *       but previously accumulated results remain intact.
 *
 * @warning This function is intended for use only within the thread pool (thpool) context.
 *          The result mutex must be initialized before any call.
 */
static void thread_work(void *arg)
{
    task_data_t *task = (task_data_t*)arg;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    word_block_t *filtered = filter_typable_words(task->block);
    free_word_block(task->block);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    if (filtered && filtered->count > 0)
    {
        pthread_mutex_lock(task->result_mutex);
        int new_total = *(task->result_count) + filtered->count;
        char **new_words = realloc(*(task->result_words), new_total * sizeof(char*));
        if (new_words)
        {
            *(task->result_words) = new_words;
            memcpy(new_words + *(task->result_count), filtered->words, filtered->count * sizeof(char*));
            *(task->result_count) = new_total;
            *(task->timing_count) += elapsed;
            free(filtered->words);
            free(filtered);
        } else
        {
            // realloc failed: free filtered block to avoid leak
            free_word_block(filtered);
        }
        pthread_mutex_unlock(task->result_mutex);
    } else if (filtered)
    {
        free_word_block(filtered);
    }

    free(task);
}

/**
 * @brief Main function - reads dictionary and finds typable words
 *
 * Program workflow:
 * 1. Initialize QWERTY adjacency matrix
 * 2. Create thread pool with number of threads equal to CPU cores
 * 3. Read dictionary file in blocks of LINE_BLOCK_SIZE words
 * 4. Submit each block as a task to the thread pool
 * 5. Wait for all tasks to complete
 * 6. Display results: total processing time, count of typable words,
 *    and list of typable words (first 100 if more than 100)
 *
 * @param argc - argument count
 * @param argv - argument vector (argv[1] = dictionary file path)
 * @return 0 on success, 1 on error
 */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <dictionary_file>\n", argv[0]);
        return 1;
    }

    init_qwerty_layout();

    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores < 1) num_cores = 1;

    printf("Using %d worker threads.\n", num_cores);
    threadpool thpool = thpool_init(num_cores);

    char **result_words = NULL;
    int result_count = 0;
    double timing_count = 0;
    pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

    FILE *file = fopen(argv[1], "r");
    if (!file)
    {
        perror("Error opening file");
        thpool_destroy(thpool);
        return 1;
    }

    char line[MAX_WORD_LENGTH];
    word_block_t *current = malloc(sizeof(word_block_t));
    current->words = malloc(LINE_BLOCK_SIZE * sizeof(char*));
    current->count = 0;

    while (fgets(line, sizeof(line), file))
    {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n')
        {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) continue;

        current->words[current->count] = strdup(line);
        current->count++;

        if (current->count == LINE_BLOCK_SIZE)
        {
            task_data_t *task = malloc(sizeof(task_data_t));
            task->block = current;
            task->result_words = &result_words;
            task->result_count = &result_count;
            task->timing_count = &timing_count;
            task->result_mutex = &result_mutex;
            thpool_add_work(thpool, thread_work, task);

            current = malloc(sizeof(word_block_t));
            current->words = malloc(LINE_BLOCK_SIZE * sizeof(char*));
            current->count = 0;
        }
    }

    if (current->count > 0)
    {
        task_data_t *task = malloc(sizeof(task_data_t));
        task->block = current;
        task->result_words = &result_words;
        task->result_count = &result_count;
        task->timing_count = &timing_count;
        task->result_mutex = &result_mutex;
        thpool_add_work(thpool, thread_work, task);
    } else
    {
        free(current->words);
        free(current);
    }

    fclose(file);
    thpool_wait(thpool);
    thpool_destroy(thpool);

    printf("Total time for analyse words that can be typed: %.10f\n", timing_count);
    printf("Total words that can be typed: %d\n", result_count);

    if (result_count > 0 && result_count <= 100)
    {
        printf("List of words:\n");
        for (int i = 0; i < result_count; i++)
        {
            printf("  %s\n", result_words[i]);
        }
    } else if (result_count > 100)
    {
        printf("(List too long, showing first 100 words)\n");
        for (int i = 0; i < 100; i++)
        {
            printf("  %s\n", result_words[i]);
        }
    }

    for (int i = 0; i < result_count; i++)
    {
        free(result_words[i]);
    }
    free(result_words);

    return 0;
}
