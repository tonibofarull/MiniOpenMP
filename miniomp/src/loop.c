#include "libminiomp.h"

/* The *_next routines are called when the thread completes processing of 
   the iteration block currently assigned to it.  If the work-share 
   construct is bound directly to a parallel construct, then the iteration
   bounds may have been set up before the parallel.  In which case, this
   may be the first iteration for the thread.

   Returns true if there is work remaining to be performed; *ISTART and
   *IEND are filled with a new iteration block.  Returns false if all work
   has been assigned.  */

bool
GOMP_loop_dynamic_next (long *istart, long *iend) {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  miniomp_loop_t *loop = d->task.loop;
  pthread_mutex_lock(&loop->loop_lock);
  long end = loop->end;
  long incr = loop->incr;
  long cs = loop->chunk_size;
  *istart = loop->next;
  *iend = *istart + incr*cs;
  loop->next = *iend;
  pthread_mutex_unlock(&loop->loop_lock);
  if (incr > 0) {
    if (end < *iend) *iend = end;
    return *istart < end;
  }
  if (*iend < end) *iend = end;
  return *istart > end;
}

/* The *_start routines are called when first encountering a loop construct
   that is not bound directly to a parallel construct.  The first thread 
   that arrives will create the work-share construct; subsequent threads
   will see the construct exists and allocate work from it.

   START, END, INCR are the bounds of the loop; CHUNK_SIZE is the
   scheduling parameter. 

   Returns true if there's any work for this thread to perform.  If so,
   *ISTART and *IEND are filled with the bounds of the iteration block
   allocated to this thread.  Returns false if all work was assigned to
   other threads prior to this thread's arrival.  */

bool
GOMP_loop_dynamic_start (long start, long end, long incr, long chunk_size,
                         long *istart, long *iend)
{
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  pthread_mutex_lock(&d->task.loop->loop_lock);
  if (!d->task.loop->ini) fill_loop(d->task.loop,start,end,incr,chunk_size);
  pthread_mutex_unlock(&d->task.loop->loop_lock);
  return GOMP_loop_dynamic_next(istart,iend);
}

/* The GOMP_loop_end* routines are called after the thread is told that
   all loop iterations are complete.  The first version synchronize
   all threads; the nowait version does not. */

void
GOMP_loop_end (void) {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  d->task.loop = d->task.loop->next_loop;
  GOMP_barrier();
}

void
GOMP_loop_end_nowait (void) {
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  d->task.loop = d->task.loop->next_loop;
}

void fill_loop(miniomp_loop_t *loop, long start, long end, long incr, long chunk_size) {
  loop->ini = true;
  loop->start = start;
  loop->end = end;
  loop->incr = incr;
  loop->schedule = ws_DYNAMIC;
  loop->chunk_size = chunk_size;

  loop->next = start;

  loop->next_loop = malloc(sizeof(miniomp_loop_t));
  loop->next_loop->ini = false;
  loop->next_loop->prev_loop = loop;
  pthread_mutex_init(&loop->next_loop->loop_lock, NULL);
}
