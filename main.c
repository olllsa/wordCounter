#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "thpool/thpool.h"
#include "word_counter/word_counter.h"

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
    //num_cores = 100;

    printf("Using %d worker threads.\n", num_cores);

    threadpool thpool = thpool_init(num_cores);

    int total_typable = 0;
    char **result_words = NULL;
    int result_count = 0;
    double timing_count = 0;
    pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

    consumer_args_t consumer_args =
    {
        .total_count = &total_typable,
        .result_words = &result_words,
        .timing_count = &timing_count,
        .result_count = &result_count,
        .result_mutex = &result_mutex
    };

    FILE *file = fopen(argv[1], "r");
    if (!file)
    {
        perror("Error opening file");
        thpool_destroy(thpool);
        return 1;
    }

    char line[MAX_WORD_LENGTH];

    word_block_t *current_block = malloc(sizeof(word_block_t));
    current_block->words = malloc(LINE_BLOCK_SIZE * sizeof(char*));
    current_block->count = 0;

    while (fgets(line, sizeof(line), file))
    {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n')
        {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) continue;

        current_block->words[current_block->count] = strdup(line);
        current_block->count++;

        if (current_block->count == LINE_BLOCK_SIZE)
        {
            void *task_data = malloc(sizeof(task_data_t));
            ((task_data_t*)task_data)->block = current_block;
            ((task_data_t*)task_data)->args = &consumer_args;
            thpool_add_work(thpool, process_block, task_data);

            current_block = malloc(sizeof(word_block_t));
            current_block->words = malloc(LINE_BLOCK_SIZE * sizeof(char*));
            current_block->count = 0;
        }
    }

    if (current_block->count > 0)
    {
        void *task_data = malloc(sizeof(task_data_t));
        ((task_data_t*)task_data)->block = current_block;
        ((task_data_t*)task_data)->args = &consumer_args;
        thpool_add_work(thpool, process_block, task_data);
    }
    else
    {
        free(current_block->words);
        free(current_block);
    }

    fclose(file);

    thpool_wait(thpool);
    thpool_destroy(thpool);

    printf("Total time for analyse words that can be typed: %.10f\n", timing_count);
    printf("Total words that can be typed: %d\n", total_typable);
    if (total_typable > 0 && total_typable <= 100)
    {
        printf("List of words:\n");
        for (int i = 0; i < result_count; i++)
        {
            printf("  %s\n", result_words[i]);
        }
    }
    else if (total_typable > 100)
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
