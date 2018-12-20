#include "word_list.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int _inv_word_compare(const void* word1, const void* word2) {
  struct inv_word* inv1 = word1;
  struct inv_word* inv2 = word2;
  return strncmp(inv1->word, inv2->word, WORD_MAX_LENGTH);
}

struct word_list* word_list_alloc() {
  struct word_list* result = malloc(sizeof(struct word_list));
  if (!result) {
    return NULL;
  }
  result->words = malloc(sizeof(struct word));
  if (!result->words) {
    free(result);
    return NULL;
  }
  result->num_words = 0;
  result->capacity = 1;
}

void word_list_free(struct word_list* list) {
  free(list->words);
  free(list);
}

int word_list_add(struct word_list* list, const char* word) {
  if (list->capacity == list->num_words) {
    struct word* words =
        realloc(list->words, (list->capacity * 2 * sizeof(struct word)));
    if (!words) {
      return 0;
    }
    list->words = words;
    list->capacity *= 2;
  }
  strncpy(list->words[list->num_words++].word, word, WORD_MAX_LENGTH);
  return list->num_words;
}

const char* word_list_lookup(struct word_list* list, int id) {
  if (!id) {
    return "<UNKNOWN>";
  }
  if (id < 0 || id > list->num_words) {
    return "<UNKNOWN>";
  }
  return list->words[id - 1].word;
}

struct inv_word_list* inv_word_list_new(struct word_list* list) {
  struct inv_word_list* result = malloc(sizeof(struct inv_word_list));
  if (!result) {
    return NULL;
  }
  result->words = malloc(sizeof(struct inv_word) * list->num_words);
  if (!result->words) {
    free(result);
    return NULL;
  }
  result->num_words = list->num_words;
  for (int i = 0; i < list->num_words; ++i) {
    memcpy(result->words[i].word, list->words[i].word, WORD_MAX_LENGTH);
    result->words[i].id = i + 1;
  }
  qsort(result->words, result->num_words, sizeof(struct inv_word),
        _inv_word_compare);
}

int inv_word_list_lookup(struct inv_word_list* list, const char* word) {
  struct inv_word key;
  strncpy(key.word, word, WORD_MAX_LENGTH);
  struct inv_word* res = bsearch(&key, list->words, list->num_words,
                                 sizeof(struct inv_word), _inv_word_compare);
  if (res) {
    return res->id;
  }
  return 0;
}

void inv_word_list_free(struct inv_word_list* list) {
  free(list->words);
  free(list);
}
