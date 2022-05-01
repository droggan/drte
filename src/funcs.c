#define _POSIX_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "display.h"
#include "funcs.h"
#include "gapbuffer.h"
#include "chunk_list.h"
#include "menus.h"
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


UserFunc uf_insert = {
	.type = USER_FUNC_INSERTION,
	.name = "insert",
	.description = "",
	.func = insert
};

void
insert(Editor *e) {
	Buffer *b = e->current_buffer;
	gbf_insert(b->gbuf, e->string_arg, b->position.offset);
	right(e);
	b->has_changed = true;
	b->redraw = true;
}

UserFunc uf_newline = {
	.type = USER_FUNC_INSERTION,
	.name = "newline",
	.description = "Inserts a newline at the current position.",
	.func = newline
};

void
newline(Editor *e) {
	e->string_arg = "\n";
	insert(e);
}

UserFunc uf_tab = {
	.type = USER_FUNC_INSERTION,
	.name = "tab",
	.description = "Inserts a tab at the current position.",
	.func = tab
};

void
tab(Editor *e) {
	insert(e);
}

UserFunc uf_delete = {
	.type = USER_FUNC_DELETION,
	.name = "delete",
	.description = "Deletes the character under the cursor.",
	.func = delete
};

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

UserFunc uf_backspace = {
	.type = USER_FUNC_DELETION,
	.name = "backspace",
	.description = "Deletes the character before the cursor.",
	.func = backspace
};

void
backspace (Editor *e) {
	Buffer *b = e->current_buffer;

	if (b->position.offset == 0) {
		return;
	}
	left(e);
	delete(e);
}

UserFunc uf_menu_up = {
	.type = USER_FUNC_MOVEMENT,
	.name = "menu_up",
	.description = "Select the previous menu item.",
	.func = menu_up
};

void
menu_up(Editor *e) {
	Buffer *b = e->current_buffer;
	MenuItemList *items = b->menu_items;

	if (items->selected == NULL) {
		items->selected = items->first_visible_item;
	} else {
		MenuItem *start = b->menu_items->selected;
		bool found = false;
		while (b->menu_items->selected->prev != NULL) {
			b->menu_items->selected = b->menu_items->selected->prev;
			if (b->menu_items->selected->is_visible) {
				found = true;
				break;
			}
		}
		if (!found) {
			b->menu_items->selected = start;
		}
	}
}

UserFunc uf_menu_down = {
	.type = USER_FUNC_MOVEMENT,
	.name = "menu_down",
	.description = "Select the next menu item.",
	.func = menu_down
};

void
menu_down(Editor *e) {
	Buffer *b = e->current_buffer;
	MenuItemList *items = b->menu_items;

	if (items->selected == NULL) {
		items->selected = items->first_visible_item;
	} else {
		MenuItem *start = b->menu_items->selected;
		bool found = false;
		while (b->menu_items->selected->next != NULL) {
			b->menu_items->selected = b->menu_items->selected->next;
			if (b->menu_items->selected->is_visible) {
				found = true;
				break;
			}
		}
		if (!found) {
			b->menu_items->selected = start;
		}
	}
}

UserFunc uf_menu_tab = {
	.type = USER_FUNC_MOVEMENT,
	.name = "menu_tab",
	.description = "Cycle through matching menu items.",
	.func = menu_tab
};

void
menu_tab(Editor *e) {
	Buffer *b = e->current_buffer;
	MenuItem *start = b->menu_items->selected;
	bool found = false;

	if (b->menu_items->selected == NULL) {
		menu_down(e);
		return;
	}
	while (b->menu_items->selected->next != NULL) {
		b->menu_items->selected = b->menu_items->selected->next;
		if (b->menu_items->selected->is_visible) {
			found = true;
			break;
		}
	}
	if (!found) {
		b->menu_items->selected = b->menu_items->first;

		while (b->menu_items->selected != start) {
			if (b->menu_items->selected->is_visible) {
				found = true;
				break;
			}
			b->menu_items->selected = b->menu_items->selected->next;
		}
	}

}

