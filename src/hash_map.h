#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

#include "word_list.h"

struct hash_map_entry {
  char word[WORD_MAX_LENGTH];
  int count;
};

struct hash_map_bin {
  struct hash_map_entry* entries;
  int num_entries;
  int capacity;
};

struct hash_map {
  struct hash_map_bin* bins;
  int num_bins;
  int num_words;
};

struct hash_map* hash_map_new(int num_bins);
int hash_map_add(struct hash_map* h, const char* word);
struct word_list* hash_map_words(struct hash_map* h, int max_words);
void hash_map_free(struct hash_map* h);

#endif
