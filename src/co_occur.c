#include "co_occur.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int _co_occur_num_bins(int num_rows) {
  for (int i = 0; i < num_rows; ++i) {
    if (i * i >= num_rows) {
      return i;
    }
  }
  return num_rows;
}

static struct co_occur_entry* _co_occur_bin_entry(struct co_occur_bin* bin,
                                                  int other) {
  for (int i = 0; i < bin->num_entries; ++i) {
    if (bin->entries[i].other == other) {
      return &bin->entries[i];
    }
  }
  if (bin->num_entries == bin->capacity) {
    struct co_occur_entry* new_entries = realloc(
        bin->entries, (1 + 2 * bin->capacity) * sizeof(struct co_occur_entry));
    if (!new_entries) {
      return NULL;
    }
    bin->entries = new_entries;
    bin->capacity = 1 + 2 * bin->capacity;
  }
  struct co_occur_entry* res = &bin->entries[bin->num_entries++];
  res->count = 0;
  res->other = other;
  return res;
}

static int _co_occur_row_init(struct co_occur_row* row, int num_bins) {
  if (pthread_mutex_init(&row->lock, NULL)) {
    return 0;
  }
  row->num_bins = num_bins;
  row->bins = malloc(sizeof(struct co_occur_bin) * num_bins);
  if (!row->bins) {
    pthread_mutex_destroy(&row->lock);
    return 0;
  }
  bzero(row->bins, sizeof(struct co_occur_bin) * num_bins);
  return 1;
}

static void _co_occur_row_destroy(struct co_occur_row* row) {
  for (int i = 0; i < row->num_bins; ++i) {
    if (row->bins[i].entries) {
      free(row->bins[i].entries);
    }
  }
  free(row->bins);
  pthread_mutex_destroy(&row->lock);
}

static struct co_occur_entry* _co_occur_row_entry(struct co_occur_row* row,
                                                  int other) {
  int bin_idx = (other % row->num_bins);
  struct co_occur_bin* bin = &row->bins[bin_idx];
  return _co_occur_bin_entry(bin, other);
}

struct co_occur* co_occur_new(int num_rows) {
  struct co_occur* result = malloc(sizeof(struct co_occur));
  result->num_rows = num_rows;
  result->rows = malloc(sizeof(struct co_occur_row) * num_rows);
  if (!result->rows) {
    free(result);
  }
  int num_bins = _co_occur_num_bins(num_rows);
  for (int i = 0; i < num_rows; ++i) {
    if (!_co_occur_row_init(&result->rows[i], num_bins)) {
      for (int j = 0; j < i; ++j) {
        _co_occur_row_destroy(&result->rows[j]);
      }
      free(result->rows);
      free(result);
      return NULL;
    }
  }
  return result;
}

static struct co_occur_entry* _co_occur_entry(struct co_occur* c,
                                              int word1,
                                              int word2) {
  if (word1 > word2) {
    return _co_occur_entry(c, word2, word1);
  }
  struct co_occur_row* row = &c->rows[word1];
  pthread_mutex_lock(&row->lock);
  struct co_occur_entry* res = _co_occur_row_entry(row, word2);
  if (!res) {
    pthread_mutex_unlock(&row->lock);
  }
  return res;
}

static void _co_occur_entry_unlock(struct co_occur* c, int word1, int word2) {
  if (word1 > word2) {
    _co_occur_entry_unlock(c, word2, word1);
  } else {
    struct co_occur_row* row = &c->rows[word1];
    pthread_mutex_unlock(&row->lock);
  }
}

int co_occur_add(struct co_occur* c, int word1, int word2, float amount) {
  struct co_occur_entry* entry = _co_occur_entry(c, word1, word2);
  if (!entry) {
    return 0;
  }
  entry->count += amount;
  _co_occur_entry_unlock(c, word1, word2);
  return 1;
}

float co_occur_get(struct co_occur* c, int word1, int word2) {
  struct co_occur_entry* entry = _co_occur_entry(c, word1, word2);
  if (!entry) {
    return 0;
  }
  float res = entry->count;
  _co_occur_entry_unlock(c, word1, word2);
  return res;
}