UserFunc uf_toggle_show_hidden_files = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "toggle_show_hidden_files.",
	.description = "Show/hide hidden files.",
	.func = toggle_show_hidden_files
};

void
toggle_show_hidden_files(Editor *e) {
	if (e->current_buffer->show_hidden_files) {
		e->current_buffer->show_hidden_files = false;
	} else {
		e->current_buffer->show_hidden_files = true;
	}
	e->current_buffer->has_changed = true;
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

UserFunc uf_left = {
	.type = USER_FUNC_MOVEMENT,
	.name = "left",
	.description = "Moves the cursor backward.",
	.func = left
};

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

UserFunc uf_right = {
	.type = USER_FUNC_MOVEMENT,
	.name = "right",
	.description = "Moves the cursor forward.",
	.func = right
};

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

UserFunc uf_up = {
	.type = USER_FUNC_MOVEMENT,
	.name = "up",
	.description = "Moves the cursor up.",
	.func = up
};

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

UserFunc uf_down = {
	.type = USER_FUNC_MOVEMENT,
	.name = "down",
	.description = "Moves the cursor down",
	.func = down
};

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

UserFunc uf_bol = {
	.type = USER_FUNC_MOVEMENT,
	.name = "bol",
	.description = "Moves the cursor to the beginning of the current line",
	.func = bol
};

void
bol(Editor *e) {
	Buffer *b = e->current_buffer;
	while (b->position.column > 1) {
		left(e);
	}
}

UserFunc uf_eol = {
	.type = USER_FUNC_MOVEMENT,
	.name = "eol",
	.description = "Moves the cursor to the end of the current line.",
	.func = eol
};

void
eol(Editor *e) {
	Buffer *b = e->current_buffer;
	while ((gbf_at(b->gbuf, b->position.offset) != '\n') &&
		   (b->position.offset < gbf_text_length(b->gbuf))) {
		right(e);
	}
}

UserFunc uf_page_up = {
	.type = USER_FUNC_MOVEMENT,
	.name = "page_up",
	.description = "Scrolls up by one screen.",
	.func = page_up
};

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

UserFunc uf_page_down = {
	.type = USER_FUNC_MOVEMENT,
	.name = "page_down",
	.description = "Scrolls down by one screen.",
	.func = page_down
};

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

UserFunc uf_isearch = {
	.type = USER_FUNC_MOVEMENT,
	.name = "isearch",
	.description = "Start searching at the current cursor position.",
	.func = isearch
};

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

UserFunc uf_isearch_next = {
	.type = USER_FUNC_MOVEMENT,
	.name = "isearch_next",
	.description = "Go to the next result.",
	.func = isearch_next
};

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

UserFunc uf_isearch_previous = {
	.type = USER_FUNC_MOVEMENT,
	.name = "isearch_previous",
	.description = "Go to the previous result.",
	.func = isearch_previous
};

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

UserFunc uf_region_start_stop = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "region_start_stop",
	.description = "Start or stop selecting text.",
	.func = region_start_stop
};

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

UserFunc uf_region_off = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "region_off",
	.description = "Clear the current region.",
	.func = region_off
};

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

UserFunc uf_copy = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "copy",
	.description = "Copy the selected text.",
	.func = copy
};

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

UserFunc uf_cut = {
	.type = USER_FUNC_DELETION,
	.name = "cut",
	.description = "Cut the selected text.",
	.func = cut
};

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

UserFunc uf_paste = {
	.type = USER_FUNC_INSERTION,
	.name = "paste",
	.description = "Paste previously cut/copied text.",
	.func = paste
};

void
paste(Editor *e) {
	if (e->copy_bytes_written == 0) {
		editor_show_message(e, "No text to paste.");
		return;
	}
	size_t i = 0;
	while (i < e->copy_bytes_written) {
		size_t s = utf8_byte_size(e->copy_buffer[i]);
		strncpy(e->string_arg, e->copy_buffer + i, s);
		insert(e);
		i += s;
	}
}

UserFunc uf_macro_start_stop = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "macro_start_stop",
	.description = "Start or stop recording a macro.",
	.func = macro_start_stop
};

