#ifndef LOOP_H
#define LOOP_H

// Type declaration for loop worksharing descriptor
typedef struct miniomp_loop_t {
  bool ini;
  
  long start;           // loop bounds and increment 
  long end;
  long incr;

  int schedule;         // schedule kind for loop
  long chunk_size;
  
  long next;

  pthread_mutex_t loop_lock;
  struct miniomp_loop_t *next_loop;
  struct miniomp_loop_t *prev_loop;
} miniomp_loop_t;

void fill_loop(miniomp_loop_t *loop, long start, long end, long incr, long chunk_size);

#define ws_STATIC 	0
#define ws_STATICCHUNK 	1
#define ws_DYNAMIC 	2
#define ws_GUIDED 	3
#define ws_RUNTIME 	4
#define ws_AUTO 	5

#endif
