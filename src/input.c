#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <string.h>

#include "utf8.h"
#include "input.h"
#include "display.h"

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
input_check(char *input) {
	if (input[0] == KEY_ESCAPE) {
		// We have ESC
		if (input[1] == '\0') {
			return KEY_ESCAPE;
		}

		if (input[1] == 'O') {
			// We have ESCO
			switch (input[2]) {
			case 'A': return KEY_UP;
			case 'B': return KEY_DOWN;
			case 'C': return KEY_RIGHT;
			case 'D': return KEY_LEFT;
			case 'F': return KEY_END;
			case 'H': return KEY_HOME;
			case 'P': return KEY_F1;
			case 'Q': return KEY_F2;
			case 'R': return KEY_F3;
			case 'S': return KEY_F4;
			default: return KEY_INVALID;
			}
		}

		if (input[1] == '[') {
			// We have ESC[
			switch (input[2]) {
			case 'A': return KEY_UP;
			case 'B': return KEY_DOWN;
			case 'C': return KEY_RIGHT;
			case 'D': return KEY_LEFT;
			case 'F': return KEY_END;
			case 'H': return KEY_HOME;
			}

			if (input[3] == '~') {
				// We have ESC[x~
				switch (input[2]) {
				case '2': return KEY_INSERT;
				case '3': return KEY_DELETE;
				case '5': return KEY_PAGE_UP;
				case '6': return KEY_PAGE_DOWN;
				case '7': return KEY_HOME;
				case '8': return KEY_END;
				default: return KEY_INVALID;
				}
			}
			if (input[4] == '~') {
				// We have ESC[xx~
				if (input[2] == '1') {
					// We have ESC[1x~
					switch (input[3]) {
					case '1': return KEY_F1;
					case '2': return KEY_F2;
					case '3': return KEY_F3;
					case '4': return KEY_F4;
					case '5': return KEY_F5;
					case '7': return KEY_F6;
					case '8': return KEY_F7;
					case '9': return KEY_F8;
					default: return KEY_INVALID;
					}
				}
				if (input[2] == '2') {
					// We have ESC[2x~
					switch (input[3]) {
					case '0': return KEY_F9;
					case '1': return KEY_F10;
					case '3': return KEY_F11;
					case '4': return KEY_F12;
					default: return KEY_INVALID;
					}
				}
			}
		} else {
			// We have ESCx, where x is not [
			// \x61 is a, \x7A is z.
			if ((input[1] >= 'a') && (input[1] <= 'z'))
				return input[1] - 'a' + KEY_ALT_OFFSET;
			else
				return KEY_INVALID;
		}
	}
	// The input is not a special key. It's either a control character or a literal.
	if (input[0] >= 0 && input[0] <= 31)
		return input[0];
	if (input[0] == 127)
		return KEY_CTRL_QUESTIONMARK;
	if (utf8_is_valid(input))
		return KEY_VALID;

	return KEY_INVALID;
}

KeyCode
input_get(char *input, size_t timeout) {
	if (timeout != 0) {
		display_set_timeout(timeout);
	}
	int ret = read(1, input, 10);
	if (ret == 0) {
		display_clear_timeout();
		return KEY_TIMEOUT;
	}
	if (ret == -1) {
		if (errno == EINTR) {
			return KEY_RESIZE;
		} else {
			fprintf(stderr, "FAIL\r\n");
			exit(-1); // TODO
		}
	}
	return input_check(input);
}
