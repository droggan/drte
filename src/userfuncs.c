#include "funcs.h"
#include "userfuncs.h"

UserFunc uf_insert = {
	.type = USER_FUNC_INSERTION,
	.name = "insert",
	.description = "",
	.func = insert
};

UserFunc uf_newline = {
	.type = USER_FUNC_INSERTION,
	.name = "newline",
	.description = "Inserts a newline at the current position.",
	.func = newline
};

UserFunc uf_tab = {
	.type = USER_FUNC_INSERTION,
	.name = "tab",
	.description = "Inserts a tab at the current position.",
	.func = tab
};

UserFunc uf_delete = {
	.type = USER_FUNC_DELETION,
	.name = "delete",
	.description = "Deletes the character under the cursor.",
	.func = delete
};

UserFunc uf_backspace = {
	.type = USER_FUNC_DELETION,
	.name = "backspace",
	.description = "Deletes the character before the cursor.",
	.func = backspace
};

UserFunc uf_left = {
	.type = USER_FUNC_MOVEMENT,
	.name = "left",
	.description = "Moves the cursor backward.",
	.func = left
};

UserFunc uf_right = {
	.type = USER_FUNC_MOVEMENT,
	.name = "right",
	.description = "Moves the cursor forward.",
	.func = right
};

UserFunc uf_menu_up = {
	.type = USER_FUNC_MOVEMENT,
	.name = "menu_up",
	.description = "Select the previous menu item.",
	.func = menu_up
};

UserFunc uf_menu_down = {
	.type = USER_FUNC_MOVEMENT,
	.name = "menu_down",
	.description = "Select the next menu item.",
	.func = menu_down
};

UserFunc uf_up = {
	.type = USER_FUNC_MOVEMENT,
	.name = "up",
	.description = "Moves the cursor up.",
	.func = up
};

UserFunc uf_down = {
	.type = USER_FUNC_MOVEMENT,
	.name = "down",
	.description = "Moves the cursor down",
	.func = down
};

UserFunc uf_bol = {
	.type = USER_FUNC_MOVEMENT,
	.name = "bol",
	.description = "Moves the cursor to the beginning of the current line",
	.func = bol
};

UserFunc uf_eol = {
	.type = USER_FUNC_MOVEMENT,
	.name = "eol",
	.description = "Moves the cursor to the end of the current line.",
	.func = eol
};

UserFunc uf_page_up = {
	.type = USER_FUNC_MOVEMENT,
	.name = "page_up",
	.description = "Scrolls up by one screen.",
	.func = page_up
};

UserFunc uf_page_down = {
	.type = USER_FUNC_MOVEMENT,
	.name = "page_down",
	.description = "Scrolls down by one screen.",
	.func = page_down
};

UserFunc uf_isearch = {
	.type = USER_FUNC_MOVEMENT,
	.name = "isearch",
	.description = "Start searching at the current cursor position.",
	.func = isearch
};

UserFunc uf_isearch_next = {
	.type = USER_FUNC_MOVEMENT,
	.name = "isearch_next",
	.description = "Go to the next result.",
	.func = isearch_next
};

UserFunc uf_isearch_previous = {
	.type = USER_FUNC_MOVEMENT,
	.name = "isearch_previous",
	.description = "Go to the previous result.",
	.func = isearch_previous
};

UserFunc uf_region_start_stop = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "region_start_stop",
	.description = "Start or stop selecting text.",
	.func = region_start_stop
};

UserFunc uf_region_off = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "region_off",
	.description = "Clear the current region.",
	.func = region_off
};

UserFunc uf_copy = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "copy",
	.description = "Copy the selected text.",
	.func = copy
};

UserFunc uf_cut = {
	.type = USER_FUNC_DELETION,
	.name = "cut",
	.description = "Cut the selected text.",
	.func = cut
};

UserFunc uf_paste = {
	.type = USER_FUNC_INSERTION,
	.name = "paste",
	.description = "Paste previously cut/copied text.",
	.func = paste
};

UserFunc uf_macro_start_stop = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "macro_start_stop",
	.description = "Start or stop recording a macro.",
	.func = macro_start_stop
};

UserFunc uf_macro_play = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "macro_play",
	.description = "Play a previously recorded macro.",
	.func = macro_play
};

UserFunc uf_next_buffer = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "next_buffer",
	.description = "Switch to the next buffer.",
	.func = next_buffer
};

UserFunc uf_previous_buffer = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "previous_buffer",
	.description = "Switch to the previous buffer.",
	.func = previous_buffer
};

UserFunc uf_resize = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "resize",
	.description = "Resize the editor.",
	.func = resize
};

UserFunc uf_suspend = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "suspend",
	.description = "Suspend the editor.",
	.func = suspend
};

UserFunc uf_ok = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "ok",
	.description = "", // TODO
	.func = ok
};

UserFunc uf_cancel = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "cancel",
	.description = "", // TODO
	.func = cancel
};

UserFunc uf_prefix = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "prefix",
	.description = "",
	.func = prefix
};

UserFunc uf_openfile = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "open",
	.description = "Open a file.",
	.func = openfile
};

UserFunc uf_save = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "save",
	.description = "Saves the current buffer.",
	.func = save
};

UserFunc uf_save_as = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "Save As",
	.description = "Save the current buffer under a different name", // TODO: better descrption
	.func = save_as
};

UserFunc uf_close_buffer = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "close_buffer",
	.description = "Close the current buffer.",
	.func = close_buffer
};

UserFunc uf_quit = {
	.type = USER_FUNC_MANAGEMENT,
	.name = "quit",
	.description = "Quit.",
	.func = quit
};
