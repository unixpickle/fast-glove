// Use trained embeddings to find the neighbors of a
// specified word.

#include <math.h>
#include <stdio.h>
#include "matrix.h"
#include "word_list.h"

static int read_words(const char* path,
                      struct word_list** words,
                      struct inv_word_list** inv);
static struct matrix* read_matrix(const char* path);
static int next_match(struct matrix* m, float* worse_than, int row);
static float correlation(float* a, float* b, int n);

int main(int argc, const char** argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <word_list> <embed_file> <word>\n", argv[0]);
    return 1;
  }

  int res = 0;

  const char* word_list_path = argv[1];
  const char* embed_path = argv[2];
  const char* word = argv[3];

  struct word_list* words;
  struct inv_word_list* inv;
  if (!read_words(word_list_path, &words, &inv)) {
    fprintf(stderr, "failed to read word list.\n");
    return 1;
  }

  struct matrix* matrix = read_matrix(embed_path);
  if (!matrix) {
    fprintf(stderr, "failed to read embeddings.\n");
    res = 1;
    goto cleanup_1;
  }

  int word_index = inv_word_list_lookup(inv, word);
  printf("neighbors for word id: %d\n", word_index);

  float best_corr = 2.0;
  for (int i = 0; i < 10; ++i) {
    int match = next_match(matrix, &best_corr, word_index);
    printf("%d %s (%f)\n", match, word_list_lookup(words, match), best_corr);
  }

  matrix_free(matrix);
cleanup_1:
  word_list_free(words);
  inv_word_list_free(inv);

  return res;
}

static int read_words(const char* path,
                      struct word_list** words,
                      struct inv_word_list** inv) {
  FILE* f = fopen(path, "r");
  if (!f) {
    return 0;
  }

  *words = word_list_new();
  if (!*words) {
    fclose(f);
    return 0;
  }

  int res = word_list_read(*words, f);
  fclose(f);

  if (!res) {
    word_list_free(*words);
    return 0;
  }

  *inv = inv_word_list_new(*words);
  if (!*inv) {
    word_list_free(*words);
    return 0;
  }

  return 1;
}

static struct matrix* read_matrix(const char* path) {
  FILE* f = fopen(path, "r");
  if (!f) {
    return NULL;
  }
  struct matrix* m = matrix_read(f);
  fclose(f);
  return m;
}

static int next_match(struct matrix* m, float* worse_than, int row) {
  float* vec = matrix_row(m, row);
  float best_corr = -2;
  int best_row = 0;
  for (int i = 0; i < m->rows; ++i) {
    if (i == row) {
      continue;
    }
    float* other = matrix_row(m, i);
    float corr = correlation(vec, other, m->cols);
    if (corr < *worse_than && corr > best_corr) {
      best_corr = corr;
      best_row = i;
    }
  }
  *worse_than = best_corr;
  return best_row;
}

static float correlation(float* a, float* b, int n) {
  float dot = 0;
  float magA = 0;
  float magB = 0;
  for (int i = 0; i < n; ++i) {
    dot += a[i] * b[i];
    magA += a[i] * a[i];
    magB += b[i] * b[i];
  }
  return dot / (sqrtf(magA) * sqrtf(magB));
}
