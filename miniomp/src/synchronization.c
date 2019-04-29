#include "libminiomp.h"

// Default lock for critical sections
pthread_mutex_t miniomp_default_lock;

void GOMP_critical_start (void) {
  pthread_mutex_lock(&miniomp_default_lock);
}

void GOMP_critical_end (void) {
  pthread_mutex_unlock(&miniomp_default_lock);
}

void GOMP_critical_name_start (void **pptr) {
  // if plock is NULL it means that the lock associated to the name has not yet been allocated and initialized
  if (*pptr == NULL) {
    pthread_mutex_lock(&miniomp_default_lock);
    if (*pptr == NULL) {
      *pptr = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
      pthread_mutex_init(*pptr, NULL);
    }
    pthread_mutex_unlock(&miniomp_default_lock);
  }
  pthread_mutex_lock(*pptr);
}

void GOMP_critical_name_end (void **pptr) {
  // if plock is still NULL something went wrong
  pthread_mutex_unlock(*pptr);
}


void GOMP_barrier() {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  while (d->task.pr->barrier.exit != 0) __sync_synchronize();
  pthread_mutex_lock(&d->task.pr->barrier.lock);
  if (d->task.pr->barrier.count == 0) {
    d->task.pr->barrier.release = 0;
  }
  d->task.pr->barrier.count++;
  if (d->task.pr->barrier.count == d->task.pr->num_threads) {
    d->task.pr->barrier.count = 0;
    d->task.pr->barrier.release = 1;
  }
  pthread_mutex_unlock(&d->task.pr->barrier.lock);
  if (d->id == 0) {
    while (d->task.pr->barrier.exit != d->task.pr->num_threads - 1) __sync_synchronize();
    d->task.pr->barrier.exit = 0;
  }
  else {
    while (!d->task.pr->barrier.release)__sync_synchronize();
    __sync_fetch_and_add(&d->task.pr->barrier.exit,1);
  }
}
