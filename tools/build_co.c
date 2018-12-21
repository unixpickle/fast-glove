#include <stdio.h>
#include <stdlib.h>
#include "build.h"
#include "threads.h"

#define MAX_DOC_SIZE 1024

int main(int argc, const char** argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <word_file> <window_size> <output_file>\n",
            argv[0]);
    return 1;
  }

  const char* words_path = argv[1];
  int window_size = atoi(argv[2]);
  const char* output_path = argv[3];

  struct word_list* words = word_list_new();
  if (!words) {
    fprintf(stderr, "failed to allocate word list.\n");
    return 1;
  }

  FILE* word_file = fopen(words_path, "r");
  if (!word_file) {
    word_list_free(words);
    fprintf(stderr, "failed to open word list.\n");
    return 1;
  }

  int res = word_list_read(words, word_file);
  fclose(word_file);

  if (!res) {
    word_list_free(words);
    fprintf(stderr, "failed to read word list.\n");
    return 1;
  }

  struct inv_word_list* inv = inv_word_list_new(words);
  word_list_free(words);
  if (!inv) {
    fprintf(stderr, "failed to create inverse word list.\n");
    return 1;
  }

  printf("Building co-occurrence matrix...\n");
  struct co_occur* co = build_co_occur(inv, stdin, window_size, MAX_DOC_SIZE,
                                       default_num_threads());
  inv_word_list_free(inv);
  if (!co) {
    fprintf(stderr, "failed to create matrix.\n");
    return 1;
  }

  printf("Enumerating co-occurrences...\n");
  struct co_occur_pairs* pairs = co_occur_pairs_new(co);
  co_occur_free(co);
  if (!pairs) {
    fprintf(stderr, "failed to create pairs.\n");
    return 1;
  }

  printf("Got %lld pairs.\n", (long long)pairs->num_pairs);
  FILE* output = fopen(output_path, "w+");
  if (!output) {
    fprintf(stderr, "failed to open output file.\n");
    co_occur_pairs_free(pairs);
    return 1;
  }
  res = co_occur_pairs_write(pairs, output);
  fclose(output);
  co_occur_pairs_free(pairs);
  if (!res) {
    fprintf(stderr, "failed to write pairs.\n");
    return 1;
  }
  return 0;
}