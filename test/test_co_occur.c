#include <assert.h>
#include <stdio.h>
#include "co_occur.h"

int main() {
  struct co_occur* c = co_occur_new(10);
  assert(c);

  co_occur_add(c, 3, 2);
  assert(co_occur_get(c, 3, 2) == 1);
  assert(co_occur_get(c, 2, 3) == 1);
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      if ((i == 2 && j == 3) || (j == 2 && i == 3)) {
        continue;
      }
      assert(co_occur_get(c, i, j) == 0);
    }
  }

  co_occur_add(c, 3, 2);
  assert(co_occur_get(c, 3, 2) == 2);
  assert(co_occur_get(c, 2, 3) == 2);
  co_occur_add(c, 0, 0);
  assert(co_occur_get(c, 3, 2) == 2);
  assert(co_occur_get(c, 0, 0) == 1);
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      if ((i == 2 && j == 3) || (j == 2 && i == 3)) {
        continue;
      } else if (i == 0 && j == 0) {
        continue;
      }
      assert(co_occur_get(c, i, j) == 0);
    }
  }

  co_occur_free(c);

  c = co_occur_new(10);
  assert(c);
  co_occur_add(c, 2, 3);
  co_occur_add(c, 3, 4);
  co_occur_add(c, 4, 5);
  struct co_occur_pairs* pairs = co_occur_pairs_new(c);
  assert(pairs->num_pairs == 3);
  assert(pairs->pairs[0].word1 == 2);
  assert(pairs->pairs[0].word2 == 3);
  assert(pairs->pairs[1].word1 == 3);
  assert(pairs->pairs[1].word2 == 4);
  assert(pairs->pairs[2].word1 == 4);
  assert(pairs->pairs[2].word2 == 5);
  co_occur_pairs_free(pairs);
  co_occur_free(c);

  printf("PASS\n");

  return 0;
}
