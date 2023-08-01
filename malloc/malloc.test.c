#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "testlib.h"
#include "malloc.h"


// **********************
// **** MALLOC TESTS ****
// **********************

static void
successful_malloc_returns_non_null_pointer(void)
{
	char *var = malloc(100);
	ASSERT_TRUE("successful malloc returns non null pointer", var != NULL);
	free(var);
}

static void
correct_copied_value(void)
{
	char *test_string = "FISOP malloc is working!";
	char *var = malloc(100);
	strcpy(var, test_string);
	ASSERT_TRUE("allocated memory should contain the copied value",
	            strcmp(var, test_string) == 0);

	free(var);
}

static void
correct_amount_of_mallocs(void)
{
	struct malloc_stats stats;
	char *var = malloc(100);
	get_stats(&stats);

	ASSERT_TRUE("amount of mallocs should be one", stats.mallocs == 1);

	free(var);
}

static void
correct_amount_of_frees(void)
{
	struct malloc_stats stats;
	char *var = malloc(100);
	free(var);

	get_stats(&stats);

	ASSERT_TRUE("amount of frees should be one", stats.frees == 1);
	ASSERT_TRUE("amount of requested memory is 0",
	            stats.requested_memory == 0);
}

static void
correct_amount_of_requested_memory(void)
{
	struct malloc_stats stats;

	char *var = malloc(600);
	get_stats(&stats);

	ASSERT_TRUE("amount of requested memory should be 600",
	            stats.requested_memory == 600);

	free(var);
}

static void
unsuccesful_malloc_when_asking_for_more_than_max_size()
{
	char *var = malloc(33554435);
	ASSERT_TRUE("malloc should return NULL when asking too much memory",
	            var == NULL);
	free(var);
}

static void
malloc_of_0_return_null_pointer()
{
	char *var = malloc(0);
	ASSERT_TRUE("malloc should return NULL when passing 0 as argument",
	            var == NULL);
	free(var);
}

static void
malloc_of_0_stats_are_same()
{
	struct malloc_stats stats;
	char *var = malloc(0);
	get_stats(&stats);
	ASSERT_TRUE("amount of mallocs, frees and requested memory should be "
	            "0, when doing malloc of 0",
	            stats.mallocs == 0 && stats.frees == 0 &&
	                    stats.requested_memory == 0);
	free(var);
}


static void
malloc_de_2_bloques_medianos_y_se_libera_uno_actuliza_stats_correctamente()
{
	struct malloc_stats stats;

	char *bloque1 = malloc(MEDIUM_BLOCK_SIZE);
	char *bloque2 = malloc(MEDIUM_BLOCK_SIZE);
	free(bloque1);


	get_stats(&stats);

	ASSERT_TRUE("amount of requested memory should be 1048576",
	            stats.requested_memory == MEDIUM_BLOCK_SIZE);

	free(bloque2);
}


static void
malloc_con_menos_de_memoria_minima_te_devuelve_region_con_memoria_minima()
{
	struct malloc_stats stats;

	char *var = malloc(100);
	get_stats(&stats);

	ASSERT_TRUE("amount of requested memory should be min size region",
	            stats.requested_memory == MIN_SIZE);

	free(var);
}


static void
four_mallocs_and_four_frees_correctly()
{
	struct malloc_stats stats;
	int size1 = 400;
	int size2 = 20000;
	int size3 = 1148576;
	int size4 = 2048576;
	char *var1 = malloc(size1);
	char *var2 = malloc(size2);
	char *var3 = malloc(size3);
	char *var4 = malloc(size4);
	get_stats(&stats);
	ASSERT_TRUE("amount of malloc should be 4", stats.mallocs == 4);
	ASSERT_TRUE("amount of frees should be 0", stats.frees == 0);
	ASSERT_TRUE(
	        "amount of requested memory should be the sum of the 4 sizes",
	        stats.requested_memory == size1 + size2 + size3 + size4);
	free(var1);
	free(var2);
	free(var3);
	free(var4);
	get_stats(&stats);
	ASSERT_TRUE(
	        "amount of frees should be 4 and requested memory should be 0",
	        stats.requested_memory == 0 && stats.frees == 4);
}


static void
null_free_doesnt_change_amount_of_frees()
{
	struct malloc_stats stats;
	char *var1 = malloc(500);
	free(var1);
	free(NULL);
	get_stats(&stats);
	ASSERT_TRUE("null free should have an unexpected behaviour and amount "
	            "of frees should not increase",
	            stats.frees == 1 && stats.requested_memory == 0);
}

