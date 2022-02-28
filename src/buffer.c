#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <errno.h>

#include "utf8.h"
#include "gapbuffer.h"
#include "display.h"
#include "input.h"
#include "funcs.h"
#include "userfuncs.h"
#include "buffer.h"
#include "editor.h"


static void buffer_draw_func(Editor *e);
static Buffer *make_isearch_buffer(Editor *e);


static Buffer *
make_isearch_buffer(Editor *e) {
	Buffer *buf = malloc(sizeof(*buf));
	if (buf == NULL) {
		editor_show_message(e, "Out of memory");
		return NULL;
	}

	memset(buf, 0, sizeof(*buf));

	buf->gbuf = gbf_new();
	if (buf->gbuf == NULL) {
		editor_show_message(e, "Out of memory");
		return NULL;
	}

	buf->redraw = 1;

	buf->position.line = 1;
	buf->position.column = 1;

	if (e != NULL) {
		buf->win = &e->window;
		buf->statusbar_win = &e->statusbar_win;
		buf->messagebar_win = &e->messagebar_win;
	}

	buf->next = buf;
	buf->prev = buf;

	buf->funcs[KEY_CTRL_A] = &uf_bol;
	buf->funcs[KEY_CTRL_B] = &uf_left;
	buf->funcs[KEY_CTRL_C] = &uf_cancel;

	buf->funcs[KEY_CTRL_D] = &uf_delete;
	buf->funcs[KEY_CTRL_E] = &uf_eol;
	buf->funcs[KEY_CTRL_F] = &uf_right;

	buf->funcs[KEY_CTRL_H] = &uf_backspace;
	buf->funcs[KEY_CTRL_I] = &uf_tab;

	buf->funcs[KEY_CTRL_S] = &uf_isearch_next;
	buf->funcs[KEY_CTRL_R] = &uf_isearch_previous;
	buf->funcs[KEY_CTRL_Z] = &uf_suspend;

	buf->funcs[KEY_ALT_Y] = &uf_paste;

	buf->funcs[KEY_RIGHT] = &uf_right;
	buf->funcs[KEY_LEFT] = &uf_left;

	buf->funcs[KEY_BACKSPACE] = &uf_backspace;
	buf->funcs[KEY_DELETE] = &uf_delete;
	buf->funcs[KEY_RESIZE] = &uf_resize;


	return buf;
}

