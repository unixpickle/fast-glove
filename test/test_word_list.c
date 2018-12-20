#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "word_list.h"

int main() {
  struct word_list* list = word_list_new();
  assert(list);
  assert(1 == word_list_add(list, "hello"));
  assert(2 == word_list_add(list, "world"));
  assert(3 == word_list_add(list, "awesome"));
  assert(!strcmp("awesome", word_list_lookup(list, 3)));
  assert(!strcmp("world", word_list_lookup(list, 2)));
  assert(!strcmp("hello", word_list_lookup(list, 1)));

  struct inv_word_list* inv = inv_word_list_new(list);
  assert(1 == inv_word_list_lookup(inv, "hello"));
  assert(2 == inv_word_list_lookup(inv, "world"));
  assert(3 == inv_word_list_lookup(inv, "awesome"));
  assert(0 == inv_word_list_lookup(inv, "awesome123"));

  inv_word_list_free(inv);
  word_list_free(list);

  printf("PASS\n");

  return 0;
}
