#ifndef DRTE_FUNCS_H
#define DRTE_FUNCS_H


struct Editor;

typedef enum {
	USER_FUNC_MOVEMENT,
	USER_FUNC_INSERTION,
	USER_FUNC_DELETION,
	USER_FUNC_MANAGEMENT
} UserFuncType;

typedef struct {
	UserFuncType type;
	char *name;
	char *description;
	void (*func)(struct Editor *);
} UserFunc;


void insert(struct Editor *e);
void newline(struct Editor *e);
void tab(struct Editor *e);
void delete(struct Editor *e);
void backspace(struct Editor *e);
void menu_up(struct Editor *e);
void menu_down(struct Editor *e);
void menu_tab(struct Editor *e);
void toggle_show_hidden_files(struct Editor *e);
void left(struct Editor *e);
void right(struct Editor *e);
void up(struct Editor *e);
void down(struct Editor *e);
void eol(struct Editor *e);
void bol(struct Editor *e);
void page_up(struct Editor *e);
void page_down(struct Editor *e);
void isearch(struct Editor *e);
void isearch_next(struct Editor *e);
void isearch_previous(struct Editor *e);
void region_start_stop(struct Editor *e);
void region_off(struct Editor *e);
void copy(struct Editor *e);
void cut(struct Editor *e);
void paste(struct Editor *e);
void macro_start_stop(struct Editor *e);
void macro_append(struct Editor *e);
void macro_play(struct Editor *e);
void next_buffer(struct Editor *e);
void previous_buffer(struct Editor *e);
void resize(struct Editor *e);
void suspend(struct Editor *e);
void ok(struct Editor *e);
void cancel(struct Editor *e);
void timeout(struct Editor *e);
void prefix(struct Editor *e);
void openfile(struct Editor *e);
void switch_buffer(struct Editor *e);
void save(struct Editor *e);
void save_as(struct Editor *e);
void close_buffer(struct Editor *e);
void quit(struct Editor *e);


extern UserFunc uf_insert;
extern UserFunc uf_newline;
extern UserFunc uf_tab;
extern UserFunc uf_delete;
extern UserFunc uf_backspace;
extern UserFunc uf_left;
extern UserFunc uf_right;
extern UserFunc uf_menu_up;
extern UserFunc uf_menu_down;
extern UserFunc uf_menu_tab;
extern UserFunc uf_toggle_show_hidden_files;
extern UserFunc uf_up;
extern UserFunc uf_down;
extern UserFunc uf_bol;
extern UserFunc uf_eol;
extern UserFunc uf_page_up;
extern UserFunc uf_page_down;
extern UserFunc uf_isearch;
extern UserFunc uf_isearch_next;
extern UserFunc uf_isearch_previous;
extern UserFunc uf_region_start_stop;
extern UserFunc uf_region_off;
extern UserFunc uf_copy;
extern UserFunc uf_cut;
extern UserFunc uf_paste;
extern UserFunc uf_macro_start_stop;
extern UserFunc uf_macro_play;
extern UserFunc uf_next_buffer;
extern UserFunc uf_previous_buffer;
extern UserFunc uf_resize;
extern UserFunc uf_suspend;
extern UserFunc uf_ok;
extern UserFunc uf_cancel;
extern UserFunc uf_timeout;
extern UserFunc uf_prefix;
extern UserFunc uf_openfile;
extern UserFunc uf_switch_buffer;
extern UserFunc uf_save;
extern UserFunc uf_save_as;
extern UserFunc uf_close_buffer;
extern UserFunc uf_quit;


#endif
