#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "display.h"
#include "utf8.h"

struct termios old_config;
struct termios config;
int terminal;


void
display_init(void) {
	terminal = open("/dev/tty", O_RDWR);
	// TODO: error handling
	if (!isatty(terminal)) {
		fprintf(stderr, "Can't open /dev/tty\n");
		exit(-1);
	}
	// Get the current terminal configuration.
	tcgetattr(terminal, &old_config);
	// Save the configuration, so we can restore it on exit.
	config = old_config;

	// In canonical mode, input is read line by line. Since we want
	// to receive input as soon as it's available, we need to turn
	// it off.
	config.c_lflag &= ~ICANON;

	// Disable echo.
	config.c_lflag &= ~ECHO;

	// Disable flow control, so we receive ^Q and ^S.
	config.c_iflag &= ~IXON;

	// Stop the terminal from eating ^C (SIGINTR) ^\ (SIGQUIT) and ^Z (SIGSUSP).
	config.c_lflag &= ~ISIG;

	// Don't do implementation defined input processing. Without this,
	// the terminal  will eat ^V.
	config.c_lflag &= ~IEXTEN;

	// If ICRNL is set, the terminal returns the same code for ^M and ^J. We need to
	// turn it off, so we can bind the keys to different functions.
	config.c_iflag &= ~ICRNL;

	// Don't strip the 8th bit.
	config.c_iflag &= ~ISTRIP;

	// Don't do output processing.
	config.c_oflag &= ~OPOST;

	// Set the new attributes (TCSANOW: apply changes immediately).
	tcsetattr(terminal, TCSANOW, &config);
	display_to_alt_screen();
}

void
display_close(void) {
	// Restore the termminal to it's previous state. TCSAFLUSH causes
	// leftover input to be discarded.
	tcsetattr(terminal, TCSAFLUSH, &old_config);
	display_from_alt_screen();
}

void
display_to_alt_screen(void) {
	printf("\x1B[?1049h");
}

void
display_from_alt_screen(void) {
	printf("\x1B[?1049l");
}

void
display_clear(void) {
	printf("\x1B[2J");
}

void
display_clear_line(Window w, size_t line) {
	display_move_cursor(w, line, 0);
	printf("\x1B[2K");
}

void
display_clear_window(Window w) {
	for (size_t l = 0; l < w.size.lines; l++) {
		display_clear_line(w, l);
	}
}

void
display_show_cp(Window w, size_t line, size_t column, char *cp) {
	size_t l = w.position.line + line;
	size_t c = w.position.column + column;

	printf("\x1B[%zu;%zuH%s", l, c, cp);
}

size_t
display_show_string(Window w, size_t line, size_t column, char *text) {
	char cp[5] = {0};
	size_t col = 0;

	for (size_t i = 0;
		 (column < w.size.columns) && (text[i] != '\0');
		 column += utf8_draw_width(text[i]), i++) {
		for (size_t j = 0; j < utf8_byte_size(text[i]); j++) {
			cp[j] = text[i + j];
		}
		display_show_cp(w, line, column + col, cp);
	}
	return column;
}

void
display_move_cursor(Window w, size_t line, size_t column) {
	size_t l = w.position.line + line;
	size_t c = w.position.column + column;
	printf("\x1B[%zu;%zuH", l, c);
}

void
display_move_window(Window *w, size_t line, size_t column) {
	w->position.line = line;
	w->position.column = column;
}

void
display_resize_window(Window *w, size_t lines, size_t columns) {
	w->size.lines = lines;
	w->size.columns = columns;
}

void
display_set_size(Display *d) {
	struct winsize ws;

	ioctl(1, TIOCGWINSZ, &ws);
	d->lines = ws.ws_row;
	d->columns = ws.ws_col;
}

void
display_refresh(void) {
	fflush(stdout);
}

void
display_set_color(Color c) {
	printf("\x1B[%dm", c);
}
