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

typedef struct Buffer {
	char *filename; ///< The filename or NULL, if unnamed
	bool has_changed; ///< True, if the text has changed.
	bool redraw; ///< True, if the display has changed.

	bool ok; ///< This is used by menus.
	bool cancel; ///< This is used by menus.

	Func prev_func; ///< The previously called function.
	char *prompt; ///< The prompt shown in menus.
	size_t target_column; ///< Used by up/down to find the correnct column.

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

	UserFunc funcs[KEY_N_SPECIAL_KEYS]; ///< The keybindings.
	GapBuffer *gbuf; ///< The GapBuffer.

	DisplayFunc draw; ///< This function draws the buffer.
	DisplayFunc draw_statusbar; ///< This function draws the statusbar.
	size_t first_visible_char; ///< The start of the first visible line.

	struct Buffer *next; ///< The next buffer.
	struct Buffer *prev; ///< The previous buffer.
} Buffer;

Buffer *buffer_new(struct Editor *e, char *filename);
void buffer_free(Buffer **buf);
void buffer_append(Buffer **current, Buffer *new);


#endif
