#ifndef DRTE_GAPBUFFER_H
#define DRTE_GAPBUFFER_H

/// \file
/// gapbuffer.h implements a gapbuffer and various functions that manipulate it.
///
/// Usage:
/// \code
/// #include <stdlib.h>
///
/// #include "gapbuffer.h"
/// \endcode
///
/// TODO Describe what a gapbuffer is and how it works.

/// A gap buffer.
typedef struct GapBuffer GapBuffer;

/// gbf_new creates a new GapBuffer.
/// \return An empty GapBuffer. The GapBuffer needs to be freed with gbf_free.
/// If memory allocation fails, the function will return NULL.
GapBuffer *gbf_new(void);

/// gbf_free frees a GapBuffer and sets gbuf to NULL.
/// \param gbuf A GapBuffer.
void gbf_free(GapBuffer **gbuf);

/// gbf_text returns the contents of buf.
/// \param gbuf A GapBuffer.
/// \return A pointer to dynamically allocated memory containing the text or
/// NULL on allocation failure.
char *gbf_text(GapBuffer *gbuf);

/// gbf_insert inserts a string into the gap buffer.
/// \param gbuf A GapBuffer
/// \param s The string to insert.
/// \param offset The position.
///        If offset is out of range, the string will be inserted at the end.
void gbf_insert(GapBuffer *gbuf, char *s, size_t offset);

/// gbf_delete deletes n bytes after off.
/// \param gbuf A GapBuffer.
/// \param offset The position where the deletion starts.
///        If this is out of range, nothing will be deleted.
/// \param bytes The number of bytes to delete. If this is larger
///        than the number of remaining bytes in gbuf, the function
///        will delete to the end of the buffer.
void gbf_delete(GapBuffer *gbuf, size_t offset, size_t bytes);

/// gbf_clear clears the gapbuffer (deletes all text).
/// \param gbuf A GapBuffer.
void gbf_clear(GapBuffer *gbuf);

/// gbf_at returns the byte at position off.
/// \param gbuf A GapBuffer.
/// \param offset The position of the byte.
/// \return The character at offset or '\0' if offset is out of range.
char gbf_at(GapBuffer *gbuf, size_t offset);

/// gbf_text_length returns the length of the text contained in the GapBuffer.
/// \param gbuf A GapBuffer.
/// \return The text length in bytes.
size_t gbf_text_length(GapBuffer *gbuf);


/// gbf_search searches for a pattern in the GapBuffer, from left to right.
/// \param gbuf The GapBuffer to search.
/// \param pattern The pattern to search for.
/// \param plen The length of the pattern.
/// \param start Where to start the search.
/// \param off This will be set to the start of the match, if any.
/// \return true on success, false otherwise.
bool gbf_search(GapBuffer *gbuf, char *pattern, size_t plen, size_t start, size_t *off);

/// gbf_search_reverse searches for a pattern in the GapBuffer, from right to left.
/// \param gbuf The GapBuffer to search.
/// \param pattern The pattern to search for.
/// \param plen The length of the pattern.
/// \param start Where to start the search.
/// \param off This will be set to the start of the match, if any.
/// \return true on success, false otherwise.
bool gbf_search_reverse(GapBuffer *gbuf, char *pattern, size_t plen, size_t start, size_t *off);


#endif