Buffer *
buffer_new(Editor *e, char *filename) {
	Buffer *buf = malloc(sizeof(*buf));
	if (buf == NULL) {
		editor_show_message(e, "Out of memory");
		return NULL;
	}

	memset(buf, 0, sizeof(*buf));
	buf->filename = filename;

	buf->gbuf = gbf_new();
	if (buf->gbuf == NULL) {
		editor_show_message(e, "Out of memory");
		return NULL;
	}

	buf->redraw = 1;
	buf->draw = buffer_draw_func;
	buf->draw_statusbar = editor_draw_statusbar;

	buf->position.line = 1;
	buf->position.column = 1;

	if (e != NULL) {
		buf->win = &e->window;
		buf->statusbar_win = &e->statusbar_win;
		buf->messagebar_win = &e->messagebar_win;
	}

	buf->isearch_buffer = make_isearch_buffer(e);
	if (buf->isearch_buffer == NULL) {
		editor_show_message(e, "Out of memory");
		return NULL;
	}

	buf->next = buf;
	buf->prev = buf;

	buf->prev_func = &uf_resize;

	buf->funcs[KEY_CTRL_SPACE] = &uf_region_start_stop;
	buf->funcs[KEY_CTRL_A] = &uf_bol;
	buf->funcs[KEY_CTRL_B] = &uf_left;
	buf->funcs[KEY_CTRL_C] = &uf_region_off;
	buf->funcs[KEY_CTRL_D] = &uf_delete;
	buf->funcs[KEY_CTRL_E] = &uf_eol;
	buf->funcs[KEY_CTRL_F] = &uf_right;
	buf->funcs[KEY_CTRL_G] = &uf_prefix;
	buf->funcs[KEY_CTRL_H] = &uf_backspace;
	buf->funcs[KEY_CTRL_I] = &uf_tab;
	buf->funcs[KEY_CTRL_J] = &uf_prefix;

	buf->funcs[KEY_CTRL_M] = &uf_newline;
	buf->funcs[KEY_CTRL_N] = &uf_down;
	buf->funcs[KEY_CTRL_P] = &uf_up;
	buf->funcs[KEY_CTRL_Q] = &uf_resize;
	buf->funcs[KEY_CTRL_S] = &uf_isearch;
	buf->funcs[KEY_CTRL_U] = &uf_page_up;
	buf->funcs[KEY_CTRL_V] = &uf_page_down;
	buf->funcs[KEY_CTRL_W] = &uf_cut;
	buf->funcs[KEY_CTRL_Y] = &uf_paste;
	buf->funcs[KEY_CTRL_Z] = &uf_suspend;

	buf->funcs[KEY_ALT_V] = &uf_page_up;
	buf->funcs[KEY_ALT_W] = &uf_copy;

	buf->funcs[KEY_RIGHT] = &uf_right;
	buf->funcs[KEY_LEFT] = &uf_left;
	buf->funcs[KEY_DOWN] = &uf_down;
	buf->funcs[KEY_UP] = &uf_up;
	buf->funcs[KEY_HOME] = &uf_bol;
	buf->funcs[KEY_END] = &uf_eol;
	buf->funcs[KEY_PAGE_UP] = &uf_page_up;
	buf->funcs[KEY_PAGE_DOWN] = &uf_page_down;

	buf->funcs[KEY_F3] = &uf_macro_start_stop;
	buf->funcs[KEY_F4] = &uf_macro_play;
	buf->funcs[KEY_F8] = &uf_close_buffer;
	buf->funcs[KEY_F10] = &uf_quit;

	buf->funcs[KEY_BACKSPACE] = &uf_backspace;
	buf->funcs[KEY_DELETE] = &uf_delete;
	buf->funcs[KEY_RESIZE] = &uf_resize;


	if (filename != NULL) {
		struct stat st;

		if (stat(filename, &st) != 0) {
			if (errno == ENOENT) {
				return buf;
			} else {
				char out[1024];
				snprintf(out, 1023, "Cannot open %s", filename);
				editor_show_message(e, out);
				return NULL;
			}
		} else {
			char *text = malloc(st.st_size + 1);
			if (text == NULL) {
				editor_show_message(e, "Out of memory");
				return NULL;
			}

			FILE *file = fopen(filename, "r");
			long read = fread(text, st.st_size, 1, file);
			if (read != 1) {
				char out[1024];
				snprintf(out, 1023, "Cannot open %s", filename);
				editor_show_message(e, out);
				return NULL;
			}
			text[st.st_size] = '\0';
			gbf_insert(buf->gbuf, text, 0);
			free(text);
		}
	}

	return buf;
}

void
buffer_free(Buffer **buf) {
	if ((*buf)->next == *buf) {
		// Only one element.
		gbf_free(&(*buf)->gbuf);
		if ((*buf)->filename != NULL) {
			free((*buf)->filename);
		}
		if ((*buf)->isearch_buffer != NULL) {
			buffer_free(&(*buf)->isearch_buffer);
		}

		free(*buf);
		*buf = NULL;
	} else {
		// More than one element.
		Buffer *current = *buf;
		Buffer *b = (*buf)->prev;

		b->next = b->next->next;
		b->next->prev = b;

		gbf_free(&current->gbuf);
		if (current->filename != NULL) {
			free(current->filename);
		}
		if (current->isearch_buffer != NULL) {
			buffer_free(&current->isearch_buffer);
		}
		free(current);

		*buf = b;
	}
}

void
buffer_append(Buffer **current, Buffer *new) {
	if (*current == NULL) {
		*current = new;
	} else {
		Buffer *b = *current;
		new->next = b->next;
		new->prev = b;
		b->next->prev = new;
		b->next = new;
	}
}