void
macro_start_stop(Editor *e) {
	if (e->macro_info.recording_macro) {
		editor_show_message(e, "Stopped recording");
		e->macro_info.recording_macro = false;
	} else {
		editor_show_message(e, "Recording");
		e->macro_info.recording_macro = true;
		while (e->macro_info.first != NULL) {
			MacroElement *m = e->macro_info.first;
			e->macro_info.first = e->macro_info.first->next;
			free(m->text);
			free(m);
		}
		e->macro_info.first = NULL;
		e->macro_info.last = NULL;
	}
}


void
macro_append(Editor *e, UserFunc *uf, char *arg) {
	MacroElement *me = malloc(sizeof(*me));

	if (me == NULL) {
		editor_show_message(e, "Cannot save macro. Out of memory");
		e->macro_info.recording_macro = false;
		return;
	}
	me->uf = uf;
	if (arg != NULL) {
		ChunkListItem *ci = chunk_list_insert(e->macro_info.chunk_list, arg);
		me->text = ci;
	}
	me->next = NULL;

	if (e->macro_info.last == NULL) {
		e->macro_info.first = me;
		e->macro_info.last = me;
	} else {
		e->macro_info.last->next = me;
		e->macro_info.last = me;
	}
}

UserFunc uf_macro_play = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "macro_play",
	.description = "Play a previously recorded macro.",
	.func = macro_play
};

void
macro_play(Editor *e) {
	if (e->macro_info.recording_macro) {
		macro_start_stop(e);
	}
	for (MacroElement *me = e->macro_info.first; me != NULL; me = me->next) {
		if (me->text != NULL) {
			e->string_arg = chunk_list_get_item(me->text);
		}
		me->uf->func(e);
		free(e->string_arg);
	}
}

UserFunc uf_next_buffer = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "next_buffer",
	.description = "Switch to the next buffer.",
	.func = next_buffer
};

void
next_buffer(Editor *e) {
	e->current_buffer = e->current_buffer->next;
	e->current_buffer->redraw = 1;
}

UserFunc uf_previous_buffer = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "previous_buffer",
	.description = "Switch to the previous buffer.",
	.func = previous_buffer
};

void
previous_buffer(Editor *e) {
	e->current_buffer = e->current_buffer->prev;
	e->current_buffer->redraw = 1;
}

UserFunc uf_resize = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "resize",
	.description = "Resize the editor.",
	.func = resize
};

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

UserFunc uf_suspend = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "suspend",
	.description = "Suspend the editor.",
	.func = suspend
};

void
suspend(Editor *unused) {
	(void)unused;
	kill(0, SIGSTOP);
}

UserFunc uf_ok = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "ok",
	.description = "", // TODO
	.func = ok
};

void
ok(Editor *e) {
	e->current_buffer->ok = true;
}

UserFunc uf_cancel = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "cancel",
	.description = "", // TODO
	.func = cancel
};

void
cancel(Editor *e) {
	e->current_buffer->cancel = true;
}

UserFunc uf_timeout = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "timeout",
	.description = "", // TODO
	.func = timeout
};

void
timeout(Editor *e) {
	e->current_buffer->timeout = true;
}

static void
prefix_draw_func(Editor *e) {
	Buffer *b = e->current_buffer;

	if (b->timeout) {
		// TODO: generate from key bindings and UserFuncs
		display_clear_window(*b->messagebar_win);
		display_clear_window(*b->win);
		display_show_string(*b->win, 0, 0, "Ctrl+B Switch Buffer");
		display_show_string(*b->win, 1, 0, "Ctrl+C Cancel");
		display_show_string(*b->win, 2, 0, "Ctrl+K Close Buffer");
		display_show_string(*b->win, 3, 0, "Ctrl+N Previous Buffer");
		display_show_string(*b->win, 4, 0, "Ctrl+O Open File");
		display_show_string(*b->win, 5, 0, "Ctrl+P Next Buffer");
		display_show_string(*b->win, 6, 0, "Ctrl+Q Quit");
		display_show_string(*b->win, 7, 0, "Ctrl+S Save");
		display_show_string(*b->win, 8, 0, "Ctrl+W Save As");
	} else {
		display_show_string(*b->messagebar_win, 0, 0, "Prefix");
		display_move_cursor(*b->win, b->position.line - 1, b->position.column - 1);
	}
	display_refresh();
}

