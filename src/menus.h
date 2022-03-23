#ifndef DRTE_MENUS_H
#define DRTE_MENUS_H

/// \file
/// menus.h implements menus.


/// MenuResult is the return type of menu_yes_no.
typedef enum {
	MENU_YES = 0, ///< The user entered "yes".
	MENU_NO = 1, ///< The user entered "no".
	MENU_CANCEL = 2 ///< The user canceled the menu.
} MenuResult;

/// MenuItem is an entry in a MenuItemList.
typedef struct MenuItem {
	ChunkListItem *item; ///< The actual item.
	bool is_dir; ///< True, if item is a directory.
	bool is_visible; ///< True, if the item is visible.
	struct MenuItem *next; ///< The next item in the menu list.
	struct MenuItem *prev; ///< The previous item in the menu list.
} MenuItem;

/// MenuItemList is a list of menu items.
typedef struct MenuItemList {
	ChunkList *chunk_list; // TODO: remove
	MenuItem *first; ///< The first item in the menu list.
	MenuItem *selected; ///< The currently selected item, if any.
	MenuItem *first_visible_item; ///< The first visible item.
} MenuItemList;


/// menu_choose_file shows a file chooser.
/// \param e The editor structure.
/// \return The selected file, or NULL when the user cancelled the menu.
char *menu_choose_file(struct Editor *e);

/// menu_yes_no shows a menu, that will only allow yes or no as answer.
/// \param e The editor structure.
/// \param prompt The prompt to show to the user.
/// \return A MenuResult. See \ref MenuResult.
MenuResult menu_yes_no(struct Editor *e, char *prompt);


#endif
