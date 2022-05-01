#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "gapbuffer.h"
#include "display.h"
#include "input.h"
#include "funcs.h"
#include "chunk_list.h"
#include "menus.h"
#include "buffer.h"
#include "editor.h"
#include "utf8.h"


static MenuItem *item_list_sort(MenuItem *list);
static MenuItemList *new_menu_item_list(void);
static MenuItem *menu_item_list_insert(MenuItemList *list, char *item, bool is_dir);
static void free_menu_item_list(MenuItemList **list);
static void file_chooser_draw_func(Editor *e);
static Buffer *make_file_chooser_buffer(Editor *e);
static void buffer_chooser_draw_func(Editor *e);
static Buffer *make_buffer_chooser_buffer(Editor *e);
static void yes_no_draw_func(Editor *e);
static Buffer *make_yes_no_buffer(Editor *e);


// Sort the item list using merge sort.
static MenuItem *
item_list_sort(MenuItem *list) {
	MenuItem *left;
	MenuItem *right;
	MenuItem *slow = list;
	MenuItem *fast = list;

	if (list == NULL || list->next == NULL) {
		return list;
	}

	// Split the list in two halves.
	while (fast->next != NULL && fast->next->next != NULL) {
		fast = fast->next->next;
		slow = slow->next;
	}
	left = list;
	right = slow->next;
	slow->next = NULL;

	// Sort the two halves.
	left = item_list_sort(left);
	right = item_list_sort(right);

	// Merge the two lists.
	MenuItem *out = NULL;
	MenuItem **new = &out;
	while (left != NULL || right != NULL) {
		if (left == NULL) {
			*new = right;
			break;
		}
		if (right == NULL) {
			*new = left;
			break;
		}

		// Compare the items. We want to sort directories first, so if one item
		// is a directory and the other isn't, it automatically comes first.
		int direction = 0;
		if (left->is_dir && !right->is_dir) {
			direction = -1;
		} else if (!left->is_dir && right->is_dir) {
			direction = 1;
		} else {
			char *left_item = chunk_list_get_item(left->item);
			char *right_item = chunk_list_get_item(right->item);

			direction = strcmp(left_item, right_item);

			free(left_item);
			free(right_item);
		}

		if (direction <= 0) {
			*new = left;
			left = left->next;
			new = &(*new)->next;
		} else  {
			*new = right;
			right = right->next;
			new = &(*new)->next;
		}
	}

	// Fix the prev pointers.
	// TODO: we should be able to fix the prev pointers during merging.
	MenuItem *last = NULL;
	MenuItem *iter = out;
	while (iter != NULL) {
		iter->prev = last;
		last = iter;
		iter = iter->next;
	}

	return out;
}

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
menu_item_list_insert(MenuItemList *list, char *item, bool is_dir) {
	MenuItem *new_item = malloc(sizeof(*new_item));
	if (new_item == NULL) {
		return NULL;
	}

	ChunkListItem *citem = chunk_list_insert(list->chunk_list, item);
	if (citem == NULL) {
		return NULL;
	}

	new_item->item = citem;
	new_item->is_dir = is_dir;
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
	size_t text_len = strlen(text);
	size_t lines = b->win->size.lines;
	size_t line = 0;
	MenuItemList *items = b->menu_items;
	size_t first_visible = 0;
	size_t selected = 0;

	if (b->has_changed) {
		items->selected = NULL;
		items->first_visible_item = NULL;
		b->has_changed = false;
	}
	// Scroll up/down, if necessary.
	if (items->selected != NULL) {
		MenuItem *iter = b->menu_items->first;
		size_t i = 0;

		// Measure the distance between the first visible item and the selected item.
		while(iter != NULL) {
			if (iter == items->first_visible_item) {
				first_visible = i;
			} else if (iter == items->selected) {
				selected = i;
			}
			i++;
			iter = iter->next;
		}
		if (first_visible < selected) {
			size_t dist = selected - first_visible;
			// If the distance is bigger than the window size,
			// the selected item is offscreen and we need to scroll down.
			while (dist >= b->win->size.lines) {
				items->first_visible_item = items->first_visible_item->next;
				dist--;
			}
		} else if (first_visible > selected) {
			// If the selected item is above the screen, we simply
			// set the first visible item  to the selected item.
			items->first_visible_item = items->selected;
		}
	}

	MenuItem *item = items->first;
	while (item != NULL) {
		char *item_text = chunk_list_get_item(item->item);

		if (!b->show_hidden_files && item_text[0] == '.' && strcmp(item_text, "..") != 0) {
			item->is_visible = false;
		} else if (strncmp(text, item_text, text_len) == 0) {
			item->is_visible = true;
		} else {
			item->is_visible = false;
		}
		free(item_text);
		item = item->next;
	}

	MenuItem *current = b->menu_items->first;
	display_clear_window(*b->win);
	while ((current != NULL) && (line < lines)) {
		if (current->is_visible) {
			char *item_text = chunk_list_get_item(current->item);
			if (items->first_visible_item == NULL) {
				items->first_visible_item = current;
			}
			if (current == b->menu_items->selected) {
				display_set_color(BACKGROUND_GREEN);
				display_set_color(FOREGROUND_BLACK);
			}
			size_t drawn = display_show_string(*b->win, line, 0, item_text);
			if (current->is_dir) {
				display_show_string(*b->win, line, drawn, "/");
			}
			line++;
			display_set_color(OFF);
			free(item_text);
		}
		current = current->next;
	}
	display_set_color(BACKGROUND_BLUE);
	size_t col = display_show_string(*b->statusbar_win, 0, 0, "Choose a file");
	while (col < b->statusbar_win->size.columns) {
		display_show_cp(*b->statusbar_win, 0, col, " ");
		col++;
	}
	display_set_color(OFF);

	display_clear_window(*b->messagebar_win);

	col = display_show_string(*b->messagebar_win, 0, 0, "File: ");
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

	buffer_bind_key(buf, KEY_CTRL_A, &uf_bol);
	buffer_bind_key(buf, KEY_CTRL_B, &uf_left);
	buffer_bind_key(buf, KEY_CTRL_C, &uf_cancel);
	buffer_bind_key(buf, KEY_CTRL_D, &uf_delete);
	buffer_bind_key(buf, KEY_CTRL_E, &uf_eol);
	buffer_bind_key(buf, KEY_CTRL_F, &uf_right);
	buffer_bind_key(buf, KEY_CTRL_H, &uf_backspace);
	buffer_bind_key(buf, KEY_CTRL_I, &uf_menu_tab);
	buffer_bind_key(buf, KEY_CTRL_M, &uf_ok);
	buffer_bind_key(buf, KEY_CTRL_N, &uf_menu_down);
	buffer_bind_key(buf, KEY_CTRL_P, &uf_menu_up);
	buffer_bind_key(buf, KEY_CTRL_Z, &uf_suspend);

	buffer_bind_key(buf, KEY_ALT_H, &uf_toggle_show_hidden_files);

	buffer_bind_key(buf, KEY_UP, &uf_menu_up);
	buffer_bind_key(buf, KEY_DOWN, &uf_menu_down);
	buffer_bind_key(buf, KEY_LEFT, &uf_left);
	buffer_bind_key(buf, KEY_RIGHT, &uf_right);

	buffer_bind_key(buf, KEY_HOME, &uf_bol);
	buffer_bind_key(buf, KEY_END, &uf_eol);

	buffer_bind_key(buf, KEY_BACKSPACE, &uf_backspace);
	buffer_bind_key(buf, KEY_DELETE, &uf_delete);
	buffer_bind_key(buf, KEY_RESIZE, &uf_resize);

	return buf;
}

