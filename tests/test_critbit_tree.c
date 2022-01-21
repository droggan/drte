#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "test.h"
#include "../src/gapbuffer.h"
#include "../src/critbit_tree.h"


void
test_empty(void) {
	CritbitTree *t = NULL;
	bool res = false;
	size_t val = 0;
	GapBuffer *gbuf = NULL;
	size_t len = 0;

	res = critbit_lookup(t, "hello", &val);
	test_assert_int_eql(res, false);
	test_assert_size_t_eql(val, (size_t)0);

	gbuf = critbit_keys_with_prefix(t, "x");
	len = gbf_text_length(gbuf);
	test_assert_size_t_eql(len, (size_t)0);
	gbf_free(&gbuf);
}

void
test_one(void) {
	CritbitTree *t = NULL;
	bool res = false;
	size_t val = 0;
	GapBuffer *gbuf = NULL;
	char *keys = "hello\n";
	char *text = NULL;

	critbit_insert(&t, "hello", 42);

	res = critbit_lookup(t, "hello", &val);
	test_assert_int_eql(res, true);
	test_assert_size_t_eql(val, (size_t)42);

	res = critbit_lookup(t, "hell", &val);
	test_assert_int_eql(res, false);
	test_assert_size_t_eql(val, (size_t)42);

	gbuf = critbit_keys_with_prefix(t, "");
	text = gbf_text(gbuf);
	test_assert_str_eql(text, keys);
	free(text);
	gbf_free(&gbuf);

	gbuf = critbit_keys_with_prefix(t, "h");
	text = gbf_text(gbuf);
	test_assert_str_eql(text, keys);
	free(text);
	gbf_free(&gbuf);

	gbuf = critbit_keys_with_prefix(t, "hel");
	text = gbf_text(gbuf);
	test_assert_str_eql(text, keys);
	free(text);
	gbf_free(&gbuf);

	gbuf = critbit_keys_with_prefix(t, "hello");
	text = gbf_text(gbuf);
	test_assert_str_eql(text, keys);
	free(text);
	gbf_free(&gbuf);

	gbuf = critbit_keys_with_prefix(t, "x");
	text = gbf_text(gbuf);
	test_assert_str_eql(text, "");
	free(text);
	gbf_free(&gbuf);

	critbit_free(&t);
	test_assert_null(t);
}

void
test_many(void) {
	CritbitTree *t = NULL;
	bool res = false;
	size_t val = 0;
	GapBuffer *gbuf = NULL;
	char *text = NULL;
	char *keys[8] = {"stick", "stone", "stuck", "banana",
		"tree", "fireplace", "friend", "search"};
	char *all_keys = "banana\nfireplace\nfriend\nsearch\nstick\nstone\nstuck\ntree\n";
	char *st_keys = "stick\nstone\nstuck\n";
	char *s_keys = "search\nstick\nstone\nstuck\n";
	char *f_keys = "fireplace\nfriend\n";
	char *b_keys = "banana\n";

	for (size_t i = 0; i < 8; i++) {
		critbit_insert(&t, keys[i], i + 1);
	}

	for (size_t i = 0; i < 8; i++) {
		res = critbit_lookup(t, keys[i], &val);
		test_assert_int_eql(res, true);
		test_assert_size_t_eql(val, i + 1);
	}

	gbuf = critbit_keys_with_prefix(t, "");
	text = gbf_text(gbuf);
	test_assert_str_eql(text, all_keys);
	free(text);
	gbf_free(&gbuf);

	gbuf = critbit_keys_with_prefix(t, "s");
	text = gbf_text(gbuf);
	test_assert_str_eql(text, s_keys);
	free(text);
	gbf_free(&gbuf);

	gbuf = critbit_keys_with_prefix(t, "st");
	text = gbf_text(gbuf);
	test_assert_str_eql(text, st_keys);
	free(text);
	gbf_free(&gbuf);

	gbuf = critbit_keys_with_prefix(t, "f");
	text = gbf_text(gbuf);
	test_assert_str_eql(text, f_keys);
	free(text);
	gbf_free(&gbuf);

	gbuf = critbit_keys_with_prefix(t, "banan");
	text = gbf_text(gbuf);
	test_assert_str_eql(text, b_keys);
	free(text);
	gbf_free(&gbuf);

	gbuf = critbit_keys_with_prefix(t, "truck");
	text = gbf_text(gbuf);
	test_assert_str_eql(text, "");
	free(text);
	gbf_free(&gbuf);

	critbit_free(&t);
	test_assert_null(t);
}

int
main(void) {

	test_empty();
	test_one();
	test_many();

	test_print_message();

	return 0;
}
