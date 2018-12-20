// A simple tool to analyze the top words in a document.
// Send standard input to this program.

#include <stdio.h>
#include <stdlib.h>
#include "build.h"

int main(int argc, const char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <max_words>\n", argv[0]);
    return -1;
  }
  int max_words = atoi(argv[1]);
  struct word_list* words = build_word_list(stdin, max_words);
  if (!words) {
    fprintf(stderr, "failed to create word list.\n");
    return -1;
  }
  for (int i = 0; i < 5 && i < words->num_words; ++i) {
    printf("word %d: %s\n", i, word_list_lookup(words, i + 1));
  }
}