char *
menu_choose_file(Editor *e) {
	Buffer *b = make_file_chooser_buffer(e);
	Buffer *tmp = e->current_buffer;
	char *selected = NULL;
	char *cwd;
	GapBuffer *path = gbf_new();
	struct dirent *entry;
	struct stat st;

	e->current_buffer = b;

	cwd = getcwd(NULL, 0);
	gbf_insert(path, cwd, 0);
	free(cwd);

	do {
		char *p = gbf_text(path);
		DIR *dir = opendir(p);
		free(p);

		MenuItemList *list = new_menu_item_list();
		while ((entry = readdir(dir))) {
			bool is_dir = false;
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				continue;
			}
			// TODO: according to the man page, this does not work on all filesystems.
			// Is there a relevant fs, that doesn't support this?
			if (entry->d_type == DT_DIR) {
				is_dir = true;
			}
			menu_item_list_insert(list, entry->d_name, is_dir);
		}
		closedir(dir);

		list->first = item_list_sort(list->first);
		menu_item_list_insert(list, "..", true);
		list->first_visible_item = list->first;

		b->menu_items = list;

		editor_loop(e);

		if (b->cancel) {
			selected = NULL;
		} else {
			if (list->selected != NULL) {
				selected = chunk_list_get_item(list->selected->item);
			} else {
				selected = gbf_text(e->current_buffer->gbuf);
			}
		}
		free_menu_item_list(&list);

		if (selected != NULL) {
			char *cp;

			gbf_insert(path, "/", gbf_text_length(path));
			gbf_insert(path, selected, gbf_text_length(path));
			free(selected);

			cp = gbf_text(path);

			int res = stat(cp, &st);
			if (res == 0 && S_ISDIR(st.st_mode)) {
				b->ok = false;
			}
			free(cp);
		}
	} while (!b->cancel && !b->ok);

	char *ret = NULL;
	if (!b->cancel) {
		ret = gbf_text(path);
	}
	buffer_free(&b);
	gbf_free(&path);
	e->current_buffer = tmp;
	e->current_buffer->redraw = true;

	return ret;
}

