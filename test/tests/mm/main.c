/**
* @file test/tests/mm/main.c
*
* @brief Test: Main program of the mm test
*
* Entry point for the test cases related to the memory allocator
*
* SPDX-FileCopyrightText: 2008-2021 HPDCS Group <rootsim@googlegroups.com>
* SPDX-License-Identifier: GPL-3.0-only
*/
#include <test.h>
#include "log/log.h"

extern test_ret_t model_allocator_test(void *);
extern test_ret_t model_allocator_test_hard(void *);
extern test_ret_t parallel_malloc_test(void *);

int main(void)
{
	init(0);

	// This is necessary to print out of memory messages without crashing the test
	FILE *f = fopen(".bitbucket", "w");
	log_init(f);

	test("Testing buddy system", model_allocator_test, NULL);
	test("Testing buddy system (hard test)", model_allocator_test_hard, NULL);
	test("Testing parallel memory operations", parallel_malloc_test, NULL);

	fclose(f);
	remove(".bitbucket");

	finish();
}
