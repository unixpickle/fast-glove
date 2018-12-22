// Compute the correlation between words.

#include <stdio.h>
#include "util.h"

int main(int argc, const char** argv) {
  if (argc < 5) {
    fprintf(stderr,
            "Usage: %s <word_list> <embed_file> <word> <other_word> "
            "[other_word ...]\n",
            argv[0]);
    return 1;
  }

  const char* word_file = argv[1];
  const char* embed_file = argv[2];
  const char* word = argv[3];
  const char** other_words = &argv[4];
  int num_other_words = argc - 4;

  struct word_list* words;
  struct inv_word_list* inv;
  if (!read_words(word_file, &words, &inv)) {
    fprintf(stderr, "failed to read word list.\n");
    return 1;
  }
  word_list_free(words);

  struct matrix* matrix = read_matrix(embed_file);
  if (!matrix) {
    fprintf(stderr, "failed to read embeddings.\n");
    inv_word_list_free(inv);
    return 1;
  }

  int word_index = inv_word_list_lookup(inv, word);
  float* word_vector = matrix_row(matrix, word_index);
  for (int i = 0; i < num_other_words; ++i) {
    int other_index = inv_word_list_lookup(inv, other_words[i]);
    float* other_vector = matrix_row(matrix, other_index);
    float corr = correlation(word_vector, other_vector, matrix->cols);
    printf("%f\n", corr);
  }

  matrix_free(matrix);
  inv_word_list_free(inv);

  return 0;
}
