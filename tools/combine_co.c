// Combine two co-occurrence matrices.

#include <stdio.h>
#include "co_occur.h"

static struct co_occur_pairs* read_pairs(const char* path);

int main(int argc, const char** argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <co1> <co2> <output>\n", argv[0]);
    return 1;
  }

  const char* co1_path = argv[1];
  const char* co2_path = argv[2];
  const char* output_path = argv[3];

  struct co_occur_pairs* pairs1 = read_pairs(co1_path);
  if (!pairs1) {
    fprintf(stderr, "failed to read: %s\n", co1_path);
    return 1;
  }
  struct co_occur_pairs* pairs2 = read_pairs(co2_path);
  if (!pairs2) {
    co_occur_pairs_free(pairs1);
    fprintf(stderr, "failed to read: %s\n", co2_path);
    return 1;
  }

  int num_rows = co_occur_pairs_num_rows(pairs1);
  int num_rows_2 = co_occur_pairs_num_rows(pairs2);
  if (num_rows_2 > num_rows) {
    num_rows = num_rows_2;
  }

  struct co_occur* combo = co_occur_pairs_pack(pairs1, num_rows);
  co_occur_pairs_free(pairs1);
  if (!combo) {
    co_occur_pairs_free(pairs2);
    fprintf(stderr, "failed to create combo.\n");
    return 1;
  }
  for (size_t i = 0; i < pairs2->num_pairs; ++i) {
    struct co_occur_pair* pair = &pairs2->pairs[i];
    co_occur_add(combo, pair->word1, pair->word2, pair->count);
  }
  co_occur_pairs_free(pairs2);

  FILE* f = fopen(output_path, "w+");
  if (!f) {
    fprintf(stderr, "failed to open output.\n");
    co_occur_free(combo);
    return 1;
  }
  int res = co_occur_write(combo, f);
  co_occur_free(combo);
  if (!res) {
    fprintf(stderr, "failed to write output.\n");
    return 1;
  }
  return 0;
}

static struct co_occur_pairs* read_pairs(const char* path) {
  FILE* f = fopen(path, "r");
  if (!f) {
    return NULL;
  }
  struct co_occur_pairs* res = co_occur_pairs_read(f);
  fclose(f);
  return res;
}
