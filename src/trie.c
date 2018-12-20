#include "trie.h"
#include <assert.h>
#include <errno.h>
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

static void _word_freqs_add_trie(struct word_freqs* f,
                                 struct trie* t,
                                 char* word,
                                 int idx) {
  word[idx] = t->letter;
  if (t->value) {
    word[idx + 1] = 0;
    _word_freqs_add(f, word, t->value);
  }
  for (int i = 0; i < t->num_children; ++i) {
    struct trie* child = &t->children[i];
    _word_freqs_add_trie(f, child, word, idx + 1);
  }
}

static void _word_freqs_free(struct word_freqs* f) {
  free(f->freqs);
  free(f);
}

struct trie* trie_new() {
  struct trie* res = malloc(sizeof(struct trie));
  res->letter = 0;
  res->value = 0;
  res->children = NULL;
  res->num_children = 0;
  return res;
}

static void _trie_add(struct trie* t, const char* word) {
  if (word[0] == 0) {
    ++t->value;
    return 1;
  }
  for (int i = 0; i < t->num_children; ++i) {
    if (word[0] == t->children[i].letter) {
      return _trie_add(&t->children[i], word + 1);
    }
  }
  struct trie* new_children =
      realloc(t->children, sizeof(struct trie) * (t->num_children + 1));
  if (!new_children) {
    return 0;
  }
  t->children = new_children;
  struct trie* child = &t->children[t->num_children++];
  child->letter = word[0];
  child->value = 0;
  child->children = NULL;
  child->num_children = 0;
  return _trie_add(child, word + 1);
}

int trie_add(struct trie* t, const char* word) {
  int len = strlen(word);
  if (!len || len >= WORD_MAX_LENGTH) {
    return 0;
  }
  return _trie_add(t, word);
}

int trie_count(struct trie* t) {
  int count = 0;
  for (int i = 0; i < t->num_children; ++i) {
    count += trie_count(&t->children[i]);
  }
  if (t->value) {
    count += 1;
  }
  return count;
}

struct word_list* trie_words(struct trie* t, int max_words) {
  struct word_freqs* freqs = _word_freqs_new(trie_count(t));
  if (!freqs) {
    return NULL;
  }
  char word_buffer[WORD_MAX_LENGTH];
  for (int i = 0; i < t->num_children; ++i) {
    _word_freqs_add_trie(freqs, &t->children[i], word_buffer, 0);
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

static void _trie_free(struct trie* t) {
  for (int i = 0; i < t->num_children; ++i) {
    _trie_free(&t->children[i]);
  }
  free(t->children);
}

void trie_free(struct trie* t) {
  _trie_free(t);
  free(t);
}