static void
buffer_draw_func(Editor *e) {
	Buffer *b = e->current_buffer;
	Buffer *ib = e->current_buffer->isearch_buffer;
	size_t current = b->first_visible_char;
	size_t end = gbf_text_length(b->gbuf);
	size_t lines = b->win->size.lines;
	size_t columns = b->win->size.columns;
	size_t line = 0;
	size_t column = 0;
	size_t pcol = 0;
	int last_whitespace = 0;
	static size_t first_column = 0;

	e->current_buffer->draw_statusbar(e);
	if (e->shows_message) {
		e->shows_message = false;
	} else if (ib->isearch_is_active) {
		char *text = gbf_text(ib->gbuf);

		display_clear_line(*b->messagebar_win, 0);
		if (!ib->isearch_has_match) {
			pcol = display_show_string(e->messagebar_win, 0, pcol, "Failing ");
		}
		if (ib->isearch_has_wrapped) {
			pcol = display_show_string(e->messagebar_win, 0, pcol, "Wrapped ");
		}
		if (ib->isearch_direction == ISEARCH_DIRECTION_BACKWARD) {
			pcol = display_show_string(e->messagebar_win, 0, pcol, "Reverse ");
		}
		pcol = display_show_string(e->messagebar_win, 0, pcol, "ISearch: ");
		pcol = display_show_string(e->messagebar_win, 0, pcol, text);
		free(text);
	} else {
		display_clear_window(e->messagebar_win);
	}
	if (first_column > b->position.column) {
		first_column = 0;
		b->redraw = true;
	}
	while (b->position.column > first_column + columns) {
		first_column += columns / 2;
		b->redraw = true;
	}
	if (e->current_buffer->redraw) {

		display_clear_window(*b->win);

		while ((current < end) && (line < lines)) {
			char current_char = gbf_at(b->gbuf, current);
			char cp[5] = {0};

			if (b->region_type != REGION_OFF && current >= b->region_start && current < b->region_end) {
				display_set_color(INVERSE);
			}
			if (ib->isearch_has_match && current >= ib->isearch_match_start &&
				current < ib->isearch_match_end) {
				display_set_color(FOREGROUND_BLACK);
				display_set_color(BACKGROUND_GREEN);
			}
			if ((column >= first_column) && (column <= first_column + columns)) {
				// column is visible
				if (utf8_is_whitespace(current_char)) {
					if (last_whitespace == -1) {
						last_whitespace = column;
					}
				} else {
					last_whitespace = -1;
				}
				if (current_char == '\t') {
					for (size_t i = 0; i < 4; i++) {
						display_show_cp(*b->win, line, column + i, " ");
					}
				} else {
					for (size_t i = 0; i < utf8_byte_size(current_char); i++) {
						cp[i] = gbf_at(b->gbuf, current + i);
					}
					display_show_cp(*b->win, line, column - first_column, cp);
				}
			}
			if (current_char == '\n' || current == end - 1) {
				if (last_whitespace != -1) {
					if ((current == end - 1) && (gbf_at(b->gbuf, end - 1) != '\n')) {
						column += utf8_draw_width(gbf_at(b->gbuf, end - 1));
					}
					display_move_cursor(*b->win, line, last_whitespace);
					display_set_color(BACKGROUND_RED);
					for (size_t col = last_whitespace; col < column; col++) {
						display_show_cp(*b->win, line, col, " ");
					}
					display_set_color(OFF);
					last_whitespace = -1;
				}
				line++;
				column = 0;
			} else {
				column += utf8_draw_width(current_char);
			}
			current += utf8_byte_size(current_char);
			if (b->region_type != REGION_OFF && current == b->region_end) {
						display_set_color(OFF);
			}
			if (ib->isearch_has_match && current ==  ib->isearch_match_end) {
				display_set_color(OFF);
			}

		}
		b->redraw = 0;
	}
	if (ib->isearch_is_active) {
		display_move_cursor(*b->messagebar_win,
							ib->cursor.line,
							pcol);

	} else {
		display_move_cursor(*b->win,
							b->cursor.line,
							b->cursor.column - first_column);
	}
	display_refresh();

}
