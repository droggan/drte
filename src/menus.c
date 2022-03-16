#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "gapbuffer.h"
#include "display.h"
#include "input.h"
#include "userfuncs.h"
#include "funcs.h"
#include "chunk_list.h"
#include "menus.h"
#include "buffer.h"
#include "editor.h"
#include "utf8.h"


static MenuItemList *new_menu_item_list(void);
static MenuItem *menu_item_list_insert(MenuItemList *list, char *item);
static void free_menu_item_list(MenuItemList **list);
static void file_chooser_draw_func(Editor *b);
static Buffer *make_file_chooser_buffer(Editor *e);
static void yes_no_draw_func(Editor *e);
static Buffer *make_yes_no_buffer(Editor *e);


static MenuItemList *
new_menu_item_list(void) {
	MenuItemList *list = malloc(sizeof(*list));
	if (list == NULL) {
		return NULL;
	}
	memset(list, 0, sizeof(*list));

	list->chunk_list = chunk_list_new(0);
	if (list->chunk_list == NULL) {
		return NULL;
	}

	return list;
}

static MenuItem *
menu_item_list_insert(MenuItemList *list, char *item) {
	MenuItem *new_item = malloc(sizeof(*new_item));
	if (new_item == NULL) {
		return NULL;
	}

	ChunkListItem *citem = chunk_list_insert(list->chunk_list, item);
	if (citem == NULL) {
		return NULL;
	}

	new_item->item = citem;
	new_item->next = list->first;
	if (new_item->next != NULL) {
		new_item->next->prev = new_item;
	}
	new_item->prev = NULL;
	list->first = new_item;
	list->first_visible_item = new_item;

	return new_item;
}

static void
free_menu_item_list(MenuItemList **list) {
	chunk_list_free(&(*list)->chunk_list);
	while ((*list)->first != NULL) {
		MenuItem *i = (*list)->first;
		(*list)->first = (*list)->first->next;
		free(i->item);
		free(i);
	}
	free(*list);
	*list = NULL;
}

static void
file_chooser_draw_func(Editor *e) {
	Buffer *b = e->current_buffer;
	char *text = gbf_text(b->gbuf);

	display_clear_window(*b->messagebar_win);

	size_t col = display_show_string(*b->messagebar_win, 0, 0, "File: ");
	display_show_string(*b->messagebar_win, 0, col, text);
	display_move_cursor(*b->messagebar_win, b->cursor.line, b->cursor.column + col);

	free(text);
	display_refresh();
}

static Buffer *
make_file_chooser_buffer(Editor *e) {
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
	buf->draw = file_chooser_draw_func;
	buf->draw_statusbar = editor_draw_statusbar;

	buf->position.line = 1;
	buf->position.column = 1;

	buf->win = &e->window;
	buf->statusbar_win = &e->statusbar_win;
	buf->messagebar_win = &e->messagebar_win;

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
	buf->funcs[KEY_CTRL_M] = &uf_ok;
	buf->funcs[KEY_CTRL_Z] = &uf_suspend;

	buf->funcs[KEY_RIGHT] = &uf_right;
	buf->funcs[KEY_LEFT] = &uf_left;

	buf->funcs[KEY_HOME] = &uf_bol;
	buf->funcs[KEY_END] = &uf_eol;

	buf->funcs[KEY_BACKSPACE] = &uf_backspace;
	buf->funcs[KEY_DELETE] = &uf_delete;
	buf->funcs[KEY_RESIZE] = &uf_resize;

	return buf;
}

char *
menu_choose_file(Editor *e) {
	Buffer *b = make_file_chooser_buffer(e);
	Buffer *tmp = e->current_buffer;
	char *ret = NULL;

	e->current_buffer = b;

	editor_loop(e);

	if (b->cancel) {
		ret = NULL;
	} else {
		ret = gbf_text(e->current_buffer->gbuf);
	}
	buffer_free(&b);
	e->current_buffer = tmp;
	e->current_buffer->redraw = true;

	return ret;
}

static void
yes_no_draw_func(Editor *e) {
	Buffer *b = e->current_buffer;
	char *text = gbf_text(b->gbuf);

	display_clear_window(*b->messagebar_win);

	size_t col = display_show_string(*b->messagebar_win, 0, 0, b->prompt);
	display_show_string(*b->messagebar_win, 0, col, text);
	display_move_cursor(*b->messagebar_win, b->cursor.line, b->cursor.column + col);

	display_refresh();
	free(text);
}

static Buffer *
make_yes_no_buffer(Editor *e) {
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
	buf->draw = yes_no_draw_func;
	buf->draw_statusbar = editor_draw_statusbar;

	buf->position.line = 1;
	buf->position.column = 1;

	buf->win = &e->window;
	buf->statusbar_win = &e->statusbar_win;
	buf->messagebar_win = &e->messagebar_win;

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
	buf->funcs[KEY_CTRL_M] = &uf_ok;
	buf->funcs[KEY_CTRL_Z] = &uf_suspend;

	buf->funcs[KEY_RIGHT] = &uf_right;
	buf->funcs[KEY_LEFT] = &uf_left;

	buf->funcs[KEY_HOME] = &uf_bol;
	buf->funcs[KEY_END] = &uf_eol;

	buf->funcs[KEY_BACKSPACE] = &uf_backspace;
	buf->funcs[KEY_DELETE] = &uf_delete;
	buf->funcs[KEY_RESIZE] = &uf_resize;

	return buf;
}

MenuResult
menu_yes_no(Editor *e, char *prompt) {
	Buffer *b = make_yes_no_buffer(e);
	Buffer *tmp = e->current_buffer;
	int stop = false;
	MenuResult ret = false;

	e->current_buffer = b;
	b->prompt = prompt;

	while (stop == false) {
		editor_loop(e);
		if (b->cancel) {
			stop = true;
			ret = MENU_CANCEL;
		} else {
			char *text = gbf_text(e->current_buffer->gbuf);
			if (strcmp(text, "yes") == 0) {
				stop = true;
				ret = MENU_YES;
			} else if (strcmp(text, "no") == 0) {
				stop = true;
				ret = MENU_NO;
			}
			free(text);
		}
		b->ok = false;
	}

	e->current_buffer = tmp;
	e->current_buffer->redraw = true;
	buffer_free(&b);

	return ret;
}
