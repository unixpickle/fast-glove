// Train embeddings using an existing co-occurrence matrix
// and word list.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "co_occur.h"
#include "threads.h"
#include "train.h"

#define NUM_DIMS 64
#define STEP_SIZE 0.05
#define ROUND_STEPS 10000000

static struct co_occur_pairs* read_pairs(const char* path,
                                         const char** extras,
                                         int num_extra);
static int save(struct matrix* words, const char* output_path);

int main(int argc, const char** argv) {
  if (argc < 4) {
    fprintf(
        stderr,
        "Usage: %s <co_occur_file> <num_epochs> <output_file> [extra_co ...]\n",
        argv[0]);
    return 1;
  }

  const char* co_occur_path = argv[1];
  int num_epochs = atoi(argv[2]);
  const char* output_path = argv[3];
  const char** extra_co_paths = &argv[4];
  int num_extra_cos = argc - 4;

  printf("Reading co-occurrences...\n");
  struct co_occur_pairs* pairs =
      read_pairs(co_occur_path, extra_co_paths, num_extra_cos);
  if (!pairs) {
    fprintf(stderr, "failed to read co-occurrences.\n");
    return 1;
  }

  int num_words = co_occur_pairs_num_rows(pairs);
  printf("Training with %d words...\n", num_words);

  struct trainer* trainer = trainer_new(STEP_SIZE, pairs, num_words, NUM_DIMS);
  if (!trainer) {
    co_occur_pairs_free(pairs);
    fprintf(stderr, "failed to create trainer.\n");
    return 1;
  }

  unsigned long long total_iters = 0;
  while (total_iters < (unsigned long long)num_epochs *
                           (unsigned long long)pairs->num_pairs) {
    float loss = 0.0;
    int res = trainer_train(trainer, ROUND_STEPS, default_num_threads(), &loss);
    if (!res) {
      fprintf(stderr, "training round failed.\n");
      trainer_free(trainer);
      co_occur_pairs_free(pairs);
      return 1;
    }
    if (isnan(loss)) {
      printf("Got NaN. Restarting...\n");
      total_iters = 0;
      trainer_reset(trainer);
      continue;
    }
    total_iters += ROUND_STEPS;
    printf("step %lld: loss=%f\n", total_iters, loss);
  }

  int res = save(trainer->word_vecs, output_path);
  trainer_free(trainer);
  co_occur_pairs_free(pairs);

  if (!res) {
    fprintf(stderr, "failed to save result.\n");
    return 1;
  }

  return 0;
}

static struct co_occur_pairs* read_pairs(const char* path,
                                         const char** extras,
                                         int num_extra) {
  FILE* f = fopen(path, "r");
  if (!f) {
    return NULL;
  }
  struct co_occur_pairs* pairs = co_occur_pairs_read(f);
  fclose(f);
  if (!pairs || !num_extra) {
    return pairs;
  }

  struct co_occur* co =
      co_occur_pairs_pack(pairs, co_occur_pairs_num_rows(pairs));
  co_occur_pairs_free(pairs);
  if (!co) {
    return NULL;
  }

  for (int i = 0; i < num_extra; ++i) {
    f = fopen(extras[i], "r");
    if (!f) {
      co_occur_free(co);
      return NULL;
    }
    pairs = co_occur_pairs_read(f);
    fclose(f);
    if (!pairs) {
      co_occur_free(co);
      return NULL;
    }
    for (int j = 0; j < pairs->num_pairs; ++j) {
      struct co_occur_pair* pair = &pairs->pairs[j];
      co_occur_add(co, pair->word1, pair->word2, pair->count);
    }
    co_occur_pairs_free(pairs);
  }

  pairs = co_occur_pairs_new(co);
  co_occur_free(co);
  return pairs;
}

static int save(struct matrix* words, const char* output_path) {
  FILE* f = fopen(output_path, "w+");
  if (!f) {
    return 0;
  }
  int res = matrix_write(words, f);
  fclose(f);
  return res;
}
