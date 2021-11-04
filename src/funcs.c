#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "display.h"
#include "funcs.h"
#include "gapbuffer.h"
#include "userfuncs.h"
#include "input.h"
#include "buffer.h"
#include "editor.h"
#include "menus.h"
#include "utf8.h"


static int scroll_up(Buffer *buf);
static int scroll_down(Buffer *buf);


void
insert(Editor *e) {
	Buffer *b = e->current_buffer;
	gbf_insert(b->gbuf, e->string_arg, b->position.offset);
	right(e);
	b->has_changed = true;
	b->redraw = true;
}

void
newline(Editor *e) {
	Buffer *b = e->current_buffer;
	gbf_insert(b->gbuf, "\n", b->position.offset);
	right(e);
	b->has_changed = true;
	b->redraw = true;
}

void
tab(Editor *e) {
	Buffer *b = e->current_buffer;
	gbf_insert(b->gbuf, "\t", b->position.offset);
	right(e);
	b->has_changed = true;
	b->redraw = true;
}

void
delete(Editor *e) {
	Buffer *b = e->current_buffer;

	if (b->position.offset == gbf_text_length(b->gbuf)) {
		return;
	}
	size_t bytes = utf8_byte_size(gbf_at(b->gbuf, b->position.offset));
	gbf_delete(b->gbuf, b->position.offset, bytes);
	b->has_changed = true;
	b->redraw = true;
}

void
backspace (Editor *e) {
	Buffer *b = e->current_buffer;

	if (b->position.offset == 0) {
		return;
	}
	left(e);
	delete(e);
}

// Scrolls up by one line. Returns 1 on success, 0 on failure.
static int
scroll_up(Buffer *b) {
	int newlines = 0;

	if (b->first_visible_char == 0) {
		return false;
	}
	do {
		b->first_visible_char--;
		if (gbf_at(b->gbuf, b->first_visible_char) == '\n') {
			newlines++;
		}
	} while ((newlines != 2) && (b->first_visible_char != 0));

	if (b->first_visible_char != 0) {
		b->first_visible_char++;
	}
	b->redraw = true;
	return true;
}

// Scrolls down by one line. Returns 1 on success, 0 on failure.
static int
scroll_down(Buffer *b) {
	size_t start = b->first_visible_char;

	while (gbf_at(b->gbuf, b->first_visible_char) != '\n') {
		if (b->first_visible_char == gbf_text_length(b->gbuf)) {
			b->first_visible_char = start;
			return false;
		}
		b->first_visible_char += utf8_byte_size(gbf_at(b->gbuf, b->first_visible_char));
	}
	b->first_visible_char += utf8_byte_size(gbf_at(b->gbuf, b->first_visible_char));
	b->redraw = true;

	return true;
}
void
left(Editor *e) {
	Buffer *b = e->current_buffer;

	if(b->position.offset == 0) {
		return;
	}

	if (b->position.column == 1) {
		b->position.line--;
		if (b->cursor.line == 0) {
			scroll_up(b);
		} else {
			b->cursor.line--;
		}
		do {
			b->position.offset--;
		} while (!utf8_is_valid_first_byte(b->position.offset));

		size_t s = b->position.offset;
		size_t col = 0;

		while ((s != 0) && (gbf_at(b->gbuf, s - 1) != '\n')) {
			do {
				s--;
			} while (!utf8_is_valid_first_byte(s));
			col += utf8_draw_width(gbf_at(b->gbuf, s));
		}
		b->cursor.column = col;
		b->position.column = col + 1;
	} else {
		do {
			b->position.offset--;
		} while (!utf8_is_valid_first_byte(b->position.offset));
		b->cursor.column -= utf8_draw_width(gbf_at(b->gbuf, b->position.offset));
		b->position.column -= utf8_draw_width(gbf_at(b->gbuf, b->position.offset));
	}
}

void
right(Editor *e) {
	Buffer *b = e->current_buffer;

	if (b->position.offset == gbf_text_length(b->gbuf)) {
		return;
	}
	if (gbf_at(b->gbuf, b->position.offset) == '\n') {
		if (b->cursor.line == b->win->size.lines - 1) {
			scroll_down(b);
		} else {
			b->cursor.line++;
		}
		b->position.line++;
		b->cursor.column = 0;
		b->position.column = 1;
	} else {
		size_t width = utf8_draw_width(gbf_at(b->gbuf, b->position.offset));
		b->cursor.column += width;
		b->position.column += width;
	}
	size_t bytes = utf8_byte_size(gbf_at(b->gbuf, b->position.offset));
	b->position.offset += bytes;
}

