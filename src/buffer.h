#ifndef DRTE_BUFFER_H
#define DRTE_BUFFER_H

/// \file
/// buffer.h implements a textbuffer and various functions that manipulate it.

struct Editor;
struct Buffer;

typedef void (*DisplayFunc)(struct Editor *);

/// The region type
typedef enum {
	REGION_OFF, ///< There is no region.
	REGION_FLUID, ///< The region extends to the current cursor position.
	REGION_ON ///< The region has a fixed start and end position.
} RegionType;

/// The region diretion.
typedef enum {
	REGION_DIRECTION_NONE, ///< There is no region.
	REGION_DIRECTION_LEFT, ///< The region extends to the left of the starting position.
	REGION_DIRECTION_RIGHT ///< The region extends to the right of the starting position.
} RegionDirection;

typedef enum {
	ISEARCH_DIRECTION_NONE, ///< Isearch is off.
	ISEARCH_DIRECTION_FORWARD, ///< Isearch searches forward.
	ISEARCH_DIRECTION_BACKWARD ///< Isearch searches backward.
} ISearchDirection;

typedef struct Buffer {
	char *filename; ///< The filename or NULL, if unnamed
	bool has_changed; ///< True, if the text has changed.
	bool redraw; ///< True, if the display has changed.

	bool ok; ///< This is used by menus.
	bool cancel; ///< This is used by menus.

	UserFunc *prev_func; ///< The previously called function.
	char *prompt; ///< The prompt shown in menus.
	size_t target_column; ///< Used by up/down to find the correct column.

	///< This struct saves the current position.
	struct {
		size_t offset; ///< The current position as offset in bytes.
		size_t line; ///< The current line.
		size_t column; ///< The current column.
	} position;

	Window *win; ///< The window displaying the buffer.
	Window *statusbar_win; ///< The window displaying the statusbar.
	Window *messagebar_win; ///< The window displaying the messagebar.
	Cursor cursor; ///< The cursor position.

	RegionType region_type; ///< The region type (see above).
	RegionDirection region_direction; ///< The region direction (see above).
	size_t region_start; ///< The start of the region as byte offset.
	size_t region_end; ///< The end of the region as byte offset.

	ISearchDirection isearch_direction; ///< The direction isearch searches in.
	bool isearch_is_active; ///< True, if isearch is active.
	bool isearch_has_match; ///< True, if isearch has found a match.
	bool isearch_has_wrapped; ///< True, if isearch has wrapped in any direction.
	size_t isearch_start; ///< Where isearch starts searching.
	size_t isearch_match_start; ///< The beginning of the match.
	size_t isearch_match_end; ///< The end of the match.
	struct Buffer *isearch_buffer; ///< The Buffer userd by isearch.

	UserFunc *funcs[KEY_N_SPECIAL_KEYS]; ///< The keybindings.
	GapBuffer *gbuf; ///< The GapBuffer.

	DisplayFunc draw; ///< This function draws the buffer.
	DisplayFunc draw_statusbar; ///< This function draws the statusbar.
	size_t first_visible_char; ///< The start of the first visible line.

	MenuItemList *menu_items;  ///< Used by various menus to store the menu items.
	bool show_hidden_files; ///< True, if the file chooser shows hidden files.

	bool timeout; ///< True, if something timed out.

	struct Buffer *next; ///< The next buffer.
	struct Buffer *prev; ///< The previous buffer.
} Buffer;

/// buffer_new creates a new buffer.
/// \param e The editor structure.
/// \param filename The file to read into the buffer or NULL for an unnamed buffer.
/// \return A new Buffer or NULL on error.
Buffer *buffer_new(struct Editor *e, char *filename);

/// buffer_bind_key binds c to the function f, in buf.
/// \param buf The buffer.
/// \param c The key.
/// \param f The function.
void buffer_bind_key(Buffer *buf, KeyCode c, UserFunc *f);

/// buffer_call_userfunc call the function bound to c.
/// \param e The editor structure.
/// \param buf The buffer.
/// \param c The key.
/// \return The called function. This can be NULL.
UserFunc *buffer_call_userfunc(struct Editor *e, Buffer *buf, KeyCode c);

/// buffer_free frees a buffer and sets the given pointer to NULL.
/// \param buf The buffer to free.
void buffer_free(Buffer **buf);

/// buffer_append appends a new buffer after the current one.
/// \param current The current buffer.
/// \param new The buffer to append.
void buffer_append(Buffer **current, Buffer *new);


#endif