int co_occur_add_document(struct co_occur* c,
                          int* words,
                          int num_words,
                          int window) {
  for (int i = 0; i < num_words; ++i) {
    for (int j = i - 1; j >= 0 && j >= i - window; --j) {
      if (!co_occur_add(c, words[i], words[j], 1.0 / (float)(i - j))) {
        return 0;
      }
    }
  }
  return 1;
}

size_t co_occur_count(struct co_occur* c) {
  size_t num_entries = 0;
  for (int i = 0; i < c->num_rows; ++i) {
    struct co_occur_row* row = &c->rows[i];
    for (int j = 0; j < row->num_bins; ++j) {
      struct co_occur_bin* bin = &row->bins[j];
      for (int k = 0; k < bin->num_entries; ++k) {
        struct co_occur_entry* entry = &bin->entries[k];
        if (entry->count) {
          ++num_entries;
        }
      }
    }
  }
  return num_entries;
}

int co_occur_write(struct co_occur* c, FILE* f) {
  for (int i = 0; i < c->num_rows; ++i) {
    struct co_occur_row* row = &c->rows[i];
    for (int j = 0; j < row->num_bins; ++j) {
      struct co_occur_bin* bin = &row->bins[j];
      for (int k = 0; k < bin->num_entries; ++k) {
        struct co_occur_entry* entry = &bin->entries[k];
        if (entry->count) {
          struct co_occur_pair pair;
          pair.word1 = i;
          pair.word2 = entry->other;
          pair.count = entry->count;
          if (fwrite(&pair, 1, sizeof(struct co_occur_pair), f) !=
              sizeof(struct co_occur_pair)) {
            return 0;
          }
        }
      }
    }
  }
  return 1;
}

void co_occur_free(struct co_occur* c) {
  for (int i = 0; i < c->num_rows; ++i) {
    _co_occur_row_destroy(&c->rows[i]);
  }
  free(c->rows);
  free(c);
}

struct co_occur_pairs* co_occur_pairs_new(struct co_occur* c) {
  size_t num_entries = co_occur_count(c);
  size_t idx = 0;
  struct co_occur_pair* results =
      malloc(sizeof(struct co_occur_pair) * num_entries);
  if (!results) {
    return NULL;
  }
  for (int i = 0; i < c->num_rows; ++i) {
    struct co_occur_row* row = &c->rows[i];
    for (int j = 0; j < row->num_bins; ++j) {
      struct co_occur_bin* bin = &row->bins[j];
      for (int k = 0; k < bin->num_entries; ++k) {
        struct co_occur_entry* entry = &bin->entries[k];
        if (entry->count) {
          results[idx].word1 = i;
          results[idx].word2 = entry->other;
          results[idx].count = entry->count;
          ++idx;
        }
      }
    }
  }

  struct co_occur_pairs* result = malloc(sizeof(struct co_occur_pair));
  if (!result) {
    free(results);
  }
  result->num_pairs = num_entries;
  result->pairs = results;
  return result;
}

struct co_occur_pairs* co_occur_pairs_read(FILE* f) {
  fseek(f, 0, SEEK_END);
  size_t size = (size_t)ftello(f);
  size_t num_pairs = size / sizeof(struct co_occur_pair);
  rewind(f);

  struct co_occur_pair* buf = malloc(sizeof(struct co_occur_pair) * num_pairs);
  size_t res = fread(buf, sizeof(struct co_occur_pair), num_pairs, f);
  if (res != num_pairs) {
    free(buf);
    return NULL;
  }

  struct co_occur_pairs* pairs = malloc(sizeof(struct co_occur_pairs));
  if (!pairs) {
    free(buf);
    return NULL;
  }
  pairs->pairs = buf;
  pairs->num_pairs = num_pairs;
  return pairs;
}

int co_occur_pairs_write(struct co_occur_pairs* p, FILE* f) {
  size_t res = fwrite(p->pairs, sizeof(struct co_occur_pair), p->num_pairs, f);
  return res == p->num_pairs;
}

void co_occur_pairs_free(struct co_occur_pairs* p) {
  free(p->pairs);
  free(p);
}