void
up(Editor *e) {
	Buffer *b = e->current_buffer;
	Func prev = e->current_buffer->prev_func;

	if ((prev != up) && (prev != down)) {
		b->target_column = b->position.column;
	}

	bol(e);
	do {
		left(e);
	} while (b->position.column > b->target_column);
}

void
down(Editor *e) {
	Buffer *b = e->current_buffer;
	Func prev = e->current_buffer->prev_func;

	if ((prev != up) && (prev != down)) {
		b->target_column = b->position.column;
	}

	// Move the cursor to the beginning of the next line.
	do {
		right(e);
	} while ((b->position.column > 1) && (b->position.offset != gbf_text_length(b->gbuf)));

	// Move the cursor to the target column.
	while ((gbf_at(b->gbuf, b->position.offset) != '\n') &&
		   (b->position.column < b->target_column) &&
		   (b->position.offset != gbf_text_length(b->gbuf))) {
		right(e);
	}
}

void
bol(Editor *e) {
	Buffer *b = e->current_buffer;
	while (b->position.column > 1) {
		left(e);
	}
}

void
eol(Editor *e) {
	Buffer *b = e->current_buffer;
	while ((gbf_at(b->gbuf, b->position.offset) != '\n') &&
		   (b->position.offset < gbf_text_length(b->gbuf))) {
		right(e);
	}
}

void
page_up(Editor *e) {
	Buffer *b = e->current_buffer;
	size_t lines = b->win->size.lines;
	size_t l = b->cursor.line;

	for (size_t i = 0; i < lines - 1; i++) {
		if (!scroll_up(b)) {
			break;
		}
	}
	b->position.offset = b->first_visible_char;
	b->position.line -= l - 1;
	b->position.column = 1;
	b->cursor.line = 0;
	b->cursor.column = 0;
}

void
page_down(Editor *e) {
	Buffer *b = e->current_buffer;
	size_t start = b->first_visible_char;
	size_t lines = b->win->size.lines;
	size_t l = b->cursor.line;

	for (size_t i = 0; i < lines - 1; i++) {
		if (!scroll_down(b)) {
			b->first_visible_char = start;
			return;
		}
	}
	b->position.offset = b->first_visible_char;
	b->position.line += lines - l - 1;
	b->position.column = 1;
	b->cursor.line = 0;
	b->cursor.column = 0;
}

void
macro_start_stop(Editor *e) {
	if (e->recording_macro) {
		editor_show_message(e, "Stopped recording");
		e->recording_macro = false;
		e->macro_buffer[e->macro_bytes_written] = '\0';
	} else {
		editor_show_message(e, "Recording");
		e->recording_macro = true;
		e->macro_bytes_written = 0;
		e->macro_buffer[0] = '\0';
	}
}

void
macro_append(Editor *e) {
	size_t len = strlen(e->string_arg);

	strcpy(e->macro_buffer + e->macro_bytes_written, e->string_arg);
	e->macro_buffer[e->macro_bytes_written + len] = '\0';
	e->macro_bytes_written += len + 1;

	if (e->macro_bytes_written > MACRO_BUFFER_SIZE - 32) {
		editor_show_message(e, "Macro buffer full. Stopped recording.");
		e->recording_macro = false;
	}
}

void
macro_play(Editor *e) {
	size_t i = 0;

	if (e->macro_bytes_written == 0) {
		editor_show_message(e, "No macro recorded.");
	}
	while (e->macro_buffer[i] != '\0') {
		char buffer[32] = {'\0'};
		KeyCode c;

		strcpy(buffer, e->macro_buffer + i);
		c = input_check(buffer);

		if (c >= KEY_SPECIAL_MIN && c <= KEY_SPECIAL_MAX) {
			Func f = e->current_buffer->funcs[c].func;
			if (f != NULL) {
				f(e);
				e->current_buffer->prev_func = f;
			}
		} else if (c == KEY_VALID) {
			e->string_arg = buffer;
			insert(e);
			e->current_buffer->prev_func = insert;
		} else {
			editor_show_message(e, "Unrecognized input");
		}
		i += strlen(buffer) + 1;
	}
}

void
next_buffer(Editor *e) {
	e->current_buffer = e->current_buffer->next;
	e->current_buffer->redraw = 1;
}

