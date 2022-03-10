#ifndef DRTE_CHUNK_LIST_H
#define DRTE_CHUNK_LIST_H


/// !file
/// chunk_list.h implements a linked list of fixed size text buffers (chunks).

/// The ChunkList
typedef struct ChunkList ChunkList;

/// A descriptor of an item, stored in the list.
typedef struct ChunkListItem ChunkListItem;

/// chunk_list_new creates a new ChunkList.
/// \param chunk_size The size of a chunk. If this is zero, the default size will be used.
/// \return A new ChunkList or NULL, if out of memory.
///         [O[OThe list needs to be freed with chunk_list_free().
ChunkList *chunk_list_new(size_t chunk_size);

/// chunk_list_insert saves a string in the ChunkList.
/// \param cl A ChunkList.
/// \param text The string to save.
/// \return A ChunkListItem describing the saved text or NULL, if out of memory.
///         Free with free().
ChunkListItem *chunk_list_insert(ChunkList *cl, char *text);

/// chunk_list_get_item returns the saved text represented by a given ChunkListItem.
/// \param cl A ChunkList.
/// \param item A ChunkListItem.
/// \returns A dynamically allocated string, or NULL when out of memory.
char *chunk_list_get_item(ChunkList *cl, ChunkListItem *item);

/// chunk_list_free frees a ChunkList and sets the given pointer to NULL.
/// \param cl A ChunkList.OA
void chunk_list_free(ChunkList **cl);


#endif