static void
three_mallocs_one_free_one_malloc_three_frees()
{
	struct malloc_stats stats;
	int size_1 = 500;
	int size_2 = 2048576;
	int size_3 = 33554020;
	int size_4 = 10000;
	char *var1 = malloc(size_1);
	char *var2 = malloc(size_2);
	char *var3 = malloc(size_3);
	get_stats(&stats);
	ASSERT_TRUE("amount of mallocs is three, requested memory is the sum "
	            "of the 3 sizes and amounts of frees is 0",
	            stats.mallocs == 3 &&
	                    stats.requested_memory == size_1 + size_2 + size_3 &&
	                    stats.frees == 0);
	free(var2);
	get_stats(&stats);
	ASSERT_TRUE("requested memory is changed when free one malloc",
	            stats.requested_memory == size_1 + size_3 && stats.frees == 1);
	char *var4 = malloc(size_4);
	get_stats(&stats);
	ASSERT_TRUE("requested memory is changed when malloc of new size",
	            stats.mallocs == 4 &&
	                    stats.requested_memory == size_1 + size_3 + size_4);
	free(var1);
	free(var3);
	free(var4);
	get_stats(&stats);
	ASSERT_TRUE("correct frees of the three mallocs",
	            stats.frees == 4 && stats.requested_memory == 0);
}


// **********************
// **** REALLOC TESTS ****
// **********************


static void
realloc_with_null_pointer()
{
	char *var = realloc(NULL, 100);
	ASSERT_TRUE(
	        "realloc should return non null pointer when passing NULL as "
	        "first argument",
	        var != NULL);
	free(var);
}

static void
realloc_with_non_null_pointer()
{
	char *var = malloc(100);
	char *var2 = realloc(var, 200);
	ASSERT_TRUE(
	        "realloc should return non null pointer when passing non null "
	        "pointer as first argument",
	        var2 != NULL);
	free(var2);
}

static void
realloc_with_size_smaller_than_min_size_requested_memory_is_the_same()
{
	struct malloc_stats stats;

	char *var = malloc(100);
	char *var2 = realloc(var, 200);
	get_stats(&stats);
	ASSERT_TRUE("reallocs should return the same pointer when size is "
	            "smaller than min size",
	            var == var2);
	ASSERT_TRUE("realloc with size smaller than min size requested memory "
	            "is the same",
	            stats.requested_memory == MIN_SIZE);
	free(var2);
}

static void
realloc_with_size_smaller_than_pointer_size_and_min_size_updates_requested_memory_and_returns_same_pointer()
{
	struct malloc_stats stats;

	char *var = malloc(1000);
	char *var2 = realloc(var, 800);
	get_stats(&stats);
	ASSERT_TRUE("realloc should return the same pointer when size is "
	            "smaller than pointer size, but the difference between "
	            "sizes is smaller than min size",
	            var == var2);
	ASSERT_TRUE("realloc with size smaller than pointer size, but "
	            "difference with original pointer is smaller than min size "
	            "requested memory is the same",
	            stats.requested_memory == 1000);
	free(var2);
}


static void
realloc_with_size_smaller_than_pointer_size_updates_requested_memory_and_returns_same_pointer()
{
	struct malloc_stats stats;

	char *var = malloc(1000);
	char *var2 = realloc(var, 100);
	get_stats(&stats);
	ASSERT_TRUE("realloc should return the same pointer when size is "
	            "smaller than pointer size",
	            var == var2);
	ASSERT_TRUE("realloc with size smaller than pointer size requested "
	            "memory changes",
	            stats.requested_memory == MIN_SIZE);
	free(var2);
}

static void
realloc_with_equal_size_as_pointer_returns_same_pointer_ans_requested_memory_is_the_same()
{
	struct malloc_stats stats;

	char *var = malloc(1000);
	char *var2 = realloc(var, 1000);
	get_stats(&stats);
	ASSERT_TRUE("realloc should return the same pointer when size is equal "
	            "than pointer size",
	            var == var2);
	ASSERT_TRUE("realloc with size equal than pointer size requested "
	            "memory stays the same",
	            stats.requested_memory == 1000);
	free(var2);
}

static void
realloc_with_null_size()
{
	char *var = malloc(100);
	char *var2 = realloc(var, 0);
	ASSERT_TRUE("realloc should return non null pointer when passing 0 as "
	            "second argument",
	            var2 == NULL);
	free(var2);
}

static void
realloc_with_null_size_and_null_pointer()
{
	char *var = realloc(NULL, 0);
	ASSERT_TRUE("realloc should return null pointer when passing 0 as "
	            "second argument and NULL as first argument",
	            var == NULL);
}

static void
realloc_dont_change_pointer_when_fails()
{
	char *var = malloc(100);
	char *var2 = realloc(var, 33554435);
	ASSERT_TRUE("realloc should return the same pointer when fails",
	            var != NULL && var2 == NULL);
	free(var);
}

static void
realloc_dont_change_pointer_when_fails_with_null_pointer()
{
	char *var = realloc(NULL, 33554435);
	ASSERT_TRUE("realloc should return NULL when fails", var == NULL);
	free(var);
}

static void
realloc_dont_change_the_content_when_the_new_size_is_lower()
{
	char *var = malloc(100);
	char *var2 = realloc(var, 50);
	ASSERT_TRUE(
	        "realloc should not change the content when the new size is "
	        "lower",
	        var == var2);
	free(var2);
}

