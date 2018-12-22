#include "util.h"
#include <math.h>
#include <stdio.h>

int read_words(const char* path,
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

struct matrix* read_matrix(const char* path) {
  FILE* f = fopen(path, "r");
  if (!f) {
    return NULL;
  }
  struct matrix* m = matrix_read(f);
  fclose(f);
  return m;
}

float correlation(float* a, float* b, int n) {
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
