#include <stdlib.h>
#include <string.h>

#include "chunk_list.h"


static size_t DEFAULT_CHUNK_SIZE = 8096;

typedef struct Chunk {
	char *chunk;
	size_t filled;
	struct Chunk *next;
} Chunk;

struct ChunkList {
	size_t chunk_size;
	Chunk *first;
};

struct ChunkListItem {
	size_t segment;
	size_t start;
	size_t length;
};

static Chunk * new_chunk(ChunkList *cl);

static Chunk *
new_chunk(ChunkList *cl) {
	Chunk *c = malloc(sizeof(*c));
	if (c == NULL) {
		return NULL;
	}
	c->chunk = malloc(cl->chunk_size);
	if (c->chunk == NULL) {
		return NULL;
	}
	c->filled = 0;
	c->next = 0;

	return c;
}

ChunkList *
chunk_list_new(size_t chunk_size) {
	ChunkList *cl = malloc(sizeof(*cl));
	if (cl == NULL) {
		return NULL;
	}

	if (chunk_size != 0) {
		cl->chunk_size = chunk_size;
	} else {
		cl->chunk_size = DEFAULT_CHUNK_SIZE;
	}
	cl->first = new_chunk(cl);
	if (cl->first == NULL) {
		free(cl);
		return NULL;
	}

	return cl;
}

// TODO: the function should check if we have enough memory, before trying to copy
// the new item.
ChunkListItem *
chunk_list_insert(ChunkList *cl, char *text) {
	Chunk *c = cl->first;
	size_t segment = 0;
	size_t length = strlen(text);

	while (c->next != NULL) {
		c = c->next;
		segment++;
	}
	ChunkListItem *item = malloc(sizeof(*item));
	if (item == NULL) {
		return NULL;
	}
	item->segment = segment;
	item->start = c->filled;
	item->length = length;

	size_t written = 0;
	while (written < length) {
		while (written < length && cl->chunk_size - c->filled > 0) {
			c->chunk[c->filled] = text[written];
			c->filled++;
			written++;
		}
		if (cl->chunk_size - c->filled == 0) {
			c->next = new_chunk(cl);
			if (c->next == NULL) {
				return NULL;
			}
			c = c->next;
		}
	}
	return item;
}

char *
chunk_list_get_item(ChunkList *cl, ChunkListItem *item) {
	Chunk *c = cl->first;
	size_t segment = 0;
	size_t chunk_size = cl->chunk_size;

	char *buffer = malloc(item->length + 1);
	if (buffer == NULL) {
		return NULL;
	}

	while (item->segment != segment) {
		c = c->next;
		segment++;
	}
	size_t written = 0;
	size_t index = item->start;
	while (written < item->length) {
		while (written < item->length && index < chunk_size) {
			buffer[written] = c->chunk[index];
			written++;
			index++;
		}
		if (index == chunk_size) {
			c = c->next;
			index = 0;
		}
	}
	buffer[item->length] = '\0';
	return buffer;
}

void
chunk_list_free(ChunkList **cl) {
	Chunk *c = (*cl)->first;

	while (c != NULL) {
		Chunk *next = c->next;
		free(c->chunk);
		free(c);
		c = next;
	}
	free(*cl);
	*cl = NULL;
}
