#include <pthread.h>

#ifndef PARALLEL_H
#define PARALLEL_H
#include "single.h"
#include "loop.h"

typedef struct miniomp_barrier_t {
  int count;
  int exit;
  int release;
  pthread_mutex_t lock;
} miniomp_barrier_t;

// Type declaration for parallel descriptor (arguments needed to create pthreads)
typedef struct miniomp_parallel_t {
  int level;
  struct miniomp_parallel_t *father;
  int father_id;

  int nthreads_var;
  int num_threads;

  miniomp_barrier_t barrier;
  miniomp_single_t single;
} miniomp_parallel_t;

// Declaration of per-thread specific key
extern pthread_key_t miniomp_specifickey;

#endif
