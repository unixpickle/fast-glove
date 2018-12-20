#include "build.h"
#include <string.h>

#define DELIMITERS " \t\n,\"'.!;"

static int _doc_to_words(struct inv_word_list* list,
                         char* document,
                         int* words) {
  int len = strlen(document);
  int word_index = 0;
  char* saveptr;
  char* token = strtok_r(document, DELIMITERS, &saveptr);
  while (token) {
    words[word_index++] = inv_word_list_lookup(list, token);
    token = strtok_r(NULL, DELIMITERS, &saveptr);
  }
  return word_index;
}

void build_co_occur(struct co_occur* c,
                    struct inv_word_list* list,
                    FILE* documents,
                    int window,
                    size_t max_size) {
  char* document = (char*)malloc(max_size);
  int* words = (int*)malloc(sizeof(int) * max_size);
  int ch;
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
    co_occur_add_document(c, words, num_words, window);
  }
  free(document);
  free(words);
}
