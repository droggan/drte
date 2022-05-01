#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>

#include <sys/ioctl.h>

#include "display.h"
#include "input.h"
#include "gapbuffer.h"
#include "funcs.h"
#include "chunk_list.h"
#include "menus.h"
#include "buffer.h"
#include "editor.h"
#include "utf8.h"


static void sigcont_handler(int unused);
static void sigwinch_handler(int unused);

static Editor *e;


static void
sigcont_handler(int unused) {
	(void)unused;
	signal(SIGCONT, sigcont_handler);
	display_init();
	e->current_buffer->redraw = 1;
}

static void
sigwinch_handler(int unused) {
	(void)unused;
	signal(SIGWINCH, sigwinch_handler);
}

int
main(int argc, char **argv) {

	e = malloc(sizeof(*e));
	if (e == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(-1);
	}
	memset(e, 0, sizeof(*e));

	e->macro_info.chunk_list = chunk_list_new(4096);
	if (e->macro_info.chunk_list == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(-1);
	}
	e->current_buffer = NULL;
	signal(SIGCONT, sigcont_handler);
	signal(SIGWINCH, sigwinch_handler);

	display_init();
	atexit(display_close);

	resize(e);

	for (int i = 1; i < argc; i++) {
		char *s = strdup(argv[i]);
		if (s == NULL) {
			fprintf(stderr, "Out of memory\n");
			exit(-1);
		}
		Buffer *b = buffer_new(e, s);
		if (b != NULL) {
			buffer_append(&(e->current_buffer), b);
		}
	}
	if (e->current_buffer == NULL) {
		buffer_append(&(e->current_buffer), buffer_new(e, NULL));
	}
	if (e->current_buffer == NULL) {
		fprintf(stderr, "Ouf of memory\n");
	}

	editor_loop(e);

	while (e->current_buffer != NULL) {
		close_buffer(e);
	}

	return 0;
}
