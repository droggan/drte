#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "gapbuffer.h"
#include "input.h"
#include "display.h"
#include "funcs.h"
#include "userfuncs.h"
#include "chunk_list.h"
#include "menus.h"
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
editor_call_userfunc(Editor *e, KeyCode c) {
	UserFunc *uf = NULL;
	if (c == KEY_VALID) {
		uf = &uf_insert;
	} else if (c >= KEY_SPECIAL_MIN && c <= KEY_SPECIAL_MAX) {
		uf = e->current_buffer->funcs[c];
	} else {
		editor_show_message(e, "Unrecognized input");
	}
	if (uf != NULL) {
		uf->func(e);
		e->current_buffer->prev_func = uf;
	}
}

void
editor_loop_once(Editor *e) {
	KeyCode c;
	char input[32] = {0};

	if (e->current_buffer->draw != NULL) {
		e->current_buffer->draw(e);
	}

	c = input_get(input, 0);
	e->string_arg = input;

	editor_call_userfunc(e, c);

	Buffer *b = e->current_buffer;

	if (e->recording_macro && b->prev_func->func != macro_start_stop) {
		macro_append(e);
	}

	if (b->region_type == REGION_FLUID) {
		if (b->region_direction == REGION_DIRECTION_NONE) {
			if (b->position.offset > b->region_start) {
				b->region_direction = REGION_DIRECTION_RIGHT;
			} else if (b->position.offset < b->region_start) {
				b->region_direction = REGION_DIRECTION_LEFT;
			}
		}
		if (b->region_direction == REGION_DIRECTION_RIGHT) {
			b->region_end = b->position.offset;
		} else if (b->region_direction == REGION_DIRECTION_LEFT) {
			b->region_start = b->position.offset;
		}
		if (b->region_direction != REGION_DIRECTION_NONE) {
			if (b->region_start == b->region_end) {
				b->region_direction = REGION_DIRECTION_NONE;
			}
		}
		b->redraw = true;
	}
}

void
editor_loop(Editor *e) {
	while (!e->current_buffer->cancel && !e->current_buffer->ok) {
		editor_loop_once(e);
	}
}
