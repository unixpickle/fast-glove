#ifndef __CO_OCCUR_H__
#define __CO_OCCUR_H__

#include <stddef.h>

struct co_occur_entry {
  int other;
  int count;
};

struct co_occur_bin {
  struct co_occur_entry* entries;
  int num_entries;
  int capacity;
};

struct co_occur_row {
  struct co_occur_bin* bins;
  int num_bins;
};

struct co_occur {
  struct co_occur_row* rows;
  int num_rows;
};

struct co_occur* co_occur_new(int num_rows);
struct co_occur_entry* co_occur_entry(struct co_occur* c, int word1, int word2);
void co_occur_add(struct co_occur* c, int word1, int word2);
int co_occur_get(struct co_occur* c, int word1, int word2);
void co_occur_free(struct co_occur* c);

struct co_occur_pair {
  int word1;
  int word2;
};

struct co_occur_pairs {
  struct co_occur_pair* pairs;
  size_t num_pairs;
};

struct co_occur_pairs* co_occur_pairs_new(struct co_occur* c);
void co_occur_pairs_free(struct co_occur_pairs* p);

#endif
