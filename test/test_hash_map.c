#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "hash_map.h"

int main() {
  struct hash_map* h = hash_map_new(100);
  assert(h);
  assert(hash_map_add(h, "heyy"));
  assert(hash_map_add(h, "heyyllo"));
  assert(hash_map_add(h, "world"));
  assert(hash_map_add(h, "heyy"));
  assert(hash_map_add(h, "world"));
  assert(hash_map_add(h, "heyy"));
  assert(hash_map_add(h, "heyy"));

  struct word_list* list = hash_map_words(h, 2);
  assert(list);
  assert(list->num_words == 2);
  assert(!strcmp(word_list_lookup(list, 1), "heyy"));
  assert(!strcmp(word_list_lookup(list, 2), "world"));
  word_list_free(list);

  list = hash_map_words(h, 3);
  assert(list);
  assert(list->num_words == 3);
  assert(!strcmp(word_list_lookup(list, 1), "heyy"));
  assert(!strcmp(word_list_lookup(list, 2), "world"));
  assert(!strcmp(word_list_lookup(list, 3), "heyyllo"));
  word_list_free(list);

  hash_map_free(h);

  printf("PASS\n");

  return 0;
}
