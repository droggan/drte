#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "../src/gapbuffer.h"

static void
test_gbf_new(void) {
	GapBuffer *gbuf = gbf_new();
	char *text = gbf_text(gbuf);

	test_assert_str_eql(text, "");
	test_assert_uint_eql(first_part_length(gbuf), 0);
	test_assert_uint_eql(second_part_length(gbuf), 0);

	gbf_free(&gbuf);
	free(text);
}

static void
test_gbf_free(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_free(&gbuf);

	test_assert_null(gbuf);
}

static void
test_gbf_insert_start(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, "hello", 0);
	char *text = gbf_text(gbuf);

	test_assert_str_eql(text, "hello");
	test_assert_uint_eql(first_part_length(gbuf), 5);
	test_assert_uint_eql(second_part_length(gbuf), 0);

	free(text);
	gbf_free(&gbuf);
}

static void
test_gbf_insert_after(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, "hello", 0);
	gbf_insert(gbuf, " world", 5);
	char *text = gbf_text(gbuf);

	test_assert_str_eql(text, "hello world");
	test_assert_uint_eql(first_part_length(gbuf), 11);
	test_assert_uint_eql(second_part_length(gbuf), 0);

	free(text);
	gbf_free(&gbuf);
}

static void
test_gbf_insert_before(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, "world", 0);
	gbf_insert(gbuf, "hello ", 0);
	char *text = gbf_text(gbuf);

	test_assert_str_eql(text, "hello world");
	test_assert_uint_eql(first_part_length(gbuf), 6);
	test_assert_uint_eql(second_part_length(gbuf), 5);

	free(text);
	gbf_free(&gbuf);
}

static void
test_gbf_insert_oor(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, "hello", max_offset(gbuf) + 1);
	char *text = gbf_text(gbuf);

	test_assert_str_eql(text, "hello");

	free(text);
	gbf_free(&gbuf);
}

static void
test_gbf_delete_start(void) {
	GapBuffer * gbuf = gbf_new();
	gbf_insert(gbuf, "xhello world", 0);
	gbf_delete(gbuf, 0, 1);
	char *text = gbf_text(gbuf);

	test_assert_str_eql(text, "hello world");
	test_assert_uint_eql(first_part_length(gbuf), 0);
	test_assert_uint_eql(second_part_length(gbuf), 11);

	free(text);
	gbf_free(&gbuf);
}

static void
test_gbf_delete_mid(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, "helloxworld", 0);
	gbf_delete(gbuf, 5, 1);
	char *text = gbf_text(gbuf);

	test_assert_str_eql(text, "helloworld");
	test_assert_uint_eql(first_part_length(gbuf), 5);
	test_assert_uint_eql(second_part_length(gbuf), 5);

	free(text);
	gbf_free(&gbuf);
}

static void
test_gbf_delete_end(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, "hello worldxxx", 0);
	gbf_delete(gbuf, 11, 3);
	char *text = gbf_text(gbuf);

	test_assert_str_eql(text, "hello world");
	test_assert_uint_eql(first_part_length(gbuf), 11);
	test_assert_uint_eql(second_part_length(gbuf), 0);

	free(text);
	gbf_free(&gbuf);
}

static void
test_gbf_delete_oor(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, "hello", max_offset(gbuf) + 1);
	gbf_delete(gbuf, 6, 1);
	char *text = gbf_text(gbuf);

	test_assert_str_eql(text, "hello");

	free(text);
	gbf_free(&gbuf);
}

static void
test_gbf_delete_too_much(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, "hello world", 0);
	gbf_delete(gbuf, 5, 10000000);
	char *text = gbf_text(gbuf);

	test_assert_str_eql(text, "hello");

	free(text);
	gbf_free(&gbuf);
}

void
test_gbf_clear(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, "foo bar baz", 0);
	gbf_clear(gbuf);
	char *text = gbf_text(gbuf);

	test_assert_str_eql(text, "");
	test_assert_uint_eql(first_part_length(gbuf), 0);
	test_assert_uint_eql(second_part_length(gbuf), 0);

	free(text);
	gbf_free(&gbuf);
}

void
test_gbf_at(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, "hello world!\n", 0);

	char c = gbf_at(gbuf, 0);
	test_assert_uint_eql(c, 'h');

	c = gbf_at(gbuf, 7);
	test_assert_uint_eql(c, 'o');

	c = gbf_at(gbuf, 12);
	test_assert_uint_eql(c, '\n');

	c = gbf_at(gbuf, max_offset(gbuf) + 1);
	test_assert_uint_eql(c, '\0');

	gbf_free(&gbuf);
}

