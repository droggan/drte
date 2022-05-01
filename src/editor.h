#ifndef DRTE_EDITOR_H
#define DRTE_EDITOR_H


/// \file
/// editor.h defines the editor structure.

typedef struct MacroElement{
	UserFunc *uf;
	ChunkListItem *text;
	struct MacroElement *next;
} MacroElement;


/// The editor structure.
typedef struct Editor {
	Display display; ///< The display.

	Window window;   ///< The main window.
	Window statusbar_win; ///< The window for the statusbar.
	Window messagebar_win; ///< The window for the messagebar.

	char *string_arg; ///< This is used to pass strings to functions.
	size_t size_t_arg; ///< This is used to pass size_ts to functions.

	struct {
		bool recording_macro; ///< True, if the editor is recording a macro.
		ChunkList *chunk_list; ///< A ChunkList used by macro functions.
		MacroElement *first; ///< The first element in the macro list.
		MacroElement *last; ///< The last element in the macro list.
	} macro_info;

	char *copy_buffer; ///< A dynamically allocated buffer, containing cut/copied text.
	size_t copy_buffer_size; ///< The size of the copy buffer.
	size_t copy_bytes_written; ///< The number of bytes written to copy_buffer.

	bool shows_message; ///< This is true, if the editor shows a message.

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

/// editor_loop_once is like editor_loop, but it performs only one iteration.
/// \param e A pointer to the editor structure.
void editor_loop_once(Editor *e);

/// editor_loop is the main loop. It reads and processes input,
/// draws the windows, etc.
/// \param e A pointer to the editor structure.
void editor_loop(Editor *e);


#endif
