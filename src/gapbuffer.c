#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "gapbuffer.h"
#include "static.h"


struct GapBuffer {
	char *first; // A pointer to the first byte in the gap buffer.
	char *gap; // A pointer to the first byte in the gap.
	char *second; // A pointer to the first byte after the gap.
	char *end; // A pointer to the last byte in the gap buffer. This is always '\0'.
};

STATIC size_t gap_length(GapBuffer *gbuf);
STATIC size_t first_part_length(GapBuffer *gbuf);
STATIC size_t second_part_length(GapBuffer *gbuf);
STATIC size_t max_offset(GapBuffer *gbuf);
STATIC void move_gap(GapBuffer *gbuf, size_t offset);
STATIC void expand_gap(GapBuffer *gbuf);

STATIC size_t INITIAL_SIZE = 8;
STATIC size_t GAP_INCREMENT = 6;
STATIC size_t MIN_GAP_SIZE = 5;

// Returns the length of the gap.
STATIC size_t
gap_length(GapBuffer *gbuf) {
	return gbuf->second - gbuf->gap;
}

// Returns the length of the first part of the text.
// The first part is the text form the start to the gap.
STATIC size_t
first_part_length(GapBuffer  *gbuf) {
	return gbuf->gap - gbuf->first;
}

// Returns the length of the second part of the text.
// The second part is the text from the end of the gap to the end of the buffer.
STATIC size_t
second_part_length(GapBuffer *gbuf) {
	return gbuf->end - gbuf->second;
}

// Returns the maximum valid offset in gbuf.
STATIC size_t
max_offset(GapBuffer *gbuf) {
	return second_part_length(gbuf) + first_part_length(gbuf);
}

GapBuffer *
gbf_new(void) {
	GapBuffer *gbuf;

	gbuf = malloc(sizeof(*gbuf));
	if (gbuf == NULL) {
		return NULL;
	}
	gbuf->first = malloc(INITIAL_SIZE + 1);
	if (gbuf->first == NULL) {
		return NULL;
	}
	gbuf->gap = gbuf->first;
	gbuf->second = gbuf->first + INITIAL_SIZE;
	gbuf->end = gbuf->first + INITIAL_SIZE;
	*(gbuf->end) = '\0';

	return gbuf;
}

void
gbf_free(GapBuffer **gbuf) {
	free((*gbuf)->first);
	free(*gbuf);
	*gbuf = NULL;
}

char *
gbf_text(GapBuffer *gbuf) {
	size_t flen = first_part_length(gbuf);
	size_t slen = second_part_length(gbuf);
	char *str = malloc(flen + slen + 1);

	if (str == NULL) {
		return NULL;
	}
	if (flen == 0) {
		str[0] = '\0';
	} else {
		strncpy(str, gbuf->first, flen);
		str[flen] = '\0';
	}
	if (slen > 0) {
		strcat(str, gbuf->second);
	}
	return str;
}

// Moves gap to position off.
// \param gbuf A GapBuffer.
// \param offset The position to move the gap to.
STATIC void
move_gap(GapBuffer *gbuf, size_t offset) {
	size_t len;
	char *pos = gbuf->first + offset;

	if (pos == gbuf->gap) {
		return;
	}
	if (offset > max_offset(gbuf)) {
		offset = max_offset(gbuf);
	}
	if (pos > gbuf->gap) {
		// cursor after gap
		pos += gap_length(gbuf);
		len = (pos - gbuf->second);
		memmove(gbuf->gap, gbuf->second, len);
		gbuf->gap += len;
		gbuf->second += len;
	} else {
		// cursor before gap
		len = gbuf->gap - pos;
		gbuf->gap -= len;
		gbuf->second -= len;
		memmove(gbuf->second, pos, len);
	}
}

