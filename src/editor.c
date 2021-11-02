#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "gapbuffer.h"
#include "input.h"
#include "display.h"
#include "funcs.h"
#include "userfuncs.h"
#include "buffer.h"
#include "editor.h"
#include "utf8.h"


void
editor_show_message(Editor *e, char *message) {
	display_clear_window(e->messagebar_win);
	display_show_string(e->messagebar_win, 0, 0, message);
	e->shows_message = true;
}

void
editor_draw_statusbar(Editor *e) {
	char text[1024];
	Buffer *buf = e->current_buffer;

	display_set_color(BACKGROUND_BLACK);
	snprintf(text, 1023, "Pos:(%zu:%zu)Cur:(%zu|%zu)Off:(T:%zu|O:%zu)(%x):%s%s:",
			 buf->position.line, buf->position.column,
			 buf->cursor.line, buf->cursor.column,
			 buf->first_visible_char, buf->position.offset,
			 gbf_at(buf->gbuf, buf->position.offset),
			 buf->filename ? buf->filename : "Unnamed", buf->has_changed ? "*" : " ");

	size_t col = display_show_string(e->statusbar_win, 0, 0, text);
	while (col < e->display.columns) {
		display_show_cp(e->statusbar_win, 0, col, " ");
		col++;
	}

	display_set_color(OFF);
}

void
editor_loop(Editor *e) {
	KeyCode c;

	while (!e->quit && !e->current_buffer->cancel && !e->current_buffer->ok) {
		char input[32] = {0};

		e->current_buffer->draw(e);

		c = input_get(input);

		if (c >= KEY_SPECIAL_MIN && c <= KEY_SPECIAL_MAX) {
			Func f = e->current_buffer->funcs[c].func;
			if (f != NULL) {
				f(e);
				e->current_buffer->prev_func = f;
			}
		} else if (c == KEY_VALID) {
			e->string_arg = input;
			insert(e);
			e->current_buffer->prev_func = insert;
		} else {
			editor_show_message(e, "Unrecognized input");
		}

		if (e->recording_macro && e->current_buffer->prev_func != macro_start_stop) {
			e->string_arg = input;
			macro_append(e);
		}

	}
}
