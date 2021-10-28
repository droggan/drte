#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "test.h"
#include "../src/utf8.h"

void
test_whitespace(void) {
	bool b;

	b = utf8_is_whitespace(' ');
	test_assert_int_eql(b, true);

	b = utf8_is_whitespace('\t');
	test_assert_int_eql(b, true);

	b = utf8_is_whitespace('\n');
	test_assert_int_eql(b, true);

	b = utf8_is_whitespace('a');
	test_assert_int_eql(b, false);

	b = utf8_is_whitespace('1');
	test_assert_int_eql(b, false);

	b = utf8_is_whitespace('%');
	test_assert_int_eql(b, false);

}

int
main(void) {
	test_whitespace();
	test_print_message();
}
