#include "matrix.h"
#include <assert.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

struct matrix* matrix_new(int rows, int cols) {
  struct matrix* m = (struct matrix*)malloc(sizeof(struct matrix));
  if (!m) {
    return NULL;
  }
  m->rows = rows;
  m->cols = cols;
  m->data = (float*)malloc(rows * cols * sizeof(float));
  if (!m->data) {
    free(m);
    return NULL;
  }
  bzero(m->data, rows * cols * sizeof(float));
  return m;
}

struct matrix* matrix_read(FILE* f) {
  int rows_cols[2];
  if (fread(rows_cols, sizeof(int), 2, f) != 2) {
    return NULL;
  }
  int rows = rows_cols[0];
  int cols = rows_cols[1];

  int count = rows * cols;
  float* data = malloc(sizeof(float) * count);
  if (fread(data, sizeof(float), count, f) != count) {
    free(data);
    return NULL;
  }

  struct matrix* m = malloc(sizeof(struct matrix));
  if (!m) {
    free(data);
    return NULL;
  }
  m->data = data;
  m->rows = rows;
  m->cols = cols;
  return m;
}

int matrix_write(struct matrix* m, FILE* f) {
  int rows_cols[2] = {m->rows, m->cols};
  if (fwrite(rows_cols, sizeof(int), 2, f) != 2) {
    return 0;
  }
  int count = m->rows * m->cols;
  return fwrite(m->data, sizeof(float), count, f) == count;
}

void matrix_randomize(struct matrix* m) {
  for (int i = 0; i < m->rows * m->cols; ++i) {
    m->data[i] = (float)arc4random() / (float)0x80000000 - 1.0;
  }
}

float* matrix_row(struct matrix* m, int row) {
  assert(row >= 0 && row < m->rows);
  return m->data + (row * m->cols);
}

void matrix_free(struct matrix* m) {
  free(m->data);
  free(m);
}
