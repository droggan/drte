#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <string.h>

#include "static.h"
#include "utf8.h"
#include "input.h"
#include "display.h"


#define BUFFER_SIZE 4096

static KeyCode key_or_invalid(KeyCode key);
static size_t get_next(void);


char input_buffer[BUFFER_SIZE];
size_t current_char;
int input_remaining;


// Return key, if the next character is '~'. Return KEY_INVALID otherwise.
static KeyCode
key_or_invalid(KeyCode key) {
	size_t c = get_next();
	if (c == '~') {
		return key;
	} else {
		return KEY_INVALID;
	}
}

// Return the next byte or read more input.
static size_t
get_next(void) {
	if (input_remaining == 0) {
		input_remaining = read(1, input_buffer, BUFFER_SIZE);
		current_char = 0;
		if (input_remaining == 0) {
			display_clear_timeout();
			return KEY_TIMEOUT;
		}
		if (input_remaining == -1) {
			if (errno == EINTR) {
				return KEY_RESIZE;
			} else {
				fprintf(stderr, "FAIL\r\n");
				exit(-1); // TODO
				return 0;
			}
		}
	}
	size_t c = input_buffer[current_char];
	current_char++;
	input_remaining--;

	return c;
}

STATIC void
input_set(char *text) {
	if (text[0] == '\0') {
		input_buffer[0] = '\0';
		input_remaining = 1;
		current_char = 0;
	} else {
		strncpy(input_buffer, text, BUFFER_SIZE);
		input_remaining = strlen(text);
		current_char = 0;
	}
}

size_t
input_key_to_id(KeyCode c) {
	return c - KEY_SPECIAL_MIN;
}

// Different terminal produce diffent sequences.
// Sequences:
// ESC[A   -- Cursor up
// ESC[B   -- Cursor down
// ESC[C   -- Cursor right
// ESC[D   -- Cursor left
// ESC[F   -- Home
// ESC[H   -- End
// ESC[2~  -- Insert
// ESC[3~  -- Delete
// ESC[5~  -- PageUp
// ESC[6~  -- PageDown
// ESC[7~  -- Home
// ESC[8~  -- End
// ESC[11~ -- F1
// ESC[12~ -- F2
// ESC[13~ -- F3
// ESC[14~ -- F4
// ESC[15~ -- F5
// ESC[17~ -- F6
// ESC[18~ -- F7
// ESC[19~ -- F8
// ESC[20~ -- F9
// ESC[21~ -- F10
// ESC[23~ -- F11
// ESC[24~ -- F12
//
// ESCOA  -- Cursor up
// ESCOB  -- Cursor down
// ESCOC  -- Cursor right
// ESCOD  -- Cursor left
// ESCOF  -- Home
// ESCOH  -- End
// ESCOP  -- F1
// ESCOQ  -- F2
// ESCOR  -- F3
// ESCOS  -- F4
KeyCode
input_get(char buffer[]) {
	size_t c = get_next();

	if (c == 0x1B) {
		if (input_remaining == 0) {
			return KEY_ESCAPE;
		}
		c = get_next();
		if (c == 'O') {
			// We have ESCO
			c = get_next();
			switch (c) {
			case 'A': return KEY_UP;
			case 'B': return KEY_DOWN;
			case 'C': return KEY_RIGHT;
			case 'D': return KEY_LEFT;
			case 'F': return KEY_HOME;
			case 'H': return KEY_END;
			case 'P': return KEY_F1;
			case 'Q': return KEY_F2;
			case 'R': return KEY_F3;
			case 'S': return KEY_F4;
			default: return KEY_INVALID;
			}
		} else if (c == '[') {
			// We have ESC[
			c = get_next();
			switch (c) {
			case 'A': return KEY_UP;
			case 'B': return KEY_DOWN;
			case 'C': return KEY_RIGHT;
			case 'D': return KEY_LEFT;
			case 'F': return KEY_HOME;
			case 'H': return KEY_END;
			case '1':
				// We have ESC[1
				c = get_next();
				switch(c) {
				case '1': return key_or_invalid(KEY_F1);
				case '2': return key_or_invalid(KEY_F2);
				case '3': return key_or_invalid(KEY_F3);
				case '4': return key_or_invalid(KEY_F4);
				case '5': return key_or_invalid(KEY_F5);
				case '7': return key_or_invalid(KEY_F6);
				case '8': return key_or_invalid(KEY_F7);
				case '9': return key_or_invalid(KEY_F8);
				default: return KEY_INVALID;
				}
			case '2':
				// We have ESC[2
				c = get_next();
				switch(c) {
				case '0': return key_or_invalid(KEY_F9);
				case '1': return key_or_invalid(KEY_F10);
				case '3': return key_or_invalid(KEY_F11);
				case '4': return key_or_invalid(KEY_F12);
				case '~': return KEY_INSERT;
				default: return KEY_INVALID;
				}
			case '3': return key_or_invalid(KEY_DELETE);
			case '5': return key_or_invalid(KEY_PAGE_UP);
			case '6': return key_or_invalid(KEY_PAGE_DOWN);
			case '7': return key_or_invalid(KEY_HOME);
			case '8': return key_or_invalid(KEY_END);
			default: return KEY_INVALID;
			}
		} else {
			// We have ESCx, where x is not [ or O.
			if (c >= 'a' && c <= 'z') {
				return c - 'a' + KEY_ALT_OFFSET;
			} else {
				return KEY_INVALID;
			}
		}
	} else {
		// It is not ESC, so it's either a timeout, Ctrl+x or a character.
		if (c == KEY_TIMEOUT) {
			return c;
		} else if (c >= 0 && c <= 31) {
			return c + KEY_CTRL_OFFSET;
		} else if (c == 127) {
			return KEY_CTRL_QUESTIONMARK;
		} else {
			buffer[0] = c;
			if (utf8_is_valid(buffer)) {
				return KEY_VALID;
			} else {
				return KEY_INVALID;
			}
		}
	}
}
