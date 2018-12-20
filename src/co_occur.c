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
        bin->entries, 2 * bin->capacity * sizeof(struct co_occur_entry));
    if (!new_entries) {
      return NULL;
    }
    bin->entries = new_entries;
    bin->capacity *= 2;
  }
  struct co_occur_entry* res = &bin->entries[bin->num_entries++];
  res->count = 0;
  res->other = other;
  return res;
}

static int _co_occur_row_init(struct co_occur_row* row, int num_bins) {
  row->num_bins = num_bins;
  row->bins = malloc(sizeof(struct co_occur_bin) * num_bins);
  if (!row->bins) {
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

struct co_occur_entry* co_occur_entry(struct co_occur* c,
                                      int word1,
                                      int word2) {
  if (word1 > word2) {
    return co_occur_entry(c, word2, word1);
  }
  return _co_occur_row_entry(&c->rows[word1], word2);
}

void co_occur_add(struct co_occur* c, int word1, int word2) {
  struct co_occur_entry* entry = co_occur_entry(c, word1, word2);
  ++entry->count;
}

int co_occur_get(struct co_occur* c, int word1, int word2) {
  struct co_occur_entry* entry = co_occur_entry(c, word1, word2);
  return entry->count;
}

void co_occur_free(struct co_occur* c) {
  for (int i = 0; i < c->num_rows; ++i) {
    _co_occur_row_destroy(&c->rows[i]);
  }
  free(c->rows);
  free(c);
}

struct co_occur_pairs* co_occur_pairs_new(struct co_occur* c) {
  size_t num_entries = 0;
  for (int i = 0; i < c->num_rows; ++i) {
    struct co_occur_row* row = &c->rows[i];
    for (int j = 0; j < row->num_bins; ++j) {
      struct co_occur_bin* bin = &row->bins[j];
      for (int k = 0; k < bin->num_entries; ++k) {
        struct co_occur_entry* entry = &bin->entries[k];
        if (entry->other) {
          ++num_entries;
        }
      }
    }
  }

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
        if (entry->other) {
          results[idx].word1 = i;
          results[idx].word2 = entry->other;
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

void co_occur_pairs_free(struct co_occur_pairs* p) {
  free(p->pairs);
  free(p);
}
