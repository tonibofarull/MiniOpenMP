#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>

// Maximum number of threads to be supported by our implementation
// To be used whenever you need to dimension thread-related structures
#define MAX_THREADS 36

// To implement memory barrier (flush)
//void __atomic_thread_fence(int);
#define mb() __atomic_thread_fence(3)

#include "intrinsic.h"
#include "env.h"
#include "parallel.h"
#include "synchronization.h"
#include "loop.h"
#include "single.h"
#include "task.h"

#define END_BEHAVIOUR 0 // 0 -> exit, 1 -> handshake, 2 -> pthread_kill

typedef struct {
  pthread_t global_id;
  int id;
  bool ini;
  
  miniomp_task_t task;
} thread;


typedef struct {
  thread **pool;
  int waiting;
  int total_threads;
} thread_pool;


thread *threads;

thread_pool tpool;

