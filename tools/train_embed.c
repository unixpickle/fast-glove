// Train embeddings using an existing co-occurrence matrix
// and word list.

#include <stdio.h>
#include <stdlib.h>
#include "co_occur.h"
#include "train.h"

#define NUM_THREADS 4
#define NUM_DIMS 64
#define STEP_SIZE 0.01
#define ROUND_STEPS 1000000

static struct co_occur_pairs* read_pairs(const char* path);
static int count_words(struct co_occur_pairs* pairs);
static int save(struct matrix* words, const char* output_path);

int main(int argc, const char** argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <co_occur_file> <num_epochs> <output_file>\n",
            argv[0]);
    return 1;
  }

  const char* co_occur_path = argv[1];
  int num_epochs = atoi(argv[2]);
  const char* output_path = argv[3];

  printf("Reading co-occurrences...\n");
  struct co_occur_pairs* pairs = read_pairs(co_occur_path);
  if (!pairs) {
    fprintf(stderr, "failed to read co-occurrences.\n");
    return 1;
  }

  int num_words = count_words(pairs);
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
    int res = trainer_train(trainer, ROUND_STEPS, NUM_THREADS, &loss);
    if (!res) {
      fprintf(stderr, "training round failed.\n");
      trainer_free(trainer);
      co_occur_pairs_free(pairs);
      return 1;
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

static struct co_occur_pairs* read_pairs(const char* path) {
  FILE* f = fopen(path, "r");
  if (!f) {
    return NULL;
  }
  struct co_occur_pairs* pairs = co_occur_pairs_read(f);
  fclose(f);
  return pairs;
}

static int count_words(struct co_occur_pairs* pairs) {
  int num_words = 0;
  for (int i = 0; i < pairs->num_pairs; ++i) {
    int word = pairs->pairs[i].word2;
    if (word >= num_words) {
      num_words = word + 1;
    }
  }
  return num_words;
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
