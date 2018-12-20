#ifndef __FASTGLOVE_BUILD_H__
#define __FASTGLOVE_BUILD_H__

#include <stdio.h>
#include "co_occur.h"
#include "word_list.h"

void build_co_occur(struct co_occur* c,
                    struct inv_word_list* list,
                    FILE* documents,
                    int window,
                    size_t max_size);

#endif
