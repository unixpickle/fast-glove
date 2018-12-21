// Structures for storing a co-occurrence matrix. There
// are two representations for the CO matrix.
//
// The first representation is struct co_occur. It is easy
// to mutate, but hard to serialize or sample from. It
// uses a very primitive hashmap to make updates fast.
//
// The other representation is struct co_occur_pairs,
// which represents the matrix as a flat list of entries.
// This structure is easier to serialize and sample from,
// but cannot be modified easily.

#ifndef __CO_OCCUR_H__
#define __CO_OCCUR_H__

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>

struct co_occur_entry {
  int other;
  float count;
};

struct co_occur_bin {
  struct co_occur_entry* entries;
  int num_entries;
  int capacity;
};

struct co_occur_row {
  pthread_mutex_t lock;
  struct co_occur_bin* bins;
  int num_bins;
};

struct co_occur {
  struct co_occur_row* rows;
  int num_rows;
};

struct co_occur* co_occur_new(int num_rows);
int co_occur_add(struct co_occur* c, int word1, int word2, float amount);
float co_occur_get(struct co_occur* c, int word1, int word2);
int co_occur_add_document(struct co_occur* c,
                          int* words,
                          int num_words,
                          int window);
size_t co_occur_count(struct co_occur* c);
int co_occur_write(struct co_occur* c, FILE* f);
void co_occur_free(struct co_occur* c);

struct co_occur_pair {
  int word1;
  int word2;
  float count;
};

struct co_occur_pairs {
  struct co_occur_pair* pairs;
  size_t num_pairs;
};

struct co_occur_pairs* co_occur_pairs_new(struct co_occur* c);
struct co_occur_pairs* co_occur_pairs_read(FILE* f);
int co_occur_pairs_write(struct co_occur_pairs* p, FILE* f);
void co_occur_pairs_free(struct co_occur_pairs* p);

#endif
