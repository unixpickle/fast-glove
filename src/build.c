#include "build.h"
#include <ctype.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "chan.h"
#include "hash_map.h"

#define DELIMITERS " \t\n,\"'.!;"
#define IS_DELIMITER(c)                                                       \
  (c == ' ' || c == '\t' || c == '\n' || c == ',' || c == '"' || c == '\'' || \
   c == '.' || c == '!' || c == ';')

struct co_build_ctx {
  struct inv_word_list* list;
  int window;
  struct chan* documents;
  pthread_mutex_t co_lock;
  struct co_occur* co;
};

static int _doc_to_words(struct inv_word_list* list,
                         char* document,
                         int* words) {
  int word_index = 0;
  char* saveptr;
  char* token = strtok_r(document, DELIMITERS, &saveptr);
  while (token) {
    words[word_index++] = inv_word_list_lookup(list, token);
    token = strtok_r(NULL, DELIMITERS, &saveptr);
  }
  return word_index;
}

static void* _build_co_occur_worker(void* raw_ctx) {
  struct co_build_ctx* ctx = raw_ctx;
  while (1) {
    void* raw_doc = chan_recv(ctx->documents);
    if (!raw_doc) {
      return NULL;
    }
    char* doc = raw_doc;
    int doc_len = strlen(doc);
    int* words = (int*)malloc(sizeof(int) * doc_len);
    if (!words) {
      free(doc);
      continue;
    }

    int num_words = _doc_to_words(ctx->list, doc, words);
    pthread_mutex_lock(&ctx->co_lock);
    co_occur_add_document(ctx->co, words, num_words, ctx->window);
    pthread_mutex_unlock(&ctx->co_lock);
    free(doc);
    free(words);
  }
}

struct co_occur* build_co_occur(struct inv_word_list* list,
                                FILE* documents,
                                int window,
                                size_t max_size,
                                int num_threads) {
  struct co_build_ctx ctx;
  ctx.list = list;
  ctx.window = window;

  if (pthread_mutex_init(&ctx.co_lock, NULL)) {
    return NULL;
  }
  ctx.documents = chan_new(num_threads);
  if (!ctx.documents) {
    goto fail_mutex;
  }

  ctx.co = co_occur_new(list->num_words + 1);
  if (!ctx.co) {
    goto fail_documents;
  }

  char* document = (char*)malloc(max_size);
  if (!document) {
    goto fail_co;
  }

  pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
  if (!threads) {
    goto fail_document;
  }
  for (int i = 0; i < num_threads; ++i) {
    int res = pthread_create(&threads[i], NULL, _build_co_occur_worker, &ctx);
    if (res) {
      num_threads = i;
      goto fail_threads;
    }
  }

  int ch = 0;
  while (ch >= 0) {
    for (int i = 0; i < max_size - 1; ++i) {
      ch = fgetc(documents);
      if (ch < 0) {
        document[i] = 0;
        break;
      }
      document[i] = (char)tolower(ch);
      if (ch == 0) {
        break;
      }
    }
    document[max_size - 1] = 0;
    chan_send(ctx.documents, document);
    document = (char*)malloc(max_size);
    if (!document) {
      goto fail_threads;
    }
  }

  free(document);
  chan_close(ctx.documents);
  for (int i = 0; i < num_threads; ++i) {
    pthread_join(threads[i], NULL);
  }
  free(threads);
  chan_free(ctx.documents);
  pthread_mutex_destroy(&ctx.co_lock);

  return ctx.co;

fail_threads:
  chan_close(ctx.documents);
  for (int i = 0; i < num_threads; ++i) {
    pthread_join(threads[i], NULL);
  }
  free(threads);
fail_document:
  free(document);
fail_co:
  co_occur_free(ctx.co);
fail_documents:
  chan_free(ctx.documents);
fail_mutex:
  pthread_mutex_destroy(&ctx.co_lock);
  return NULL;
}

struct word_list* build_word_list(FILE* documents, int max_words) {
  struct hash_map* h = hash_map_new(max_words);
  char word[WORD_MAX_LENGTH];
  int word_len = 0;
  int ch = 0;
  while (ch >= 0) {
    ch = fgetc(documents);
    if (IS_DELIMITER(ch) || ch <= 0 || word_len == WORD_MAX_LENGTH - 1) {
      if (word_len) {
        word[word_len] = 0;
        word_len = 0;
        if (!hash_map_add(h, word)) {
          hash_map_free(h);
          return NULL;
        }
      }
    } else {
      word[word_len++] = (char)tolower(ch);
    }
  }
  struct word_list* result = hash_map_words(h, max_words);
  hash_map_free(h);
  return result;
}
