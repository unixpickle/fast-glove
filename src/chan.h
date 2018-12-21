#ifndef __CHAN_H__
#define __CHAN_H__

#include <pthread.h>

struct chan {
  pthread_mutex_t lock;
  void** values;
  int num_values;
  int capacity;
  int closed;
};

struct chan* chan_new(int capacity);
void chan_send(struct chan* c, void* value);
void* chan_recv(struct chan* c);
void chan_close(struct chan* c);
void chan_free(struct chan* c);

#endif
