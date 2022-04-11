#ifndef DRTE_DISPLAY_H
#define DRTE_DISPLAY_H

/// \file
/// display.h contains various display function,
/// such as init, close, drawing, colors, ...

/// Display defines the size of the display in lines and columns.
typedef struct {
	size_t lines; ///< The number of lines
	size_t columns; ///< The number of columns.
} Display;

/// Window defines a region of the display.
typedef struct {
	///< The upper left corner.
	struct {
		size_t line;
		size_t column;
	} position;
	///< The size in lines and columns.
	struct {
		size_t lines;
		size_t columns;
	} size;

} Window;

/// Cursor specifies the cursor position.
typedef	struct {
		size_t line;
		size_t column;
} Cursor;

/// Color defines various color constants.
typedef enum {
	OFF = 0,

	INVERSE = 7,

	DEFAULT_FOREGROUND = 39,

	FOREGROUND_BLACK = 30,
	FOREGROUND_RED = 31,
	FOREGROUND_GREEN = 32,
	FOREGROUND_YELLOW = 33,
	FOREGROUND_BLUE = 34,
	FOREGROUND_MAGENTA = 35,
	FOREGROUND_CYAN = 36,
	FOREGROUND_WHITE = 37,
	FOREGROUND_BRIGHT_BLACK = 90,
	FOREGROUND_BRIGHT_RED = 91,
	FOREGROUND_BRIGHT_GREEN = 92,
	FOREGROUND_BRIGHT_YELLOW = 93,
	FOREGROUND_BRIGHT_BLUE = 94,
	FOREGROUND_BRIGHT_MAGENTA = 95,
	FOREGROUND_BRIGHT_CYAN = 96,
	FOREGROUND_BRIGHT_WHITE = 97,

	DEFAULT_BACKGROUND = 49,

	BACKGROUND_BLACK = 40,
	BACKGROUND_RED = 41,
	BACKGROUND_GREEN = 42,
	BACKGROUND_YELLOW = 43,
	BACKGROUND_BLUE = 44,
	BACKGROUND_MAGENTA = 45,
	BACKGROUND_CYAN = 46,
	BACKGROUND_WHITE = 47,
	BACKGROUND_BRIGHT_BLACK = 100,
	BACKGROUND_BRIGHT_RED = 101,
	BACKGROUND_BRIGHT_GREEN = 102,
	BACKGROUND_BRIGHT_YELLOW = 103,
	BACKGROUND_BRIGHT_BLUE = 104,
	BACKGROUND_BRIGHT_MAGENTA = 105,
	BACKGROUND_BRIGHT_CYAN = 106,
	BACKGROUND_BRIGHT_WHITE = 107,

} Color;


/// display_init initializes the display.
void display_init(void);

/// display_close closes the display and restores the terminal configuration.
/// Forgetting to call this function, will lead to weird terminal behaviour after
/// the editor closes.
void display_close(void);

/// display_to_alt_screen commands the terminal to the alternate buffer.
void display_to_alt_screen(void);

/// display_from_alt_screen commands the terminal to not use the alternate buffer.
void display_from_alt_screen(void);

/// display_set_timeout sets the timeout for read.
/// \param timout The timeout in deciseconds.
void display_set_timeout(size_t timeout);

/// display_clear_timeout removes the timeout for read.
void display_clear_timeout(void);

/// display_show_cursor will cause the display to draw the cursor.
void display_show_cursor(void);

/// display_hide_cursor will prevent the display from drawing the cursor.
void display_hide_cursor(void);

/// display_clear clears the display.
void display_clear(void);

/// display_clear_line clears one line of a window.
/// \param w A window.
/// \param line The line to clear.
void display_clear_line(Window w, size_t line);

/// display_clear_window clears a window.
/// \param w A window.
void display_clear_window(Window w);

/// display_show_cp displays a code point at the given position.
/// \param w The window to draw into.
/// \param line The line.
/// \param column The column.
/// \param cp A code point.
void display_show_cp(Window w, size_t line, size_t column, char *cp);

/// display_show_string draws a string into a window at a given position.
/// This function is intended for drawing single lines. So the input should not
/// contain any newlines.
/// \param w The window to draw into.
/// \param line The line.
/// \param column The column.
/// \param text The text to draw at (line, column).
/// \return The number of columns drawn.
size_t display_show_string(Window w, size_t line, size_t column, char *text);

/// display_move_cursor moves the cursor to the given position.
/// \param w The window to draw into.
/// \param line The line.
/// \param column The column.
void display_move_cursor(Window w, size_t line, size_t column);

/// display_move_window moves a window.
/// \param w The window to move.
/// \param line The line to move the window to.
/// \param column The column to move the window to.
void display_move_window(Window *w, size_t line, size_t column);

/// display_resize_window resizes a window.
/// \param w The window to resize.
/// \param lines The new number of lines.
/// \param columns The new number of columns.
void display_resize_window(Window *w, size_t lines, size_t columns);

/// display_set_size computes and sets the display size.
/// \param d The display.
void display_set_size(Display *d);

/// display_refresh updates the display. The other drawing functions change
/// the internal state. This function actually makes the changes visible.
void display_refresh(void);


/// display_set_color sets the current foreground/background color.
/// \param c The color to activate.
void display_set_color(Color c);


#endif
