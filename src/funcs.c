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
#include "search.h"

#define INITIAL_COPY_BUFFER_SIZE 4096

static int scroll_up(Buffer *buf);
static int scroll_down(Buffer *buf);
static void move_to_offset(Editor *e, size_t offset);
static size_t region_size(Buffer *b);


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
	e->string_arg = "\n";
	insert(e);
}

void
tab(Editor *e) {
	insert(e);
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

static void
move_to_offset(Editor *e, size_t offset) {
	Buffer *b = e->current_buffer;
	while (b->position.offset > offset) {
		left(e);
	}
	while (b->position.offset < offset) {
		right(e);
	}
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
	UserFunc *prev = e->current_buffer->prev_func;

	if (prev != NULL && (prev->func != up) && (prev->func != down)) {
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
	UserFunc *prev = e->current_buffer->prev_func;

	if (prev != NULL && (prev->func != up) && (prev->func != down)) {
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
isearch(Editor *e) {
	Buffer *ib = e->current_buffer->isearch_buffer;
	Buffer *tb = e->current_buffer;

	ib->isearch_start = tb->position.offset;
	ib->isearch_is_active = true;
	ib->isearch_has_match = true;
	ib->isearch_direction = ISEARCH_DIRECTION_FORWARD;
	ib->isearch_start = tb->position.offset;
	e->size_t_arg = gbf_text_length(tb->gbuf);

	while (!ib->cancel) {
		e->current_buffer = tb;
		e->current_buffer->draw(e);
		e->current_buffer = ib;

		editor_loop_once(e);

		size_t len = gbf_text_length(ib->gbuf);

		if (len != 0) {
			char *s = gbf_text(ib->gbuf);
			size_t off = 0;

			if (ib->isearch_direction == ISEARCH_DIRECTION_FORWARD) {
				ib->isearch_has_match = gbf_search(tb->gbuf, s, len, ib->isearch_start, &off);
			} else if (ib->isearch_direction == ISEARCH_DIRECTION_BACKWARD) {
				ib->isearch_has_match = gbf_search_reverse(tb->gbuf, s, len, ib->isearch_start, &off);
			}
			if (ib->isearch_has_match) {
				ib->isearch_match_start = off;
				ib->isearch_match_end = off + len;
			}
			e->current_buffer = tb;
			e->current_buffer->redraw = true;
			if (ib->isearch_has_match) {
				move_to_offset(e, off);
			}
			e->current_buffer = ib;
			free(s);
		}
	}

	ib->isearch_is_active = false;
	ib->isearch_has_match = false;
	ib->isearch_has_wrapped = false;
	ib->cancel = false;
	e->current_buffer = tb;
}

void
isearch_next(Editor *e){
	Buffer *b = e->current_buffer;

	if (gbf_text_length(b->gbuf) == 0){
		return;
	} else if (b->isearch_has_match) {
		b->isearch_start = b->isearch_match_start + 1;
		b->isearch_direction = ISEARCH_DIRECTION_FORWARD;
	} else if (!b->isearch_has_match && b->isearch_direction == ISEARCH_DIRECTION_FORWARD) {
		b->isearch_start = 0;
		b->isearch_has_wrapped = true;
	} else {
		b->isearch_direction = ISEARCH_DIRECTION_FORWARD;
	}
}

void
isearch_previous(Editor *e) {
	Buffer *b = e->current_buffer;

	if (gbf_text_length(b->gbuf) == 0){
		return;
	} else if (b->isearch_has_match) {
		b->isearch_start = b->isearch_match_start - 1;
		b->isearch_direction = ISEARCH_DIRECTION_BACKWARD;
	} else if (!b->isearch_has_match && b->isearch_direction == ISEARCH_DIRECTION_BACKWARD) {
		b->isearch_start = e->size_t_arg;
		b->isearch_has_wrapped = true;
	} else if (!b->isearch_has_match && b->isearch_direction == ISEARCH_DIRECTION_FORWARD) {
		b->isearch_start = b->isearch_start + gbf_text_length(b->gbuf);
		b->isearch_direction = ISEARCH_DIRECTION_BACKWARD;
	} else {
		b->isearch_direction = ISEARCH_DIRECTION_BACKWARD;
	}
}

void
region_start_stop(Editor *e) {
	Buffer *b = e->current_buffer;
	if (b->region_type == REGION_OFF) {
		editor_show_message(e, "Region active.");
		b->region_start = b->position.offset;
		b->region_end = b->position.offset;
		b->region_type = REGION_FLUID;
	} else if (b->region_type == REGION_FLUID) {
		if (b->region_start == b->region_end) {
			editor_show_message(e, "Region off.");
			b->region_type = REGION_OFF;
		} else {
			b->region_end = b->position.offset;
			b->region_type = REGION_ON;
			editor_show_message(e, "Region set.");
		}
	} else if (b->region_type == REGION_ON) {
		region_off(e);
		editor_show_message(e, "Region active.");
		b->region_start = b->position.offset;
		b->region_end = b->position.offset;
		b->region_type = REGION_FLUID;
	}
}

void
region_off(Editor *e) {
	Buffer *b = e->current_buffer;

	if (b->region_type == REGION_OFF) {
		return;
	}
	b->region_type = REGION_OFF;
	b->region_direction = REGION_DIRECTION_NONE;
	b->region_start = 0;
	b->region_end = 0;
	b->redraw = true;
	editor_show_message(e, "Cleared region.");
}

static size_t
region_size(Buffer *b) {
	return b->region_end - b->region_start;
}

void
copy(Editor *e) {
	Buffer *b = e->current_buffer;

	if (region_size(b) == 0) {
		editor_show_message(e, "No text selected.");
		return;
	}
	size_t len = region_size(b);

	if (e->copy_buffer == NULL) {
		e->copy_buffer = malloc(INITIAL_COPY_BUFFER_SIZE);
		if (e->copy_buffer == NULL) {
			editor_show_message(e, "Copy failed (Out of memory)");
			e->copy_buffer_size = 0;
			e->copy_bytes_written = 0;
			return;
		}
		e->copy_buffer_size = INITIAL_COPY_BUFFER_SIZE;
	}
	if (e->copy_buffer_size < len + 1) {
		e->copy_buffer = realloc(e->copy_buffer, len + 1);
		if (e->copy_buffer == NULL) {
			editor_show_message(e, "Copy failed (Out of memory)");
			e->copy_buffer_size = 0;
			e->copy_bytes_written = 0;
			return;
		}
		e->copy_buffer_size = len + 1;
	}
	e->copy_bytes_written = 0;
	for (size_t i = 0; i < len; i++) {
		e->copy_buffer[i] = gbf_at(b->gbuf, b->region_start + i);
		e->copy_bytes_written++;
	}
	e->copy_buffer[e->copy_bytes_written] = '\0';
	editor_show_message(e, "Copied text.");
}

void
cut(Editor *e) {
	Buffer *b = e->current_buffer;

	if (region_size(b) == 0) {
		editor_show_message(e, "No text selected.");
		return;
	}

	copy(e);

	if (e->copy_bytes_written == 0) {
		return;
	}

	move_to_offset(e, b->region_start);
	for (size_t i = 0; i < region_size(b); i++) {
		delete(e);
	}
	region_off(e);
	editor_show_message(e, "Cut text.");
}

void
paste(Editor *e) {
	Buffer *b = e->current_buffer;

	if (e->copy_bytes_written == 0) {
		editor_show_message(e, "No text to paste.");
		return;
	}
	gbf_insert(b->gbuf, e->copy_buffer, b->position.offset);
	b->redraw = true;
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

	if (e->recording_macro) {
		macro_start_stop(e);
	}
	if (e->macro_bytes_written == 0) {
		editor_show_message(e, "No macro recorded.");
	}
	while (e->macro_buffer[i] != '\0') {
		char buffer[32] = {'\0'};
		KeyCode c;

		strcpy(buffer, e->macro_buffer + i);
		c = input_check(buffer);

		editor_call_userfunc(e, c);

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
prefix(Editor *e) {
	char input[32] = {0};
	KeyCode c;
	UserFunc *uf = NULL;

	c = input_get(input);

	switch(c) {
	case KEY_CTRL_C: uf = &uf_quit; break;
	case KEY_CTRL_K: uf = &uf_close_buffer; break;
	case KEY_CTRL_N: uf = &uf_previous_buffer; break;
	case KEY_CTRL_O: uf = &uf_openfile; break;
	case KEY_CTRL_P: uf = &uf_next_buffer; break;
	case KEY_CTRL_S: uf = &uf_save; break;
	case KEY_CTRL_W: uf = &uf_save_as; break;
	}
	if (uf != NULL) {
		uf->func(e);
		e->current_buffer->prev_func = uf;
	}
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
		MenuResult force = menu_yes_no(e, "Force newline? (yes/no)");

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
		r = menu_yes_no(e, "Buffer has changed. Save? (yes/no)");
		if (r == MENU_YES) {
			save(e);
		} else if (r == MENU_CANCEL) {
			editor_show_message(e, "Cancel");
			e->current_buffer->cancel = true;
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
	while (e->current_buffer != NULL) {
		close_buffer(e);
		if (e->current_buffer->cancel == true) {
			e->current_buffer->cancel = false;
			return;
		}
	}
	exit(0);
}
