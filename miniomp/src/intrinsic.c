#include "libminiomp.h"

void omp_set_num_threads (int n) {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  d->task.pr->nthreads_var = (n > 0 ? n : 1);
}

int omp_get_num_threads (void) {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  return d->task.pr->num_threads;
}

int omp_get_thread_num (void) {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  return d->id;
}

int omp_get_level (void) {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  return d->task.pr->level;
}

int omp_get_ancestor_thread_num(int level) {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  miniomp_parallel_t *p = d->task.pr;
  while (p != NULL && p->level != level+1) p = p->father;
  if (p != NULL) return p->father_id;
  printf("Ha habido error al acceder a id del padre");
  exit(1);
}
