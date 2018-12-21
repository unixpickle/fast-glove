#include "threads.h"

#include <unistd.h>

int default_num_threads() {
#ifdef _SC_NPROCESSORS_ONLN
  return sysconf(_SC_NPROCESSORS_ONLN);
#else
  return 4;
#endif
}
