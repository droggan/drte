#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "test.h"
#include "../src/input.h"
#include "../src/display.h"
#include "../src/userfuncs.h"
#include "../src/gapbuffer.h"
#include "../src/chunk_list.h"
#include "../src/menus.h"
#include "../src/buffer.h"


void
test_buffer_new(void) {
	Buffer *buf = NULL;
	buf = buffer_new(NULL, NULL);

	test_assert_not_null(buf);
	test_assert_null(buf->filename);
	test_assert_ptr_eql(buf, buf->next);
	test_assert_ptr_eql(buf, buf->prev);

	buffer_free(&buf);
}

void
test_buffer_append(void) {
	Buffer *one = buffer_new(NULL, NULL);
	Buffer *two = buffer_new(NULL, NULL);

	one->filename = strdup("one");
	two->filename = strdup("two");

	buffer_append(&one, two);
	test_assert_str_eql(one->next->filename, "two");
	test_assert_str_eql(one->prev->filename, "two");

	buffer_free(&one);
	buffer_free(&two);
}

void
test_buffer_append_2(void) {
	Buffer *one = buffer_new(NULL, NULL);
	Buffer *two = buffer_new(NULL, NULL);
	Buffer *three = buffer_new(NULL, NULL);

	one->filename = strdup("one");
	two->filename = strdup("two");
	three->filename = strdup("three");

	buffer_append(&one, two);
	buffer_append(&two, three);

	test_assert_str_eql(one->next->filename, "two");
	test_assert_str_eql(one->next->next->filename, "three");
	test_assert_str_eql(one->next->next->next->filename, "one");

	test_assert_str_eql(one->prev->filename, "three");
	test_assert_str_eql(one->prev->prev->filename, "two");
	test_assert_str_eql(one->prev->prev->prev->filename, "one");

	buffer_free(&one);
	buffer_free(&two);
	buffer_free(&three);
}

void
test_buffer_append_3(void) {
	Buffer *one = buffer_new(NULL, NULL);
	Buffer *two = buffer_new(NULL, NULL);
	Buffer *three = buffer_new(NULL, NULL);

	one->filename = strdup("one");
	two->filename = strdup("two");
	three->filename = strdup("three");

	buffer_append(&one, two);
	buffer_append(&one, three);

	test_assert_str_eql(one->next->filename, "three");
	test_assert_str_eql(one->next->next->filename, "two");
	test_assert_str_eql(one->next->next->next->filename, "one");

	test_assert_str_eql(one->prev->filename, "two");
	test_assert_str_eql(one->prev->prev->filename, "three");
	test_assert_str_eql(one->prev->prev->prev->filename, "one");

	buffer_free(&one);
	buffer_free(&two);
	buffer_free(&three);
}


void
test_buffer_free(void) {
	Buffer *one = buffer_new(NULL, NULL);
	Buffer *two = buffer_new(NULL, NULL);

	one->filename = malloc(10);
	two->filename = malloc(10);
	strcpy(one->filename, "one");
	strcpy(two->filename,"two");

	buffer_append(&one, two);
	buffer_free(&(one->next));
	test_assert_ptr_eql(one, one->next);
	test_assert_ptr_eql(one, one->prev);

	buffer_free(&one);
	test_assert_null(one);
}
void
test_buffer_free_2(void) {
	Buffer *first = buffer_new(NULL, NULL);
	Buffer *second = buffer_new(NULL, NULL);

	first->filename = malloc(10);
	second->filename = malloc(10);
	strcpy(first->filename, "first");
	strcpy(second->filename, "second");

	buffer_append(&first, second);
	buffer_free(&first);
	test_assert_ptr_eql(first, second);
	test_assert_ptr_eql(first, first->prev);
	test_assert_ptr_eql(first, first->next);

	buffer_free(&first);
	test_assert_null(first);
}

int
main(void) {
	test_buffer_new();
	test_buffer_append();
	test_buffer_append_2();
	test_buffer_append_3();
	test_buffer_free();
	test_print_message();
	return 0;
}
