#ifndef DRTE_FUNCS_H
#define DRTE_FUNCS_H

struct Editor;


void insert(struct Editor *e);
void newline(struct Editor *e);
void tab(struct Editor *e);
void delete(struct Editor *e);
void backspace(struct Editor *);
void left(struct Editor *e);
void right(struct Editor *e);
void up(struct Editor *e);
void down(struct Editor *e);
void eol(struct Editor *e);
void bol(struct Editor *e);
void page_up(struct Editor *e);
void page_down(struct Editor *e);
void macro_start_stop(struct Editor *e);
void macro_append(struct Editor *e);
void macro_play(struct Editor *e);
void next_buffer(struct Editor *e);
void previous_buffer(struct Editor *e);
void resize(struct Editor *e);
void suspend(struct Editor *e);
void ok(struct Editor *e);
void cancel(struct Editor *e);
void openfile(struct Editor *e);
void save(struct Editor *e);
void save_as(struct Editor *e);
void close_buffer(struct Editor *e);
void quit(struct Editor *e);


#endif
