#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "trie.h"

int main() {
  struct trie* t = trie_new();
  assert(t);
  assert(trie_add(t, "hey"));
  assert(trie_add(t, "hello"));
  assert(trie_add(t, "world"));
  assert(trie_add(t, "hey"));
  assert(trie_add(t, "world"));
  assert(trie_add(t, "hey"));
  assert(trie_add(t, "hey"));

  struct word_list* list = trie_words(t, 2);
  assert(list);
  assert(list->num_words == 2);
  assert(!strcmp(word_list_lookup(list, 1), "hey"));
  assert(!strcmp(word_list_lookup(list, 2), "world"));
  word_list_free(list);

  list = trie_words(t, 3);
  assert(list);
  assert(list->num_words == 3);
  assert(!strcmp(word_list_lookup(list, 1), "hey"));
  assert(!strcmp(word_list_lookup(list, 2), "world"));
  assert(!strcmp(word_list_lookup(list, 3), "hello"));
  word_list_free(list);

  trie_free(t);

  printf("PASS\n");

  return 0;
}
