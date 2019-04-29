#include "libminiomp.h"
// This file implements the PARALLEL construct

// Declaration of per-thread specific key
pthread_key_t miniomp_specifickey;

int min(int _X, int _Y) {
  return (_Y) ^ (((_X) ^ (_Y)) & -((_X) < (_Y)));
}

// This is the prototype for the Pthreads starting function
void *worker(void *args) {
  thread *d = (thread *) args;
  if (pthread_getspecific(miniomp_specifickey) == NULL) 
    pthread_setspecific(miniomp_specifickey, d);
  while (1) {
    d->task.fn(d->task.data);
    GOMP_barrier();
    pthread_mutex_lock(&miniomp_default_lock);
    ++tpool.waiting;
    for (int i = 0; i < MAX_THREADS; ++i) {
      if (tpool.pool[i] == NULL) {
        tpool.pool[i] = d;
        break;
      }
    }
    d->task.fn = NULL;
    pthread_mutex_unlock(&miniomp_default_lock);
    while (d->task.fn == NULL) __sync_synchronize();
  }
  return NULL;
}

void initialize_parallel(miniomp_parallel_t *par, int threads_execute) {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  par->level = d->task.pr->level + 1;
  par->father = d->task.pr;
  par->father_id = d->id;
  par->nthreads_var = d->task.pr->nthreads_var;
  par->num_threads = threads_execute;

  par->barrier.count = par->barrier.release = par->barrier.exit = 0;
  pthread_mutex_init(&par->barrier.lock, NULL);
  par->single.id_exe = -1;
  pthread_mutex_init(&par->single.single_lock, NULL);
}

void initialize_loop(miniomp_loop_t *loop) {
    loop->ini = false;
    pthread_mutex_init(&loop->loop_lock, NULL);
}

void delegate_work(int threads_execute, miniomp_task_t new_task) {
  int id = 1;

  threads_execute--;
  /// rellenamos primero los threads parados
  for (int i = 0; i < MAX_THREADS && threads_execute > 0 && tpool.waiting > 0; ++i) {
    if (tpool.pool[i] != NULL) { /// !null -> esta en pool
      tpool.pool[i]->id = id++;
      tpool.pool[i]->task = new_task;
      tpool.pool[i] = NULL;
      threads_execute--;
      tpool.waiting--;
    }
  }
  for (int i = 0; i < MAX_THREADS && threads_execute > 0; ++i) {
    if (!threads[i].ini) { /// inicializamos threads nuevos
      threads[i].ini = true;
      threads[i].id = id++;
      threads[i].task = new_task;
      threads_execute--;
      tpool.total_threads++;
      pthread_create(&threads[i].global_id, NULL, &worker, (void *) &threads[i]);
    }
  }
}

int how_many_threads(int num_threads) {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  if(!num_threads) num_threads = d->task.pr->nthreads_var;

  // se ejecutara con los siguientes threads
  int threads_execute = min(MAX_THREADS - (tpool.total_threads - tpool.waiting) + 1, num_threads);

  if (MAX_THREADS < num_threads)
    printf("@ ----> WARNING 1: MAX_THREADS < num_threads. Se ejecuta con los threads disponibles.\n");
  else if (threads_execute < num_threads)
    printf("@ ----> WARNING 2: Se ejecuta con los threads disponibles.\n");
  return threads_execute;
}

void destroy_par(miniomp_parallel_t *par) {
  pthread_mutex_destroy(&par->barrier.lock);
  pthread_mutex_destroy(&par->single.single_lock);
}

void destroy_loop(miniomp_loop_t *loop) {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  pthread_mutex_destroy(&loop->loop_lock);
  while (d->task.loop != loop) {
    pthread_mutex_destroy(&d->task.loop->loop_lock);
    free(d->task.loop);
    d->task.loop = d->task.loop->prev_loop;
  }
}

void
GOMP_parallel (void (*fn) (void *), void *data, unsigned num_threads, unsigned int flags) {
  pthread_mutex_lock(&miniomp_default_lock);
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);

  int threads_execute = how_many_threads(num_threads);

  /// inicializamos zona paralela
  miniomp_parallel_t par;
  initialize_parallel(&par,threads_execute);

  miniomp_loop_t loop;
  initialize_loop(&loop);

  miniomp_task_t new_task =  {fn,data,&par,&loop};
  delegate_work(threads_execute,new_task);

  pthread_mutex_unlock(&miniomp_default_lock);

  int act_id = d->id;
  miniomp_task_t act_task = d->task;
  d->id = 0;
  d->task = new_task;

  fn(data);
  GOMP_barrier();

  destroy_par(&par);
  destroy_loop(&loop);

  d->id = act_id;
  d->task = act_task;
}


// Only implement this if really needed, i.e. you find a case in which it is invoked

/* The GOMP_parallel_loop_* routines pre-initialize a work-share construct
   to avoid one synchronization once we get into the loop. The compiler
   does not invoke the *_start routine for the work-share. And of course,
   the compiler does not invoke GOMP_loop_end. These routines should create
   the team of threads to execute the work-share in parallel */
void
GOMP_parallel_loop_dynamic (void (*fn) (void *), void *data,
                            unsigned num_threads, long start, long end,
                            long incr, long chunk_size, unsigned flags)
{
//  printf("TBI: What another mess! Directly starting a parallel and a non-static for worksharing construct! I am lost!\n");
  // Here you should pre-initialize the work-sharing structures as done in
  // GOMP_loop_dynamic_start; only the master thread is doing this, the other
  // threads in the team do not reach this point

  pthread_mutex_lock(&miniomp_default_lock);
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);

  int threads_execute = how_many_threads(num_threads);

  miniomp_parallel_t par;
  initialize_parallel(&par,threads_execute);

  miniomp_loop_t loop;
  initialize_loop(&loop);
  fill_loop(&loop,start,end,incr,chunk_size);

  miniomp_task_t new_task =  {fn,data,&par,&loop};
  delegate_work(threads_execute,new_task);

  pthread_mutex_unlock(&miniomp_default_lock);

  int act_id = d->id;
  miniomp_task_t act_task = d->task;
  d->id = 0;
  d->task = new_task;

  fn(data);
  GOMP_barrier();

  destroy_par(&par);
  destroy_loop(&loop);

  d->id = act_id;
  d->task = act_task;
}
