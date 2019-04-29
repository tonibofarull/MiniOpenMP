#include "libminiomp.h"

// Library constructor and desctructor
void init_miniomp(void) __attribute__((constructor));
void fini_miniomp(void) __attribute__((destructor));

void
init_miniomp(void) {
  parse_env();
  
  pthread_mutex_init(&miniomp_default_lock, NULL);
  
  threads = malloc(sizeof(thread)*MAX_THREADS);
  tpool = (thread_pool) {.pool=malloc(sizeof(thread *)*MAX_THREADS), 
                         .waiting=0, 
                         .total_threads=1};
  for (int i = 0; i < MAX_THREADS; ++i) {
    threads[i].ini = false;
    tpool.pool[i] = NULL;
  }
  threads[0].global_id = pthread_self();
  threads[0].id = 0;
  threads[0].ini = true;
  
  threads[0].task.pr = malloc(sizeof(miniomp_parallel_t)); 
  threads[0].task.pr->level = 0;
  threads[0].task.pr->father = NULL;
  threads[0].task.pr->nthreads_var = miniomp_icv.nthreads_var; 
  threads[0].task.pr->num_threads = 1;

  pthread_key_create(&miniomp_specifickey, NULL);
  pthread_setspecific(miniomp_specifickey, (void *) &threads[0]);
}

void *p_exit() {
  pthread_exit(NULL);
}

void handshake_kill() {
  for (int i = 0; i < MAX_THREADS; ++i) {
    if (threads[i].ini && threads[i].global_id != pthread_self()) {
      while (threads[i].task.fn != NULL) __sync_synchronize();
      threads[i].task.fn = (void *) &p_exit;
      pthread_join(threads[i].global_id, NULL);
    } 
  }
}

void force_kill() {
  for (int i = 0; i < MAX_THREADS; ++i)
    if (threads[i].ini && threads[i].global_id != pthread_self())
      pthread_kill(threads[i].global_id, 0);
}

void
fini_miniomp(void) {

  if (END_BEHAVIOUR == 1) handshake_kill();
  else if (END_BEHAVIOUR == 2) force_kill();
  
  free(tpool.pool);
  free(threads);
  pthread_key_delete(miniomp_specifickey);
  pthread_mutex_destroy(&miniomp_default_lock);
}
