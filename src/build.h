#ifndef __FASTGLOVE_BUILD_H__
#define __FASTGLOVE_BUILD_H__

#include <stdio.h>
#include "co_occur.h"
#include "word_list.h"

struct co_occur* build_co_occur(struct inv_word_list* list,
                                FILE* documents,
                                int window,
                                size_t max_size);

struct word_list* build_word_list(FILE* documents, int max_words);

#endif
