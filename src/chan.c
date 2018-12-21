#include "chan.h"
#include <assert.h>
#include <sched.h>
#include <stdlib.h>

struct chan* chan_new(int capacity) {
  struct chan* c = malloc(sizeof(struct chan));
  if (!c) {
    return NULL;
  }
  c->num_values = 0;
  c->capacity = capacity;
  c->closed = 0;

  c->values = malloc(sizeof(void*) * capacity);
  if (!c->values) {
    free(c);
    return NULL;
  }

  if (pthread_mutex_init(&c->lock, NULL)) {
    free(c);
    free(c->values);
    return NULL;
  }

  return c;
}

void chan_send(struct chan* c, void* value) {
  int sent_value = 0;
  while (!sent_value) {
    pthread_mutex_lock(&c->lock);
    assert(!c->closed);
    if (c->num_values < c->capacity) {
      c->values[c->num_values++] = value;
      sent_value = 1;
    }
    pthread_mutex_unlock(&c->lock);
    if (!sent_value) {
      sched_yield();
    }
  }
}

void* chan_recv(struct chan* c) {
  void* result = NULL;
  int has_result = 0;
  while (!has_result) {
    pthread_mutex_lock(&c->lock);
    if (c->num_values > 0) {
      result = c->values[0];
      has_result = 1;
      --c->num_values;
      for (int i = 0; i < c->num_values; ++i) {
        c->values[i] = c->values[i + 1];
      }
    } else if (c->closed) {
      has_result = 1;
    }
    pthread_mutex_unlock(&c->lock);
    if (!has_result) {
      sched_yield();
    }
  }
  return result;
}

void chan_close(struct chan* c) {
  pthread_mutex_lock(&c->lock);
  assert(!c->closed);
  c->closed = 1;
  pthread_mutex_unlock(&c->lock);
}

void chan_free(struct chan* c) {
  pthread_mutex_destroy(&c->lock);
  free(c->values);
  free(c);
}