void
test_gbf_get_line(void) {
	GapBuffer *gbuf = gbf_new();
	char *s = "hello\nworld!\n\nfoo";
	char buffer[4096];

	gbf_insert(gbuf, s, 0);

	for (size_t i = 0; i <= 5; i++) {
		gbf_get_line(gbuf, i, buffer);
		test_assert_str_eql(buffer, "hello");
	}
	for (size_t i = 6; i <= 12; i++) {
		gbf_get_line(gbuf, i, buffer);
		test_assert_str_eql(buffer, "world!");
	}
	gbf_get_line(gbuf, 13, buffer);
	test_assert_str_eql(buffer, "");

	for (size_t i = 14; i <= 16; i++) {
		gbf_get_line(gbuf, i, buffer);
		test_assert_str_eql(buffer, "foo");
	}

	gbf_free(&gbuf);
}

static char *lorem = "Lorem ipsum dolor sit amet, consectetur adipiscing elit,\n"
	"sed do eiusmod tempor incididunt ut labore et dolore magna aliqua";

static char *foofoo = "hello foofoo world";

static void
test_make_table_1(void) {
	char *pattern = "ipsum";
	size_t len = 5;
	size_t table[len];
	size_t expected[5] = {0, 0, 0, 0, 0};

	make_lps_table(pattern, len, table);

	for (size_t i = 0; i < len; i++) {
		test_assert_size_t_eql(table[i], expected[i]);
	}
}

static void
test_make_table_2(void) {
	char *pattern = "foofoo";
	size_t len = 6;
	size_t table[len];
	size_t expected[6] = {0, 0, 0, 1, 2, 3};

	make_lps_table(pattern, len, table);

	for (size_t i = 0; i < len; i++) {
		test_assert_size_t_eql(table[i], expected[i]);
	}
}

static void
test_make_table_3(void) {
	char *pattern = "aaaaabb";
	size_t len = 7;
	size_t table[len];
	size_t expected[7] = {0, 1, 2, 3, 4, 0, 0};

	make_lps_table(pattern, len, table);

	for (size_t i = 0; i < len; i++) {
		test_assert_size_t_eql(table[i], expected[i]);
	}
}

static void
test_search_start(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t off = 0;

	res = gbf_search(gbuf, "Lorem", 5, 0, &off);
	test_assert_int_eql(res, true);
	test_assert_size_t_eql(off, (size_t)0);

	gbf_free(&gbuf);
}

static void
test_search_mid(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t off = 0;

	res = gbf_search(gbuf, "ipsum", 5, 0, &off);
	test_assert_int_eql(res, true);
	test_assert_size_t_eql(off, (size_t)6);

	gbf_free(&gbuf);
}

static void
test_search_end(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t off = 0;

	res = gbf_search(gbuf, "aliqua", 6, 0, &off);
	test_assert_int_eql(res, true);
	test_assert_size_t_eql(off, (size_t)116);

	gbf_free(&gbuf);
}

static void
test_search_start_mid(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t off = 0;

	res = gbf_search(gbuf, "aliqua", 6, 50, &off);
	test_assert_int_eql(res, true);
	test_assert_size_t_eql(off, (size_t)116);

	gbf_free(&gbuf);
}

static void
test_search_start_end(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t off = 0;

	res = gbf_search(gbuf, "aliqua", 6, 120, &off);
	test_assert_int_eql(res, false);
	test_assert_size_t_eql(off, (size_t)0);

	gbf_free(&gbuf);
}


static void
test_search_foofoo(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, foofoo, 0);
	bool res;
	size_t off = 0;

	res = gbf_search(gbuf, "foofoo", 6, 0, &off);
	test_assert_int_eql(res, true);
	test_assert_size_t_eql(off, (size_t)6);

	gbf_free(&gbuf);
}

static void
test_search_no_match(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t off = 0;

	res = gbf_search(gbuf, "hello", 5, 0, &off);
	test_assert_int_eql(res, false);
	test_assert_size_t_eql(off, (size_t)0);

	gbf_free(&gbuf);
}

