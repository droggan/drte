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

/// menu_choose_file shows a file chooser.
/// \param e The editor structure.
/// \return The selected file, or NULL when the user cancelled the menu.
char *menu_choose_file(Editor *e);

/// menu_yes_no shows a menu, that will only allow yes or no as answer.
/// \param e The editor structure.
/// \param prompt The prompt to show to the user.
/// \return A MenuResult. See \ref MenuResult.
MenuResult menu_yes_no(Editor *e, char *prompt);


#endif
