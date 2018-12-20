#include "hash_map.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct word_freq {
  char word[WORD_MAX_LENGTH];
  int freq;
};

static int _word_freq_compare(const void* data1, const void* data2) {
  const struct word_freq* f1 = data1;
  const struct word_freq* f2 = data2;
  return f2->freq - f1->freq;
}

struct word_freqs {
  struct word_freq* freqs;
  int num_freqs;
  int capacity;
};

struct word_freqs* _word_freqs_new(int capacity) {
  struct word_freqs* result = malloc(sizeof(struct word_freqs));
  if (!result) {
    return NULL;
  }
  result->freqs = malloc(sizeof(struct word_freq) * capacity);
  if (!result->freqs) {
    free(result);
    return NULL;
  }
  result->num_freqs = 0;
  result->capacity = capacity;
  return result;
}

static void _word_freqs_add(struct word_freqs* f, const char* word, int freq) {
  assert(f->num_freqs < f->capacity);
  struct word_freq* wf = &f->freqs[f->num_freqs++];
  strncpy(wf->word, word, WORD_MAX_LENGTH);
  wf->freq = freq;
}

static void _word_freqs_free(struct word_freqs* f) {
  free(f->freqs);
  free(f);
}

static unsigned int _hash_word(const char* word) {
  char full_word[4];
  bzero(full_word, sizeof(full_word));
  strncpy(full_word, word, sizeof(full_word));
  return (unsigned int)full_word[0] | ((unsigned int)full_word[1] << 8) |
         ((unsigned int)full_word[2] << 16) |
         ((unsigned int)full_word[3] << 24);
}

struct hash_map* hash_map_new(int num_bins) {
  struct hash_map* h = malloc(sizeof(struct hash_map));
  if (!h) {
    return NULL;
  }
  h->bins = malloc(sizeof(struct hash_map_bin) * num_bins);
  if (!h->bins) {
    free(h);
    return NULL;
  }
  bzero(h->bins, sizeof(struct hash_map_bin) * num_bins);
  h->num_bins = num_bins;
  h->num_words = 0;
  return h;
}

int hash_map_add(struct hash_map* h, const char* word) {
  int len = strlen(word);
  if (!len || len >= WORD_MAX_LENGTH) {
    return 0;
  }
  int bin_idx = (int)(_hash_word(word) % h->num_bins);
  struct hash_map_bin* bin = &h->bins[bin_idx];
  for (int i = 0; i < bin->num_entries; ++i) {
    struct hash_map_entry* entry = &bin->entries[i];
    if (!strcmp(entry->word, word)) {
      ++entry->count;
      return 1;
    }
  }
  if (bin->capacity == bin->num_entries) {
    struct hash_map_entry* entries = realloc(
        bin->entries, (1 + 2 * bin->capacity) * sizeof(struct hash_map_entry));
    if (!entries) {
      return 0;
    }
    bin->entries = entries;
    bin->capacity = 1 + 2 * bin->capacity;
  }
  struct hash_map_entry* entry = &bin->entries[bin->num_entries++];
  entry->count = 1;
  strncpy(entry->word, word, WORD_MAX_LENGTH);
  ++h->num_words;
  return 1;
}

struct word_list* hash_map_words(struct hash_map* h, int max_words) {
  struct word_freqs* freqs = _word_freqs_new(h->num_words);
  if (!freqs) {
    return NULL;
  }
  for (int i = 0; i < h->num_bins; ++i) {
    struct hash_map_bin* bin = &h->bins[i];
    for (int j = 0; j < bin->num_entries; ++j) {
      struct hash_map_entry* entry = &bin->entries[j];
      _word_freqs_add(freqs, entry->word, entry->count);
    }
  }
  assert(freqs->num_freqs == freqs->capacity);

  qsort(freqs->freqs, freqs->num_freqs, sizeof(struct word_freq),
        _word_freq_compare);
  if (freqs->num_freqs > max_words) {
    freqs->num_freqs = max_words;
  }

  struct word_list* result = word_list_new();
  if (!result) {
    _word_freqs_free(freqs);
    return NULL;
  }
  for (int i = 0; i < freqs->num_freqs; ++i) {
    if (!word_list_add(result, freqs->freqs[i].word)) {
      _word_freqs_free(freqs);
      word_list_free(result);
      return NULL;
    }
  }
  _word_freqs_free(freqs);

  return result;
}

void hash_map_free(struct hash_map* h) {
  for (int i = 0; i < h->num_bins; ++i) {
    struct hash_map_bin* bin = &h->bins[i];
    if (bin->entries) {
      free(bin->entries);
    }
  }
  free(h->bins);
  free(h);
}
