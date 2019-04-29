#include "libminiomp.h"

/* This routine is called when first encountering a SINGLE construct. 
   Returns true if this is the thread that should execute the clause.  */

bool
GOMP_single_start (void)
{
  thread *d = (thread *) pthread_getspecific(miniomp_specifickey);
  miniomp_single_t *sing = &d->task.pr->single;
  if (sing->id_exe == -1) {
    pthread_mutex_lock(&sing->single_lock);
    if (sing->id_exe == -1) sing->id_exe = d->id;
    pthread_mutex_unlock(&sing->single_lock);
  }
  return sing->id_exe == d->id;
}