static void
realloc_dont_change_the_content_when_the_new_size_is_higher_and_there_is_a_free_region_next()
{
	char *var = malloc(500);
	char *var2 = realloc(var, 1000);
	ASSERT_TRUE(
	        "realloc should  not change the pointer when the new size is "
	        "higher and there is a free region next to it",
	        var == var2);
	struct malloc_stats stats;
	get_stats(&stats);
	ASSERT_TRUE("amount of requested memory should be 1000",
	            stats.requested_memory == 1000);
	free(var2);
}

static void
realloc_gives_new_region_when_next_region_is_not_big_enough()
{
	char *var = malloc(15000);
	char *var2 = realloc(var, 30000);
	ASSERT_TRUE("realloc should change the pointer when the new size is "
	            "higher and there is not a free region next to it",
	            var != var2);
	struct malloc_stats stats;
	get_stats(&stats);
	ASSERT_TRUE("amount of requested memory should be 32784",
	            stats.requested_memory == 30000);
	free(var2);
}


static void
realloc_dont_change_the_size_of_the_old_region_when_ask_for_a_lower_size_of_min_region_size()
{
	char *var = malloc(400);
	char *var2 = realloc(var, 300);
	struct region *region = (struct region *) var - 1;
	size_t oldSize = region->size;
	ASSERT_TRUE("realloc should change the size of the old region when the "
	            "new size is lower",
	            oldSize == 400);
	free(var2);
}

// **********************
// **** CALLOC TESTS ****
// **********************

static void
calloc_with_null_size()
{
	char *var = calloc(0, 100);
	ASSERT_TRUE("calloc should return NULL when passing 0 as "
	            "first argument",
	            var == NULL);
	free(var);
}

static void
calloc_with_null_count()
{
	char *var = calloc(100, 0);
	ASSERT_TRUE("calloc should return NULL when passing 0 as "
	            "second argument",
	            var == NULL);
	free(var);
}

static void
calloc_with_null_count_and_null_size()
{
	char *var = calloc(0, 0);
	ASSERT_TRUE("calloc should return NULL when passing 0 as "
	            "second argument and 0 as first argument",
	            var == NULL);
	free(var);
}

static void
calloc_with_non_null_count_and_non_null_size()
{
	char *var = calloc(100, 100);
	ASSERT_TRUE(
	        "calloc should return non null pointer when passing non null "
	        "pointer as first argument and non null pointer as second "
	        "argument",
	        var != NULL);
	free(var);
}

static void
calloc_with_non_null_count_and_non_null_size_and_check_content()
{
	char *var = calloc(100, 100);
	bool isZero;
	for (int i = 0; i < 100; i++) {
		isZero = var[i] == 0;
		if (!isZero) {
			break;
		}
	}
	ASSERT_TRUE("calloc should return non null pointer when passing non "
	            "null parameters, and the content should be 0",
	            isZero);
	free(var);
}


int
main(void)
{
	run_test(successful_malloc_returns_non_null_pointer);
	run_test(correct_copied_value);
	run_test(correct_amount_of_mallocs);
	run_test(correct_amount_of_frees);
	run_test(correct_amount_of_requested_memory);
	run_test(unsuccesful_malloc_when_asking_for_more_than_max_size);
	run_test(malloc_of_0_return_null_pointer);
	run_test(malloc_of_0_stats_are_same);
	run_test(malloc_de_2_bloques_medianos_y_se_libera_uno_actuliza_stats_correctamente);
	run_test(malloc_con_menos_de_memoria_minima_te_devuelve_region_con_memoria_minima);
	run_test(four_mallocs_and_four_frees_correctly);
	run_test(three_mallocs_one_free_one_malloc_three_frees);
	run_test(null_free_doesnt_change_amount_of_frees);
	run_test(realloc_with_null_pointer);
	run_test(realloc_with_non_null_pointer);
	run_test(realloc_with_size_smaller_than_min_size_requested_memory_is_the_same);
	run_test(realloc_with_equal_size_as_pointer_returns_same_pointer_ans_requested_memory_is_the_same);
	run_test(realloc_with_size_smaller_than_pointer_size_and_min_size_updates_requested_memory_and_returns_same_pointer);
	run_test(realloc_with_size_smaller_than_pointer_size_updates_requested_memory_and_returns_same_pointer);
	run_test(realloc_with_null_size);
	run_test(realloc_with_null_size_and_null_pointer);
	run_test(realloc_dont_change_pointer_when_fails);
	run_test(realloc_dont_change_pointer_when_fails_with_null_pointer);
	run_test(realloc_dont_change_the_content_when_the_new_size_is_lower);
	run_test(realloc_dont_change_the_content_when_the_new_size_is_higher_and_there_is_a_free_region_next);
	run_test(realloc_dont_change_the_size_of_the_old_region_when_ask_for_a_lower_size_of_min_region_size);
	run_test(realloc_gives_new_region_when_next_region_is_not_big_enough);
	run_test(calloc_with_null_size);
	run_test(calloc_with_null_count);
	run_test(calloc_with_null_count_and_null_size);
	run_test(calloc_with_non_null_count_and_non_null_size);
	run_test(calloc_with_non_null_count_and_non_null_size_and_check_content);

	return 0;
}
