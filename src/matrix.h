#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stdio.h>

struct matrix {
  float* data;
  int rows;
  int cols;
};

struct matrix* matrix_new(int rows, int cols);
struct matrix* matrix_read(FILE* f);
int matrix_write(struct matrix* m, FILE* f);
void matrix_randomize(struct matrix* m);
float* matrix_row(struct matrix* m, int row);
void matrix_free(struct matrix* m);

#endif
