#ifndef __TRAIN_H__
#define __TRAIN_H__

#include "co_occur.h"
#include "matrix.h"

struct trainer {
  float step_size;

  // Not owned by the trainer.
  struct co_occur_pairs* pairs;

  // The main learned parameters.
  struct matrix* word_vecs;
  struct matrix* word_biases;
  struct matrix* ctx_vecs;
  struct matrix* ctx_biases;

  // Adaptive learning rates for all parameters.
  struct matrix* ada_word_vecs;
  struct matrix* ada_word_biases;
  struct matrix* ada_ctx_vecs;
  struct matrix* ada_ctx_biases;

  // Internal; used during training.
  struct matrix_locks* _locks;
  pthread_mutex_t _state_lock;
  long _iters;
  double _total_loss;
};

struct trainer* trainer_new(float step_size,
                            struct co_occur_pairs* pairs,
                            int num_rows,
                            int num_dims);
int trainer_train(struct trainer* t,
                  long num_iters,
                  int num_threads,
                  float* loss_out);
void trainer_free(struct trainer* t);

#endif