static Buffer *
make_prefix_buffer(Editor *e) {
	Buffer *buf = malloc(sizeof(*buf));
	if (buf == NULL) {
		editor_show_message(e, "Out of memory");
		return NULL;
	}

	memset(buf, 0, sizeof(*buf));

	buf->redraw = 1;
	buf->draw = prefix_draw_func;
	buf->draw_statusbar = editor_draw_statusbar;

	buf->position.line = 1;
	buf->position.column = 1;

	buf->win = &e->window;
	buf->statusbar_win = &e->statusbar_win;
	buf->messagebar_win = &e->messagebar_win;

	buf->next = buf;
	buf->prev = buf;

	buffer_bind_key(buf, KEY_CTRL_B, &uf_switch_buffer);
	buffer_bind_key(buf, KEY_CTRL_C, &uf_cancel);
	buffer_bind_key(buf, KEY_CTRL_K, &uf_close_buffer);
	buffer_bind_key(buf, KEY_CTRL_N, &uf_previous_buffer);
	buffer_bind_key(buf, KEY_CTRL_O, &uf_openfile);
	buffer_bind_key(buf, KEY_CTRL_P, &uf_next_buffer);
	buffer_bind_key(buf, KEY_CTRL_Q, &uf_quit);
	buffer_bind_key(buf, KEY_CTRL_S, &uf_save);
	buffer_bind_key(buf, KEY_CTRL_W, &uf_save_as);

	buffer_bind_key(buf, KEY_TIMEOUT, &uf_timeout);

	return buf;
}

UserFunc uf_prefix = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "prefix",
	.description = "",
	.func = prefix
};

void
prefix(Editor *e) {
	char input[32] = {0};
	KeyCode c;
	UserFunc *uf = NULL;
	Buffer *b = make_prefix_buffer(e);
	Buffer *tmp = e->current_buffer;

	e->current_buffer = b;
	b->position = tmp->position;
	display_set_timeout(5);
	do {
		prefix_draw_func(e);
		c = input_get(input);

		if (c >= KEY_SPECIAL_MIN && c <= KEY_SPECIAL_MAX) {
			uf = b->funcs[input_key_to_id(c)];
		}
		if (uf != NULL) {
			if (c != KEY_TIMEOUT) {
				b->cancel = true;
				e->current_buffer = tmp;
				display_show_cursor();
				uf->func(e);
			} else {
				display_hide_cursor();
				uf->func(e);
				display_clear_timeout();
			}
		}
	} while (!b->cancel);

	buffer_free(&b);
	e->current_buffer->cancel = false;
	e->current_buffer->redraw = true;
}

UserFunc uf_openfile = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "open",
	.description = "Open a file.",
	.func = openfile
};

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

UserFunc uf_switch_buffer = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "switch_buffer",
	.description = "Switch to a different buffer.",
	.func = switch_buffer
};

void
switch_buffer(Editor *e) {
	char *text = menu_choose_buffer(e);
	Buffer *start = e->current_buffer;
	if (text == NULL) {
		editor_show_message(e, "Cancel");
	} else {
		while (strcmp(e->current_buffer->filename, text) != 0) {
			e->current_buffer = e->current_buffer->next;
			if (e->current_buffer == start) {
				editor_show_message(e, "Buffer not found");
				break;
			}
		}
		free(text);
	}
}

UserFunc uf_save = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "save",
	.description = "Saves the current buffer.",
	.func = save
};

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

UserFunc uf_save_as = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "Save As",
	.description = "Save the current buffer under a different name", // TODO: better descrption
	.func = save_as
};

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

UserFunc uf_close_buffer = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "close_buffer",
	.description = "Close the current buffer.",
	.func = close_buffer
};

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

UserFunc uf_quit = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "quit",
	.description = "Quit.",
	.func = quit
};

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
