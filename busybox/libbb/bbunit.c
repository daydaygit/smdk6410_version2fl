/* vi: set sw=4 ts=4: */
/*
 * bbunit: Simple unit-testing framework for Busybox.
 *
 * Copyright (C) 2014 by Bartosz Golaszewski <bartekgola@gmail.com>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */

//kbuild:lib-$(CONFIG_UNIT_TEST) += bbunit.o
//applet:IF_UNIT_TEST(APPLET(unit, BB_DIR_USR_BIN, BB_SUID_DROP))

//usage:#define unit_trivial_usage
//usage:       ""
//usage:#define unit_full_usage "\n\n"
//usage:       "Run the unit-test suite"

#include "libbb.h"

#define WANT_TIMING 0

static llist_t *tests = NULL;
static unsigned tests_registered = 0;
static int test_retval;

void bbunit_registertest(struct bbunit_listelem *test)
{
	llist_add_to_end(&tests, test);
	tests_registered++;
}

void bbunit_settestfailed(void)
{
	test_retval = -1;
}

#if WANT_TIMING
static void timeval_diff(struct timeval* res,
				const struct timeval* x,
				const struct timeval* y)
{
	long udiff = x->tv_usec - y->tv_usec;

	res->tv_sec = x->tv_sec - y->tv_sec - (udiff < 0);
	res->tv_usec = (udiff >= 0 ? udiff : udiff + 1000000);
}
#endif

int unit_main(int argc UNUSED_PARAM, char **argv UNUSED_PARAM) MAIN_EXTERNALLY_VISIBLE;
int unit_main(int argc UNUSED_PARAM, char **argv UNUSED_PARAM)
{
	unsigned tests_run = 0;
	unsigned tests_failed = 0;
#if WANT_TIMING
	struct timeval begin;
	struct timeval end;
	struct timeval time_spent;
	gettimeofday(&begin, NULL);
#endif

	bb_error_msg("Running %d test(s)...", tests_registered);
	for (;;) {
		struct bbunit_listelem* el = llist_pop(&tests);
		if (!el)
			break;
		bb_error_msg("Case: [%s]", el->name);
		test_retval = 0;
		el->testfunc();
		if (test_retval < 0) {
			bb_error_msg("[ERROR] [%s]: TEST FAILED", el->name);
			tests_failed++;
		}
		tests_run++;
		el = el->next;
	}

#if WANT_TIMING
	gettimeofday(&end, NULL);
	timeval_diff(&time_spent, &end, &begin);
	bb_error_msg("Elapsed time %u.%06u seconds",
			(int)time_spent.tv_sec,
			(int)time_spent.tv_usec);
#endif
	if (tests_failed > 0) {
		bb_error_msg("[ERROR] %u test(s) FAILED", tests_failed);
		return EXIT_FAILURE;
	}
	bb_error_msg("All tests passed");
	return EXIT_SUCCESS;
}
