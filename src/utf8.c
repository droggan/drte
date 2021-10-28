#include <stdlib.h>
#include <stdbool.h>

#include "utf8.h"


// TODO
bool
utf8_is_valid(char *text) {
	(void)text;
	return true;
}

// TDOO
bool
utf8_is_valid_first_byte(char c) {
	(void)c;
	return true;
}

// TODO
size_t
utf8_draw_width(char text) {
	if (text == '\t')
		return 4;
	return 1;
}

// TODO
size_t
utf8_byte_size(char text) {
	(void)text;
	return 1;
}

// TODO
bool
utf8_is_whitespace(char c) {
	if ((c == '\t') || (c == ' ') || (c == '\n')) {
		return true;
	} else {
		return false;
	}
}
