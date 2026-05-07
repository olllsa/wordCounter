#include "word_counter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

static int adjacency_matrix[26][26];

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
 * @brief Checks if two characters are adjacent on a QWERTY keyboard
 *
 * Determines whether the keys corresponding to characters 'a' and 'b'
 * are adjacent according to the precomputed QWERTY layout matrix.
 * Adjacency includes horizontal neighbors, vertical neighbors,
 * and the same key (self-adjacency).
 *
 * Only lowercase English letters from 'a' to 'z' are supported.
 *
 * @param a - first character (must be in range 'a'..'z')
 * @param b - second character (must be in range 'a'..'z')
 * @return unsigned int - 1 if keys are adjacent, 0 otherwise
 *
 * @note This function uses the global adjacency_matrix initialized by
 *       init_qwerty_layout(). Ensure init_qwerty_layout() is called before
 *       using this function.
 *
 * @warning If either character is outside the 'a'..'z' range, the function
 *          returns 0.
 */
unsigned int is_adjacent(char a, char b)
{
    int i = CHAR_TO_INDEX(a);
    int j = CHAR_TO_INDEX(b);

    if (i < 0 || i > 25 || j < 0 || j > 25)
    {
        return 0;
    }

    return adjacency_matrix[i][j];
}

/**
 * @brief Filters typable words from input block
 * @param input pointer to input word block (not modified)
 * @return pointer to new word_block_t containing only typable words.
 *         Caller must free the returned block and its words using
 *         free_word_block().
 * @note Returns NULL on allocation failure.
 */
word_block_t* filter_typable_words(const word_block_t *input)
{
    if (!input || input->count == 0)
    {
        // Return empty but valid block
        word_block_t *empty = malloc(sizeof(word_block_t));
        if (!empty) return NULL;
        empty->words = NULL;
        empty->count = 0;
        return empty;
    }

    // First pass: count typable words
    int typable_count = 0;
    for (int i = 0; i < input->count; i++) {
        if (can_type_word(input->words[i]))
            typable_count++;
    }

    if (typable_count == 0)
    {
        word_block_t *empty = malloc(sizeof(word_block_t));
        if (!empty) return NULL;
        empty->words = NULL;
        empty->count = 0;
        return empty;
    }

    // Allocate result block
    word_block_t *result = malloc(sizeof(word_block_t));
    if (!result) return NULL;

    result->words = malloc(typable_count * sizeof(char*));
    if (!result->words) {
        free(result);
        return NULL;
    }

    // Second pass: copy typable words
    int idx = 0;
    for (int i = 0; i < input->count; i++) {
        if (can_type_word(input->words[i])) {
            result->words[idx] = strdup(input->words[i]);
            if (!result->words[idx]) {
                // Allocation failure – clean up already copied words
                for (int j = 0; j < idx; j++)
                    free(result->words[j]);
                free(result->words);
                free(result);
                return NULL;
            }
            idx++;
        }
    }
    result->count = typable_count;
    return result;
}

/**
 * @brief Frees memory allocated for a word block
 * @param block pointer to word_block_t to free
 */
void free_word_block(word_block_t *block)
{
    if (!block) return;
    for (int i = 0; i < block->count; i++)
        free(block->words[i]);
    free(block->words);
    free(block);
}
