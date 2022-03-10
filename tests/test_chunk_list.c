#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/chunk_list.h"
#include "test.h"

void
test_chunk_list(void) {
	ChunkList *cl = chunk_list_new(7);

	test_assert_not_null(cl);

	ChunkListItem *item1 = chunk_list_insert(cl, "lorem");
	ChunkListItem *item2 = chunk_list_insert(cl, "ipsum");
	ChunkListItem *item3 = chunk_list_insert(cl, "dolor sit amet");

	test_assert_not_null(item1);
	test_assert_not_null(item2);
	test_assert_not_null(item3);

	char *string1 = chunk_list_get_item(cl, item1);
	char *string2 = chunk_list_get_item(cl, item2);
	char *string3 = chunk_list_get_item(cl, item3);

	test_assert_str_eql(string1, "lorem");
	test_assert_str_eql(string2, "ipsum");
	test_assert_str_eql(string3, "dolor sit amet");

	chunk_list_free(&cl);
	test_assert_null(cl);

	free(item1);
	free(item2);
	free(item3);
	free(string1);
	free(string2);
	free(string3);
}


int
main(void) {
	test_chunk_list();
	test_print_message();
}