// Expands the gap.
// \param gbuf A Gapbuffer.
STATIC void
expand_gap(GapBuffer *gbuf) {
	size_t flen = first_part_length(gbuf);
	size_t slen = second_part_length(gbuf);
	size_t glen = gap_length(gbuf);
	size_t new_size = (gbuf->end - gbuf->first) + GAP_INCREMENT + 1;
	void *new = realloc(gbuf->first, new_size);

	if (new == NULL) {
		// TOOO: handle error
		exit(-1);
	} else {
		gbuf->first = new;
		gbuf->gap = gbuf->first + flen;
		gbuf->second = gbuf->gap + glen + GAP_INCREMENT;
		gbuf->end = gbuf->second + slen;
		if (slen != 0) {
			memmove(gbuf->second, gbuf->gap + glen, slen);
		}
		*(gbuf->end) = '\0';
	}
}

void
gbf_insert(GapBuffer *gbuf, char *s, size_t offset) {
	size_t len = strlen(s);

	if (offset > max_offset(gbuf)) {
		offset = max_offset(gbuf);
	}
	move_gap(gbuf, offset);
	while (gap_length(gbuf) <= MIN_GAP_SIZE + len) {
		expand_gap(gbuf);
	}
	while (*s) {
		*(gbuf->gap++) = *s++;
	}
}

void
gbf_delete(GapBuffer *gbuf, size_t offset, size_t bytes) {
	size_t len;

	if (offset > max_offset(gbuf)) {
		return;
	}
	move_gap(gbuf, offset);
	if (gbuf->second == gbuf->end) {
		return;
	}
	len = second_part_length(gbuf);
	if (len < bytes) {
		bytes = len;
	}
	gbuf->second += bytes;
}

void
gbf_clear(GapBuffer *gbuf) {
	gbuf->gap = gbuf->first;
	gbuf->second = gbuf->end;
}

char
gbf_at(GapBuffer  *gbuf, size_t offset) {
	if (offset > max_offset(gbuf)) {
		return '\0';
	}
	if (gbuf->first + offset < gbuf->gap) {
		return *(gbuf->first + offset);
	} else {
		return *(gbuf->first + offset + gap_length(gbuf));
	}
}

size_t
gbf_text_length(GapBuffer *gbuf) {
	return max_offset(gbuf);
}

STATIC void
make_lps_table(char *pattern, size_t plen, size_t array[]) {
	size_t len = 0;
	size_t i = 1;

	array[0] = 0;

	while (i < plen) {
		if (pattern[i] == pattern[len]) {
			array[i] = len + 1;
			i++;
			len++;
		} else {
			if (len != 0) {
				len = array[len - 1];
			} else {
				array[i] = 0;
				i++;
			}
		}
	}
}

bool
gbf_search(GapBuffer *gbuf, char *pattern, size_t plen, size_t start, size_t *off) {
	size_t table[plen];
	size_t ti = start;
	size_t pi = 0;
	size_t text_length = gbf_text_length(gbuf);

	make_lps_table(pattern, plen, table);

	while (ti < text_length) {
		while (gbf_at(gbuf, ti) == pattern[pi]) {
			ti++;
			pi++;
			if (pi == plen) {
				*off = ti - plen;
				return true;
			}
		}
		if (pi == 0) {
			ti++;
		} else {
			pi = table[pi - 1];
		}
	}
	return false;
}

STATIC void
make_lps_table_reverse(char *pattern, size_t plen, size_t array[]) {
	size_t last = plen - 1;
	size_t len = last;
	size_t i = last - 1;

	array[last] = len;

	while (true) {
		if (pattern[i] == pattern[len]) {
			array[i] = len - 1;
			if (i == 0) {
				return;
			}
			i--;
			len--;
		} else {
			if (len != last) {
				len = array[len + 1];
			} else {
				array[i] = last;
				if (i == 0) {
					return;
				}
				i--;
			}
		}
	}
}

bool
gbf_search_reverse(GapBuffer *gbuf, char *pattern, size_t plen, size_t start, size_t *off) {
	size_t table[plen];
	size_t ti = start;
	size_t last = plen - 1;
	size_t pi = last;

	make_lps_table_reverse(pattern, plen, table);

	while (true) {
		while (gbf_at(gbuf, ti) == pattern[pi]) {
			if (pi == 0) {
				*off = ti;
				return true;
			}
			if (ti == 0) {
				return false;
			}
			ti--;
			pi--;
		}
		if (pi == last) {
			if (ti == 0) {
				return false;
			}
			ti--;
		} else {
			pi = table[pi + 1];
		}
	}
	return false;
}
