#ifndef DRTE_UTF8_H
#define DRTE_UTF8_H

/// \file
/// utf8.h implemnts various utf8 related functions.

/// utf8_is_valid returs true if the first code point in a
/// string is valid utf8.
/// \param text A pointer to the first byte of a code point.
/// \return true, if the code point is valid. false, otherwise.
bool utf8_is_valid(char *text);

/// utf8_is_valid_first_byte checks if the given byte may occur at the
/// start of a valid code point.
/// \param c The byte to check.
/// \return true, if c may start a valid code point. false, otherwise.
bool utf8_is_valid_first_byte(char c);

/// utf8_draw_width calculates the number of column used by code point.
/// TODO
size_t utf8_draw_width(char text);

/// utf8_byte_size calculates the number of byes a code point consists of.
/// TODO
size_t utf8_byte_size(char text);

/// utf8_is_whitespace checks if a given code point is whitespace.
/// TODO
bool utf8_is_whitespace(char c);

#endif