static void
buffer_chooser_draw_func(Editor *e) {
	Buffer *b = e->current_buffer;
	char *text = gbf_text(b->gbuf);
	size_t text_len = strlen(text);
	size_t lines = b->win->size.lines;
	size_t line = 0;
	MenuItemList *items = b->menu_items;
	size_t first_visible = 0;
	size_t selected = 0;

	if (b->has_changed) {
		items->selected = NULL;
		items->first_visible_item = NULL;
		b->has_changed = false;
	}
	// Scroll up/down, if necessary.
	if (items->selected != NULL) {
		MenuItem *iter = b->menu_items->first;
		size_t i = 0;

		// Measure the distance between the first visible item and the selected item.
		while(iter != NULL) {
			if (iter == items->first_visible_item) {
				first_visible = i;
			} else if (iter == items->selected) {
				selected = i;
			}
			i++;
			iter = iter->next;
		}
		if (first_visible < selected) {
			size_t dist = selected - first_visible;
			// If the distance is bigger than the window size,
			// the selected item is offscreen and we need to scroll down.
			while (dist >= b->win->size.lines) {
				items->first_visible_item = items->first_visible_item->next;
				dist--;
			}
		} else if (first_visible > selected) {
			// If the selected item is above the screen, we simply
			// set the first visible item  to the selected item.
			items->first_visible_item = items->selected;
		}
	}

	MenuItem *item = items->first;
	while (item != NULL) {
		char *item_text = chunk_list_get_item(item->item);
		if (strncmp(text, item_text, text_len) == 0) {
			item->is_visible = true;
		} else {
			item->is_visible = false;
		}
		free(item_text);
		item = item->next;
	}

	MenuItem *current = b->menu_items->first;
	display_clear_window(*b->win);
	while ((current != NULL) && (line < lines)) {
		if (current->is_visible) {
			char *item_text = chunk_list_get_item(current->item);
			if (items->first_visible_item == NULL) {
				items->first_visible_item = current;
			}
			if (current == b->menu_items->selected) {
				display_set_color(BACKGROUND_GREEN);
				display_set_color(FOREGROUND_BLACK);
			}
			display_show_string(*b->win, line, 0, item_text);
			line++;
			display_set_color(OFF);
			free(item_text);
		}
		current = current->next;
	}
	display_set_color(BACKGROUND_BLUE);
	size_t col = display_show_string(*b->statusbar_win, 0, 0, "Choose a buffer");
	while (col < b->statusbar_win->size.columns) {
		display_show_cp(*b->statusbar_win, 0, col, " ");
		col++;
	}
	display_set_color(OFF);

	display_clear_window(*b->messagebar_win);

	col = display_show_string(*b->messagebar_win, 0, 0, "Buffer: ");
	display_show_string(*b->messagebar_win, 0, col, text);
	display_move_cursor(*b->messagebar_win, b->cursor.line, b->cursor.column + col);

	free(text);
	display_refresh();
}

