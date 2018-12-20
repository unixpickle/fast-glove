#include "build.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

#define DELIMITERS " \t\n,\"'.!;"
#define IS_DELIMITER(c)                                                       \
  (c == ' ' || c == '\t' || c == '\n' || c == ',' || c == '"' || c == '\'' || \
   c == '.' || c == '!' || c == ';')

static int _doc_to_words(struct inv_word_list* list,
                         char* document,
                         int* words) {
  int word_index = 0;
  char* saveptr;
  char* token = strtok_r(document, DELIMITERS, &saveptr);
  while (token) {
    words[word_index++] = inv_word_list_lookup(list, token);
    token = strtok_r(NULL, DELIMITERS, &saveptr);
  }
  return word_index;
}

struct co_occur* build_co_occur(struct inv_word_list* list,
                                FILE* documents,
                                int window,
                                size_t max_size) {
  struct co_occur* c = co_occur_new(list->num_words + 1);
  if (!c) {
    return NULL;
  }
  char* document = (char*)malloc(max_size);
  if (!document) {
    co_occur_free(c);
    return NULL;
  }
  int* words = (int*)malloc(sizeof(int) * max_size);
  if (!words) {
    co_occur_free(c);
    free(document);
    return NULL;
  }
  int ch = 0;
  while (ch >= 0) {
    for (int i = 0; i < max_size - 1; ++i) {
      ch = fgetc(documents);
      if (ch < 0) {
        document[i] = 0;
        break;
      }
      document[i] = (char)tolower(ch);
      if (ch == 0) {
        break;
      }
    }
    document[max_size - 1] = 0;
    int num_words = _doc_to_words(list, document, words);
    if (!co_occur_add_document(c, words, num_words, window)) {
      co_occur_free(c);
      free(document);
      free(words);
      return NULL;
    }
  }
  free(document);
  free(words);
  return c;
}

struct word_list* build_word_list(FILE* documents, int max_words) {
  struct trie* t = trie_new();
  char word[WORD_MAX_LENGTH];
  int word_len = 0;
  int ch = 0;
  while (ch >= 0) {
    ch = fgetc(documents);
    if (IS_DELIMITER(ch) || ch <= 0 || word_len == WORD_MAX_LENGTH - 1) {
      if (word_len) {
        word[word_len] = 0;
        word_len = 0;
        if (!trie_add(t, word)) {
          trie_free(t);
          return NULL;
        }
      }
    } else {
      word[word_len++] = (char)tolower(ch);
    }
  }
  struct word_list* result = trie_words(t, max_words);
  trie_free(t);
  return result;
}
