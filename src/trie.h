#ifndef __TRIE_H__
#define __TRIE_H__

#include "word_list.h"

struct trie {
  char letter;
  int value;

  struct trie* children;
  int num_children;
};

struct trie* trie_new();
int trie_add(struct trie* t, const char* word);
int trie_count(struct trie* t);
struct word_list* trie_words(struct trie* t, int max_words);
void trie_free(struct trie* t);

#endif