static Buffer *
make_buffer_chooser_buffer(Editor *e) {
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
	buf->draw = buffer_chooser_draw_func;
	buf->draw_statusbar = editor_draw_statusbar;

	buf->position.line = 1;
	buf->position.column = 1;

	buf->win = &e->window;
	buf->statusbar_win = &e->statusbar_win;
	buf->messagebar_win = &e->messagebar_win;

	buf->next = buf;
	buf->prev = buf;

	buffer_bind_key(buf, KEY_CTRL_A, &uf_bol);
	buffer_bind_key(buf, KEY_CTRL_B, &uf_left);
	buffer_bind_key(buf, KEY_CTRL_C, &uf_cancel);
	buffer_bind_key(buf, KEY_CTRL_D, &uf_delete);
	buffer_bind_key(buf, KEY_CTRL_E, &uf_eol);
	buffer_bind_key(buf, KEY_CTRL_F, &uf_right);
	buffer_bind_key(buf, KEY_CTRL_H, &uf_backspace);
	buffer_bind_key(buf, KEY_CTRL_I, &uf_menu_tab);
	buffer_bind_key(buf, KEY_CTRL_M, &uf_ok);
	buffer_bind_key(buf, KEY_CTRL_N, &uf_menu_down);
	buffer_bind_key(buf, KEY_CTRL_P, &uf_menu_up);
	buffer_bind_key(buf, KEY_CTRL_Z, &uf_suspend);

	buffer_bind_key(buf, KEY_UP, &uf_menu_up);
	buffer_bind_key(buf, KEY_DOWN, &uf_menu_down);
	buffer_bind_key(buf, KEY_LEFT, &uf_left);
	buffer_bind_key(buf, KEY_RIGHT, &uf_right);

	buffer_bind_key(buf, KEY_HOME, &uf_bol);
	buffer_bind_key(buf, KEY_END,  &uf_eol);

	buffer_bind_key(buf, KEY_BACKSPACE, &uf_backspace);
	buffer_bind_key(buf, KEY_DELETE, &uf_delete);
	buffer_bind_key(buf, KEY_RESIZE, &uf_resize);

	return buf;
}

char *
menu_choose_buffer(Editor *e) {
	Buffer *b = make_buffer_chooser_buffer(e);
	Buffer *tmp = e->current_buffer;
	Buffer *iter = e->current_buffer;
	char *selected = NULL;

	e->current_buffer = b;

	MenuItemList *list = new_menu_item_list();
	do {
		iter = iter->next;
		if (iter->filename != NULL) {
			menu_item_list_insert(list, iter->filename, false);
		}
	} while (iter != tmp);
	list->first = item_list_sort(list->first);
	list->first_visible_item = list->first;

	b->menu_items = list;

	editor_loop(e);

	if (b->cancel) {
		selected = NULL;
	} else {
		if (list->selected != NULL) {
			selected = chunk_list_get_item(list->selected->item);
		} else {
			selected = gbf_text(e->current_buffer->gbuf);
		}
	}
	free_menu_item_list(&list);
	buffer_free(&b);
	e->current_buffer = tmp;
	e->current_buffer->redraw = true;

	return selected;
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

	buffer_bind_key(buf, KEY_CTRL_A, &uf_bol);
	buffer_bind_key(buf, KEY_CTRL_B, &uf_left);
	buffer_bind_key(buf, KEY_CTRL_C, &uf_cancel);
	buffer_bind_key(buf, KEY_CTRL_D, &uf_delete);
	buffer_bind_key(buf, KEY_CTRL_E, &uf_eol);
	buffer_bind_key(buf, KEY_CTRL_F, &uf_right);
	buffer_bind_key(buf, KEY_CTRL_H, &uf_backspace);
	buffer_bind_key(buf, KEY_CTRL_I, &uf_tab);
	buffer_bind_key(buf, KEY_CTRL_M, &uf_ok);
	buffer_bind_key(buf, KEY_CTRL_Z, &uf_suspend);

	buffer_bind_key(buf, KEY_RIGHT, &uf_right);
	buffer_bind_key(buf, KEY_LEFT, &uf_left);

	buffer_bind_key(buf, KEY_HOME, &uf_bol);
	buffer_bind_key(buf, KEY_END, &uf_eol);

	buffer_bind_key(buf, KEY_BACKSPACE, &uf_backspace);
	buffer_bind_key(buf, KEY_DELETE, &uf_delete);
	buffer_bind_key(buf, KEY_RESIZE, &uf_resize);

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
