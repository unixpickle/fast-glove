#ifndef __UTIL_H__
#define __UTIL_H__

#include "matrix.h"
#include "word_list.h"

int read_words(const char* path,
               struct word_list** words,
               struct inv_word_list** inv);
struct matrix* read_matrix(const char* path);
float correlation(float* a, float* b, int n);

#endif
