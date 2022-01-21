#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "gapbuffer.h"
#include "critbit_tree.h"


struct CritbitTree {
	bool is_leaf;

	union {
		struct {
			unsigned char critbit;
			size_t critbyte;
			CritbitTree *children[2];
		} inode;
		struct {
			char *key;
			size_t value;
		} leaf;
	};
};

static CritbitTree *new_leaf(char *key, size_t value);
static void add_tree(CritbitTree *t, GapBuffer *gbuf);

static CritbitTree *
new_leaf(char *key, size_t value) {
	CritbitTree *t = malloc(sizeof(*t));
	if (t == NULL) {
		return NULL;
	}
	memset(t, 0, sizeof(*t));
	t->is_leaf = true;
	t->leaf.key = strdup(key);
	if (t->leaf.key == NULL) {
		return NULL;
	}
	t->leaf.value = value;

	return t;
}

bool
critbit_insert(CritbitTree **t, char *key, size_t value) {
	if (*t == NULL) {
		*t = new_leaf(key, value);
		if (*t == NULL) {
			return false;
		}
		return true;
	}
	size_t newlen = strlen(key);
	CritbitTree *node = *t;

	while (!node->is_leaf) {
		char c = 0;
		if (newlen > node->inode.critbyte) {
			c = key[node->inode.critbyte];
		}
		node = node->inode.children[(c & node->inode.critbit) != 0];
	}
	if (strcmp(node->leaf.key, key) == 0) {
		node->leaf.value = value;
		return true;
	}
	size_t oldlen = strlen(node->leaf.key);
	size_t minlen = oldlen;

	if (newlen < minlen) {
		minlen = newlen;
	}

	size_t critbyte = 0;
	while (critbyte < minlen) {
		if (key[critbyte] != node->leaf.key[critbyte]) {
			break;
		} else {
			critbyte++;
		}
	}

	unsigned char critbit = 128;
	size_t shifts = 0;
	size_t bits = key[critbyte] ^ node->leaf.key[critbyte];
	while (bits != 0) {
		bits = bits >> 1;
		shifts++;
	}
	critbit = critbit >> (8 - shifts);

	size_t newdir = (key[critbyte] & critbit) != 0;
	CritbitTree **pos = t;

	while (!(*pos)->is_leaf) {
		if ((*pos)->inode.critbyte > critbyte) {
			break;
		}
		if  ((*pos)->inode.critbyte == critbyte && (*pos)->inode.critbit < critbit) {
			break;
		}
		char c = 0;
		if (newlen > (*pos)->inode.critbyte) {
			c = key[(*pos)->inode.critbyte];
		}
		pos = &(*pos)->inode.children[(c & (*pos)->inode.critbit) != 0];
	}

	CritbitTree *new = malloc(sizeof(*new));
	new->is_leaf = false;
	new->inode.critbit = critbit;
	new->inode.critbyte = critbyte;
	new->inode.children[newdir] = new_leaf(key, value);
	new->inode.children[!newdir] = *pos;
	*pos = new;

	return true;
}

bool
critbit_lookup(CritbitTree *t, char *key, size_t *value) {
	if (t == NULL) {
		return false;
	}
	size_t keylen = strlen(key);
	while (!t->is_leaf) {
		char c = 0;
		if (keylen > t->inode.critbyte) {
			c = key[t->inode.critbyte];
		}
		t = t->inode.children[(c & t->inode.critbit) != 0];
	}
	if (strcmp(t->leaf.key, key) == 0) {
		*value = t->leaf.value;
		return true;
	} else {
		return false;
	}
}

void
critbit_free(CritbitTree **t) {
	if (*t == NULL) {
		return;
	}
	if ((*t)->is_leaf) {
		free((*t)->leaf.key);
		free(*t);
		*t = NULL;
	} else {
		critbit_free(&(*t)->inode.children[0]);
		critbit_free(&(*t)->inode.children[1]);
		free(*t);
	}
	*t = NULL;
}

static void
add_tree(CritbitTree *t, GapBuffer *gbuf) {
	if (t->is_leaf) {
		gbf_insert(gbuf, t->leaf.key, gbf_text_length(gbuf));
		gbf_insert(gbuf, "\n", gbf_text_length(gbuf));
	} else {
		add_tree(t->inode.children[0], gbuf);
		add_tree(t->inode.children[1], gbuf);
	}
}

GapBuffer *
critbit_keys_with_prefix(CritbitTree *t, char *prefix) {
	size_t plen = strlen(prefix);
	GapBuffer *gbuf = gbf_new();

	if (t == NULL) {
		return gbuf;
	}
	if (plen == 0) {
		add_tree(t, gbuf);
		return gbuf;
	}

	while (!t->is_leaf) {
		if (t->inode.critbyte >= plen) {
			break;
		}
		char c = 0;
		if (plen > t->inode.critbyte) {
			c = prefix[t->inode.critbyte];
		}
		t = t->inode.children[(c & t->inode.critbit) != 0];
	}
	if (t->is_leaf) {
		if (strncmp(t->leaf.key, prefix, plen) == 0) {
			gbf_insert(gbuf, t->leaf.key, gbf_text_length(gbuf));
			gbf_insert(gbuf, "\n", gbf_text_length(gbuf));
		}
	} else {
		add_tree(t, gbuf);
	}
	return gbuf;
}
