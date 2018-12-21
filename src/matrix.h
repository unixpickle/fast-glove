#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <pthread.h>
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
void matrix_reset(struct matrix* m);
float* matrix_row(struct matrix* m, int row);
void matrix_free(struct matrix* m);

struct matrix_locks {
  pthread_mutex_t* locks;
  int num_rows;
};

struct matrix_locks* matrix_locks_new(int num_rows);
void matrix_locks_lock(struct matrix_locks* m, int row);
void matrix_locks_unlock(struct matrix_locks* m, int row);
void matrix_locks_free(struct matrix_locks* m);

#endif
