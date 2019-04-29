#ifndef SINGLE_H
#define SINGLE_H

// Type declaration for single work sharing descriptor
typedef struct {
  int id_exe; // -1 -> single not assigned
  pthread_mutex_t single_lock;
} miniomp_single_t;

#endif