void
previous_buffer(Editor *e) {
	e->current_buffer = e->current_buffer->prev;
	e->current_buffer->redraw = 1;
}

void
resize(Editor *e) {
	display_set_size(&e->display);

	display_move_window(&e->window, 1, 1);
	display_resize_window(&e->window, e->display.lines - 2, e->display.columns);

	display_move_window(&e->statusbar_win, e->display.lines - 1, 1);
	display_move_window(&e->messagebar_win, e->display.lines, 1);

	display_resize_window(&e->statusbar_win, 1, e->display.columns);
	display_resize_window(&e->messagebar_win, 1, e->display.columns);

	if (e->current_buffer != NULL) {
		e->current_buffer->redraw = true;
	}
}

void
suspend(Editor *unused) {
	(void)unused;
	kill(0, SIGSTOP);
}

void
ok(Editor *e) {
	e->current_buffer->ok = true;
}

void
cancel(Editor *e) {
	e->current_buffer->cancel = true;
}

void
openfile(Editor *e) {
	char *text = menu_choose_file(e);
	if (text == NULL) {
		editor_show_message(e, "Cancel");
	} else {
		Buffer *b = buffer_new(e, text);
		if (b != NULL) {
			buffer_append(&(e->current_buffer), b);
			e->current_buffer = e->current_buffer->next;
			e->current_buffer->redraw = true;
			editor_show_message(e, text);
		} else {
			editor_show_message(e, "Cannot add buffer");
		}
	}
}

void
save(Editor *e) {
	Buffer *b = e->current_buffer;

	if (b->filename == NULL) {
		save_as(e);
		return;
	}
	size_t length = gbf_text_length(b->gbuf);
	FILE *fd;

	if (gbf_at(b->gbuf, length - 1) != '\n') {
		MenuResult force = menu_yes_no(e, "Force newline? ");

		if (force == MENU_YES) {
			gbf_insert(b->gbuf, "\n", length);
			length++;
		} else {
			editor_show_message(e, "Cancel");
			return;
		}
	}

	if((fd = fopen(b->filename, "w")) == NULL ) {
		editor_show_message(e, "Cannot save.");
		return;
	}

	char *text = gbf_text(b->gbuf);
	if (fwrite(text, 1, length, fd) != length) {
		editor_show_message(e, "Cannot save.");
	} else {
		b->has_changed = false;
		editor_show_message(e, "Wrote file.");
	}
	fclose(fd);
	free(text);
}

void
save_as(Editor *e) {
	Buffer *b = e->current_buffer;
	char *file = menu_choose_file(e);

	if (file == NULL) {
		editor_show_message(e, "Cancel");
		return;
	}
	struct stat st;

	if (stat(file, &st) == 0) {
		MenuResult r = menu_yes_no(e, "File exists. Overwrite? (yes/no): ");
		if ((r == MENU_NO) || (r == MENU_CANCEL)) {
			free(file);
			editor_show_message(e, "Cancel");
			return;
		}
	}

	size_t length = gbf_text_length(b->gbuf);
	FILE *fd;

	if (gbf_at(b->gbuf, length - 1) != '\n') {
		MenuResult force = menu_yes_no(e, "Force newline? ");
		if (force == MENU_YES) {
			gbf_insert(b->gbuf, "\n", length);
			length++;
		} else if (force == MENU_CANCEL){
			editor_show_message(e, "Cancel");
			free(file);
			return;
		}
	}
	if (b->filename != NULL) {
		free(b->filename);
	}
	b->filename = file;

	if((fd = fopen(b->filename, "w")) == NULL ) {
		editor_show_message(e, "Cannot save.");
		free(file);
		return;
	}

	char *text = gbf_text(b->gbuf);
	if (fwrite(text, 1, length, fd) != length) {
		editor_show_message(e, "Cannot save.");
	} else {
		b->has_changed = false;
		editor_show_message(e, "Wrote file.");
	}
	fclose(fd);
	free(text);
}

void
close_buffer(Editor *e) {
	MenuResult r = false;
	if (e->current_buffer->has_changed) {
		r = menu_yes_no(e, "Buffer has changed. Save? ");
		if (r == MENU_YES) {
			save(e);
		} else if (r == MENU_CANCEL) {
			editor_show_message(e, "Cancel");
			e->quit = false;
			return;
		}
	}
	buffer_free(&e->current_buffer);
	if (e->current_buffer == NULL) {
		quit(e);
	}
}

void
quit(Editor *e) {
	e->quit = true;
}
