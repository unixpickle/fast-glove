// Generic helpers for building up data structures from a
// corpus. In particular, helpers for building the CO
// matrix and the word list.

#ifndef __FASTGLOVE_BUILD_H__
#define __FASTGLOVE_BUILD_H__

#include <stdio.h>
#include "co_occur.h"
#include "word_list.h"

struct co_occur* build_co_occur(struct inv_word_list* list,
                                FILE* documents,
                                int window,
                                size_t max_size,
                                int num_threads);

struct word_list* build_word_list(FILE* documents, int max_words);

#endif