static void
test_make_table_reverse_1(void) {
	char *pattern = "ipsum";
	size_t len = 5;
	size_t table[len];
	size_t expected[5] = {4, 4, 4, 4, 4};

	make_lps_table_reverse(pattern, len, table);

	for (size_t i = 0; i < len; i++) {
		test_assert_size_t_eql(table[i], expected[i]);
	}
}

static void
test_make_table_reverse_2(void) {
	char *pattern = "oofoof";
	size_t len = 6;
	size_t table[len];
	size_t expected[6] = {2, 3, 4, 5, 5, 5};

	make_lps_table_reverse(pattern, len, table);

	for (size_t i = 0; i < len; i++) {
		test_assert_size_t_eql(table[i], expected[i]);
	}
}

static void
test_make_table_reverse_3(void) {
	char *pattern = "bbaaaaaa";
	size_t len = 7;
	size_t table[len];
	size_t expected[7] = {6, 6, 2, 3, 4, 5, 6};

	make_lps_table_reverse(pattern, len, table);

	for (size_t i = 0; i < len; i++) {
		test_assert_size_t_eql(table[i], expected[i]);
	}
}

static void
test_search_start_reverse(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t off = 0;

	res = gbf_search_reverse(gbuf, "Lorem", 5, strlen(lorem), &off);
	test_assert_int_eql(res, true);
	test_assert_size_t_eql(off, (size_t)0);

	gbf_free(&gbuf);
}

static void
test_search_mid_reverse(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t off = 0;

	res = gbf_search_reverse(gbuf, "ipsum", 5, strlen(lorem), &off);
	test_assert_int_eql(res, true);
	test_assert_size_t_eql(off, (size_t)6);

	gbf_free(&gbuf);
}

static void
test_search_end_reverse(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t off = 0;

	res = gbf_search_reverse(gbuf, "aliqua", 6, strlen(lorem), &off);
	test_assert_int_eql(res, true);
	test_assert_size_t_eql(off, (size_t)116);

	gbf_free(&gbuf);
}

static void
test_search_start_mid_reverse(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t off = 0;

	res = gbf_search_reverse(gbuf, "Lorem", 5, 50, &off);
	test_assert_int_eql(res, true);
	test_assert_size_t_eql(off, (size_t)0);

	gbf_free(&gbuf);
}

static void
test_search_start_end_reverse(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t off = 0;

	res = gbf_search_reverse(gbuf, "aliqua", 6, 0, &off);
	test_assert_int_eql(res, false);
	test_assert_size_t_eql(off, (size_t)0);

	gbf_free(&gbuf);
}


static void
test_search_foofoo_reverse(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, foofoo, 0);
	bool res;
	size_t off = 0;

	res = gbf_search_reverse(gbuf, "foofoo", 6, strlen(foofoo), &off);
	test_assert_int_eql(res, true);
	test_assert_size_t_eql(off, (size_t)6);

	gbf_free(&gbuf);
}

static void
test_search_no_match_reverse(void) {
	GapBuffer *gbuf = gbf_new();
	gbf_insert(gbuf, lorem, 0);
	bool res;
	size_t len = strlen(lorem);
	size_t off = 0;

	res = gbf_search_reverse(gbuf, "hello", 5, len, &off);
	test_assert_int_eql(res, false);
	test_assert_size_t_eql(off, (size_t)0);

	gbf_free(&gbuf);
}

int
main(void) {
	test_gbf_new();
	test_gbf_free();
	test_gbf_insert_start();
	test_gbf_insert_before();
	test_gbf_insert_after();
	test_gbf_insert_oor();
	test_gbf_delete_start();
	test_gbf_delete_mid();
	test_gbf_delete_end();
	test_gbf_delete_oor();
	test_gbf_delete_too_much();
	test_gbf_clear();
	test_gbf_at();
	test_gbf_get_line();

	test_make_table_1();
	test_make_table_2();
	test_make_table_3();

	test_search_start();
	test_search_mid();
	test_search_end();
	test_search_start_mid();
	test_search_start_end();
	test_search_foofoo();
	test_search_no_match();

	test_make_table_reverse_1();
	test_make_table_reverse_2();
	test_make_table_reverse_3();

	test_search_start_reverse();
	test_search_mid_reverse();
	test_search_end_reverse();
	test_search_start_mid_reverse();
	test_search_start_end_reverse();
	test_search_foofoo_reverse();
	test_search_no_match_reverse();

	test_print_message();
	return 0;
}
