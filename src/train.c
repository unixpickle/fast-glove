#include "train.h"
#include <math.h>
#include <stdlib.h>
#include <strings.h>

#define COUNT_MAX 100.0

static float _count_weight(float count) {
  if (count > COUNT_MAX) {
    return count;
  }
  return powf(count / COUNT_MAX, 3.0 / 4.0);
}

static float _dot(float* x, float* y, int n) {
  float res = 0;
  for (int i = 0; i < n; ++i) {
    res += x[i] * y[i];
  }
  return res;
}

static double _trainer_train_iter(struct trainer* t) {
  int pair_idx = random() % t->pairs->num_pairs;
  struct co_occur_pair* pair = &t->pairs->pairs[pair_idx];

  // No risk of deadlock because the order is consistent.
  matrix_locks_lock(t->_locks, pair->word1);
  matrix_locks_lock(t->_locks, pair->word2);

  // Randomize which matrix we use for each word.
  int word = pair->word1;
  int ctx = pair->word2;
  if (random() & 1) {
    word = pair->word2;
    ctx = pair->word1;
  }

  float weight = _count_weight(pair->count);
  float target = logf(pair->count);

  float* word_vec = matrix_row(t->word_vecs, word);
  float* word_bias = matrix_row(t->word_biases, word);
  float* ctx_vec = matrix_row(t->ctx_vecs, ctx);
  float* ctx_bias = matrix_row(t->ctx_biases, ctx);

  float* ada_word_vec = matrix_row(t->ada_word_vecs, word);
  float* ada_word_bias = matrix_row(t->ada_word_biases, word);
  float* ada_ctx_vec = matrix_row(t->ada_ctx_vecs, ctx);
  float* ada_ctx_bias = matrix_row(t->ada_ctx_biases, ctx);

  float entry =
      _dot(word_vec, ctx_vec, t->word_vecs->cols) + *word_bias + *ctx_bias;
  float diff = entry - target;
  float upstream_grad = weight * diff;

  float bias_grad_sq = powf(upstream_grad, 2.0);
  *ada_word_bias += bias_grad_sq;
  *ada_ctx_bias += bias_grad_sq;
  *word_bias -= t->step_size * upstream_grad / sqrtf(*ada_word_bias);
  *ctx_bias -= t->step_size * upstream_grad / sqrtf(*ada_ctx_bias);

  for (int i = 0; i < t->word_vecs->rows; ++i) {
    float word_grad = upstream_grad * ctx_vec[i];
    float ctx_grad = upstream_grad * word_vec[i];
    ada_word_vec[i] += powf(word_grad, 2.0);
    ada_ctx_vec[i] += powf(ctx_grad, 2.0);
    word_vec[i] -= t->step_size * word_grad / sqrtf(ada_word_vec[i]);
    ctx_vec[i] -= t->step_size * ctx_grad / sqrtf(ada_ctx_vec[i]);
  }

  matrix_locks_unlock(t->_locks, pair->word2);
  matrix_locks_unlock(t->_locks, pair->word1);

  return powf(diff, 2.0);
}

static void* _trainer_train_thread(void* raw_trainer) {
  struct trainer* t = raw_trainer;
  while (1) {
    double loss = _trainer_train_iter(t);

    pthread_mutex_lock(&t->_state_lock);
    long iters = --t->_iters;
    t->_total_loss += loss;
    pthread_mutex_unlock(&t->_state_lock);

    if (iters <= 0) {
      return NULL;
    }
  }
}

struct trainer* trainer_new(float step_size,
                            struct co_occur_pairs* pairs,
                            int num_rows,
                            int num_dims) {
  struct trainer* t = malloc(sizeof(struct trainer));
  bzero(t, sizeof(struct trainer));
  t->step_size = step_size;
  t->pairs = pairs;

  if (pthread_mutex_init(&t->_state_lock, NULL)) {
    free(t);
    return NULL;
  }

  t->_locks = matrix_locks_new(num_rows);
  if (!t->_locks) {
    trainer_free(t);
    return NULL;
  }

  struct matrix** weight_mats[4] = {&t->word_vecs, &t->ctx_vecs,
                                    &t->ada_word_vecs, &t->ada_ctx_vecs};
  for (int i = 0; i < 4; ++i) {
    *weight_mats[i] = matrix_new(num_rows, num_dims);
    if (!*weight_mats[i]) {
      trainer_free(t);
      return NULL;
    }
  }

  matrix_randomize(t->word_vecs);
  matrix_randomize(t->ctx_vecs);

  struct matrix** bias_mats[4] = {&t->word_biases, &t->ctx_biases,
                                  &t->ada_word_biases, &t->ada_ctx_biases};
  for (int i = 0; i < 4; ++i) {
    *bias_mats[i] = matrix_new(num_rows, 1);
    if (!*bias_mats[i]) {
      trainer_free(t);
      return NULL;
    }
  }

  return t;
}

int trainer_train(struct trainer* t,
                  long num_iters,
                  int num_threads,
                  float* loss_out) {
  t->_iters = num_iters;
  t->_total_loss = 0;

  pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
  if (!threads) {
    return 0;
  }
  for (int i = 0; i < num_threads; ++i) {
    int res = pthread_create(&threads[i], NULL, _trainer_train_thread, t);
    if (res) {
      pthread_mutex_lock(&t->_state_lock);
      t->_iters = 0;
      pthread_mutex_unlock(&t->_state_lock);
      for (int j = 0; j < i; ++j) {
        pthread_join(threads[j], NULL);
      }
      free(threads);
      return 0;
    }
  }

  for (int i = 0; i < num_threads; ++i) {
    pthread_join(threads[i], NULL);
  }

  if (t->_iters > 0) {
    // Threads shutdown before completion.
    return 0;
  }

  if (loss_out) {
    long total_iters = num_iters - t->_iters;
    *loss_out = (float)(t->_total_loss / (double)total_iters);
  }

  return 1;
}

void trainer_free(struct trainer* t) {
  if (t->_locks) {
    matrix_locks_free(t->_locks);
  }
  if (t->word_vecs) {
    matrix_free(t->word_vecs);
  }
  if (t->word_biases) {
    matrix_free(t->word_biases);
  }
  if (t->ctx_vecs) {
    matrix_free(t->ctx_vecs);
  }
  if (t->ctx_biases) {
    matrix_free(t->ctx_biases);
  }
  if (t->ada_word_vecs) {
    matrix_free(t->ada_word_vecs);
  }
  if (t->ada_word_biases) {
    matrix_free(t->ada_word_biases);
  }
  if (t->ada_ctx_vecs) {
    matrix_free(t->ada_ctx_vecs);
  }
  if (t->ada_ctx_biases) {
    matrix_free(t->ada_ctx_biases);
  }
  pthread_mutex_destroy(&t->_state_lock);
}
