/**
 * @file test_word_counter.c
 * @brief Unit tests for QWERTY word counter module
 *
 * Compilation and usage:
 *
 * To compile with Unity testing framework:
 * gcc -Wall -Wextra -pthread -Ithpool -Iword_counter \
 *     test_word_counter.c word_counter/word_counter.c \
 *     unity/unity.c -o test_runner
 *
 * To run tests:
 * ./test_runner
 *
 * To compile with simple custom test harness (no dependencies):
 * gcc -Wall -Wextra -pthread -DUSE_SIMPLE_TESTS \
 *     test_word_counter.c word_counter/word_counter.c -o test_runner
 *
 * ./test_runner
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "word_counter/word_counter.h"

/* ============================================================ */
/* SIMPLE TEST HARNESS (no external dependencies)              */
/* ============================================================ */

#ifdef USE_SIMPLE_TESTS

#define TEST(name) static void name(void)
#define RUN_TEST(test) do { \
    printf("Running " #test "... "); \
    test(); \
    printf("PASSED\n"); \
    tests_passed++; \
} while(0)

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition) do { \
    if (!(condition)) { \
        printf("\nFAILED at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_EQUAL_STRING(expected, actual) do { \
    if (strcmp(expected, actual) != 0) { \
        printf("\nFAILED at %s:%d: expected '%s', got '%s'\n", \
               __FILE__, __LINE__, expected, actual); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_EQUAL_INT(expected, actual) do { \
    if ((expected) != (actual)) { \
        printf("\nFAILED at %s:%d: expected %d, got %d\n", \
               __FILE__, __LINE__, expected, actual); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_TRUE(actual) TEST_ASSERT((actual) == 1)
#define TEST_ASSERT_FALSE(actual) TEST_ASSERT((actual) == 0)

/* ============================================================ */
/* UNITY TEST FRAMEWORK (if available)                         */
/* ============================================================ */

#elif __has_include("unity/unity.h")

#include "unity/unity.h"

void setUp(void) {}
void tearDown(void) {}

#define TEST(name) void name(void)

#endif

/* ============================================================ */
/* TEST FIXTURES                                               */
/* ============================================================ */

static void setup_test_environment(void)
{
    init_qwerty_layout();
}

/* ============================================================ */
/* TESTS FOR init_qwerty_layout()                              */
/* ============================================================ */

TEST(test_horizontal_adjacency)
{
    setup_test_environment();

    // Test horizontal adjacencies in top row
    TEST_ASSERT_TRUE(is_adjacent('q', 'w'));
    TEST_ASSERT_TRUE(is_adjacent('w', 'e'));
    TEST_ASSERT_TRUE(is_adjacent('e', 'r'));
    TEST_ASSERT_TRUE(is_adjacent('r', 't'));
    TEST_ASSERT_TRUE(is_adjacent('t', 'y'));
    TEST_ASSERT_TRUE(is_adjacent('y', 'u'));
    TEST_ASSERT_TRUE(is_adjacent('u', 'i'));
    TEST_ASSERT_TRUE(is_adjacent('i', 'o'));
    TEST_ASSERT_TRUE(is_adjacent('o', 'p'));

    // Test horizontal adjacencies in middle row
    TEST_ASSERT_TRUE(is_adjacent('a', 's'));
    TEST_ASSERT_TRUE(is_adjacent('s', 'd'));
    TEST_ASSERT_TRUE(is_adjacent('d', 'f'));
    TEST_ASSERT_TRUE(is_adjacent('f', 'g'));
    TEST_ASSERT_TRUE(is_adjacent('g', 'h'));
    TEST_ASSERT_TRUE(is_adjacent('h', 'j'));
    TEST_ASSERT_TRUE(is_adjacent('j', 'k'));
    TEST_ASSERT_TRUE(is_adjacent('k', 'l'));

    // Test horizontal adjacencies in bottom row
    TEST_ASSERT_TRUE(is_adjacent('z', 'x'));
    TEST_ASSERT_TRUE(is_adjacent('x', 'c'));
    TEST_ASSERT_TRUE(is_adjacent('c', 'v'));
    TEST_ASSERT_TRUE(is_adjacent('v', 'b'));
    TEST_ASSERT_TRUE(is_adjacent('b', 'n'));
    TEST_ASSERT_TRUE(is_adjacent('n', 'm'));
}

TEST(test_vertical_adjacency)
{
    setup_test_environment();

    // Test vertical adjacencies between row 0 and row 1
    TEST_ASSERT_TRUE(is_adjacent('q', 'a'));
    TEST_ASSERT_TRUE(is_adjacent('w', 's'));
    TEST_ASSERT_TRUE(is_adjacent('e', 'd'));
    TEST_ASSERT_TRUE(is_adjacent('r', 'f'));
    TEST_ASSERT_TRUE(is_adjacent('t', 'g'));
    TEST_ASSERT_TRUE(is_adjacent('y', 'h'));
    TEST_ASSERT_TRUE(is_adjacent('u', 'j'));
    TEST_ASSERT_TRUE(is_adjacent('i', 'k'));
    TEST_ASSERT_TRUE(is_adjacent('o', 'l'));

    // Test vertical adjacencies between row 1 and row 2
    TEST_ASSERT_TRUE(is_adjacent('a', 'z'));
    TEST_ASSERT_TRUE(is_adjacent('s', 'x'));
    TEST_ASSERT_TRUE(is_adjacent('d', 'c'));
    TEST_ASSERT_TRUE(is_adjacent('f', 'v'));
    TEST_ASSERT_TRUE(is_adjacent('g', 'b'));
    TEST_ASSERT_TRUE(is_adjacent('h', 'n'));
    TEST_ASSERT_TRUE(is_adjacent('j', 'm'));
}

TEST(test_self_adjacency)
{
    setup_test_environment();

    // Every key should be adjacent to itself
    for (char c = 'a'; c <= 'z'; c++)
    {
        TEST_ASSERT_TRUE(is_adjacent(c, c));
    }
}

TEST(test_non_adjacent_keys)
{
    setup_test_environment();

    // Test non-adjacent keys
    TEST_ASSERT_FALSE(is_adjacent('q', 'e'));
    TEST_ASSERT_FALSE(is_adjacent('a', 'd'));
    TEST_ASSERT_FALSE(is_adjacent('z', 'c'));
    TEST_ASSERT_FALSE(is_adjacent('q', 'p'));
    TEST_ASSERT_FALSE(is_adjacent('a', 'l'));
}

/* ============================================================ */
/* TESTS FOR can_type_word()                                   */
/* ============================================================ */

TEST(test_typable_words)
{
    setup_test_environment();

    // Single letter words should return 0
    TEST_ASSERT_FALSE(can_type_word("a"));
    TEST_ASSERT_FALSE(can_type_word("z"));

    // Horizontal sequences
    TEST_ASSERT_TRUE(can_type_word("qwerty"));
    TEST_ASSERT_TRUE(can_type_word("asdfgh"));
    TEST_ASSERT_TRUE(can_type_word("zxcvbnm"));
    TEST_ASSERT_TRUE(can_type_word("uiop"));

    // Vertical sequences
    TEST_ASSERT_TRUE(can_type_word("qaz"));
    TEST_ASSERT_TRUE(can_type_word("wsx"));
    TEST_ASSERT_TRUE(can_type_word("edc"));
    TEST_ASSERT_TRUE(can_type_word("rfv"));
    TEST_ASSERT_TRUE(can_type_word("tgb"));
    TEST_ASSERT_TRUE(can_type_word("yhn"));
    TEST_ASSERT_TRUE(can_type_word("ujm"));

    // Real words that should be typable
    TEST_ASSERT_TRUE(can_type_word("deer"));
    TEST_ASSERT_TRUE(can_type_word("loop"));
    TEST_ASSERT_TRUE(can_type_word("asdf"));
    TEST_ASSERT_TRUE(can_type_word("qwert"));
}

TEST(test_non_typable_words)
{
    setup_test_environment();

    // Words with non-adjacent consecutive letters
    TEST_ASSERT_FALSE(can_type_word("cat"));   // a->t not adjacent
    TEST_ASSERT_FALSE(can_type_word("dog"));   // d->o not adjacent
    TEST_ASSERT_FALSE(can_type_word("bird"));  // b->i not adjacent
    TEST_ASSERT_FALSE(can_type_word("fish"));  // f->i not adjacent
    TEST_ASSERT_FALSE(can_type_word("jump"));  // j->u not adjacent
    TEST_ASSERT_FALSE(can_type_word("type"));

    // Words with invalid characters should return 0
    TEST_ASSERT_FALSE(can_type_word("Hello"));   // Uppercase H
    TEST_ASSERT_FALSE(can_type_word("hello1"));  // Contains number
    TEST_ASSERT_FALSE(can_type_word(""));        // Empty string
}

TEST(test_boundary_cases)
{
    setup_test_environment();

    // Very long word (should still work)
    TEST_ASSERT_TRUE(can_type_word("asdfghjkl"));
    TEST_ASSERT_FALSE(can_type_word("abcdefghijklmnopqrstuvwxyz"));

    // Words with repeated letters
    TEST_ASSERT_TRUE(can_type_word("aaaa"));
    TEST_ASSERT_TRUE(can_type_word("ssss"));

    // Edge of keyboard
    TEST_ASSERT_TRUE(can_type_word("poi"));
    TEST_ASSERT_TRUE(can_type_word("lkj"));
    TEST_ASSERT_TRUE(can_type_word("mnb"));
}

/* ============================================================ */
/* MAIN FUNCTION                                               */
/* ============================================================ */

int main(void)
{
    printf("\n========================================\n");
    printf("WORD COUNTER UNIT TESTS\n");
    printf("========================================\n\n");

#ifdef USE_SIMPLE_TESTS

    RUN_TEST(test_horizontal_adjacency);
    RUN_TEST(test_vertical_adjacency);
    RUN_TEST(test_self_adjacency);
    RUN_TEST(test_non_adjacent_keys);
    RUN_TEST(test_typable_words);
    RUN_TEST(test_non_typable_words);
    RUN_TEST(test_boundary_cases);

    printf("\n========================================\n");
    printf("RESULTS: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("========================================\n");

    return (tests_failed == 0) ? 0 : 1;

#elif __has_include("unity/unity.h")

    UNITY_BEGIN();
    RUN_TEST(test_horizontal_adjacency);
    RUN_TEST(test_vertical_adjacency);
    RUN_TEST(test_self_adjacency);
    RUN_TEST(test_non_adjacent_keys);
    RUN_TEST(test_typable_words);
    RUN_TEST(test_non_typable_words);
    RUN_TEST(test_boundary_cases);
    return UNITY_END();

#else

    printf("ERROR: No test framework available.\n");
    printf("Compile with -DUSE_SIMPLE_TESTS or install Unity framework.\n");
    return 1;

#endif
}
