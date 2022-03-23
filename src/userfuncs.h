#ifndef DRTE_USERFUNCS_H
#define DRTE_USERFUNCS_H


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

extern UserFunc uf_insert;
extern UserFunc uf_newline;
extern UserFunc uf_tab;
extern UserFunc uf_delete;
extern UserFunc uf_backspace;
extern UserFunc uf_left;
extern UserFunc uf_right;
extern UserFunc uf_menu_up;
extern UserFunc uf_menu_down;
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
extern UserFunc uf_prefix;
extern UserFunc uf_openfile;
extern UserFunc uf_save;
extern UserFunc uf_save_as;
extern UserFunc uf_close_buffer;
extern UserFunc uf_quit;


#endif
