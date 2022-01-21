#ifndef DRTE_CRITBIT_TREE_H
#define DRTE_CRITBIT_TREE_H


/// \file
/// critbit_tree.h implements a critbit tree, a special kind of trie.
///
/// Tries either use a large alphabet, leading to lots of wasted memory,
/// but small trees. Or they use a small alphabet, wasting less memory, but
/// creating deep trees.
/// A critbit tree is a special kind of trie. It uses a small alphabet,
/// while creating short trees. The main idea is this: Instead of saving
/// all bits of a key, the tree only saves bits that differ between two keys.
///
/// Example:
/// We want to store the keys stick and stone.
/// We first convert the strings to their binary representation:
///
///        stone: 01110011 01110100 01101111 01101110 01100101
///        stick: 01110011 01110100 01101001 01100011 01101011
///
/// We now compare the keys from left to right and find the first differing
/// bit, the critical bit.
///
///        stone: 01110011 01110100 01101111 01101110 01100101
///        stick: 01110011 01110100 01101001 01100011 01101011
///                                      x
///
/// The keys differ in bit 21, leading to the following tree:
///
///                      21                       #
///                     /  \                      #
///                    /    \                     #
///                 stick  stone                  #
///
/// The 21 bit in stick is 0, so it goes to the left. The 21 bit in
/// stone is 1, so it goes to the right.
///
/// We now want to insert stuck. We first convert it to binary:
///
///        stuck: 01110011 01110100 01110101 01100011 01101011
///
/// We now walk the tree, to find the key that best matches stuck.
///
/// The root node is 21, so we check the 21 bit in stuck.
///
///        stuck: 01110011 01110100 01110101 01100011 01101011
///                                      x
///
/// It is a 1, so we walk to the right.
///
/// We've reached a leaf, so now we compare stone and stuck:
///
///        stone: 01110011 01110100 01101111 01101110 01100101
///        stuck: 01110011 01110100 01110101 01100011 01101011
///                                    x
///
/// They differ in bit 19.
///
/// We insert two new nodes: one for 19 and one for stuck:
///
///                 19                            #
///                /  \                           #
///               /    \                          #
///              21    stuck                      #
///             /  \                              #
///            /    \                             #
///        stick   stone                          #
///
/// stuck's 19 bit is 1, so it goes to the right. The 19 bit stone and stick
/// is 0, so they both go to the left. The 19 node needs to be inserted
/// before the 21 node to keep the tries prefix sorting intact.


/// A critbit tree
typedef struct CritbitTree CritbitTree;

/// critbit_insert inserts a new key/value pair into the given tree.
/// If the key already exists, the old value is overwritten.
/// \param t A critbit tree.
/// \param key The key to insert.
/// \param value The value to add.
/// \return true, if the key/value pair was added. false otherwise.
bool critbit_insert(CritbitTree **t, char *key, size_t value);

/// critbit_lookup searches the tree for a given key.
/// \param t A critbit tree.
/// \param key The key to search for.
/// \param value A pointer to allocated memory. If key was found,
///        the pointer is set to the associated value.
/// \return true, if the key was found. false, otherwise.
bool critbit_lookup(CritbitTree *t, char *key, size_t *value);

/// critbit_keys_with_prefix finds all keys, that share a given prefix.
/// \param t A critbit tree.
/// \param prefix The prefix.
/// \return A GapBuffer filled with the matching keys. One iterm per line.
///         If no key matched the GapBuffer will be empty.
GapBuffer *critbit_keys_with_prefix(CritbitTree *t, char *prefix);

/// critbit_free frees a CritbitTree and sets the given ponter to NULL.
/// \param t A critbit tree.
void critbit_free(CritbitTree **t);


#endif
