#ifndef DRTE_EDITOR_H
#define DRTE_EDITOR_H


/// \file
/// editor.h defines the editor structure.


/// The editor structure.
typedef struct Editor {
	Display display; ///< The display.

	Window window;   ///< The main window.
	Window statusbar_win; ///< The window for the statusbar.
	Window messagebar_win; ///< The window for the messagebar.

	bool shows_message; ///< This is true, if the editor shows a message.
	bool quit; ///< True, if the user wants the editor to quit.
	Buffer *current_buffer; ///< The current buffer. This is a circular doubly-linked list.
} Editor;

/// editor_show_message draws a message in the messagebar.
/// \param e A pointer to the editor structure.
/// \param message The message to show.
void editor_show_message(Editor *e, char *message);

/// editor_draw_statusbar draws the statusbar. This is used by other
/// drawing functions.
/// \param e A pointer to the editor structure.
void editor_draw_statusbar(Editor *e);

/// editor_loop is the main loop. It reads and processes input,
/// draws the windows, etc.
/// \param e A pointer to the editor structure.
void editor_loop(Editor *e);


#endif
