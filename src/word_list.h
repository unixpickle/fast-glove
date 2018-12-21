#ifndef __WORD_LIST_H__
#define __WORD_LIST_H__

#include <stdio.h>

#define WORD_MAX_LENGTH 28

struct word {
  char word[WORD_MAX_LENGTH];
};

struct word_list {
  struct word* words;
  int num_words;
  int capacity;
};

struct word_list* word_list_new();
void word_list_free(struct word_list* list);
int word_list_read(struct word_list* list, FILE* f);
int word_list_add(struct word_list* list, const char* word);
const char* word_list_lookup(struct word_list* list, int id);

struct inv_word {
  char word[WORD_MAX_LENGTH];
  int id;
};

struct inv_word_list {
  struct inv_word* words;
  int num_words;
};

struct inv_word_list* inv_word_list_new(struct word_list* list);
int inv_word_list_lookup(struct inv_word_list* list, const char* word);
void inv_word_list_free(struct inv_word_list* list);

#endif
