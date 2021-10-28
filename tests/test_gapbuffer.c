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

	test_print_message();
	return 0;
}
