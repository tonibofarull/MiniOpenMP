#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>	/* OpenMP */

// PARALLEL

void foo1() {
  omp_set_nested(1);
  #pragma omp parallel
  {
    printf("1 -> %d\n",omp_get_thread_num());
    #pragma omp parallel
    {
      printf("2 -> %d -> %d\n",omp_get_thread_num(),
                               omp_get_ancestor_thread_num(1));
      #pragma omp parallel
      printf("3 -> %d -> %d -> %d\n",omp_get_thread_num(),
                                     omp_get_ancestor_thread_num(2),
                                     omp_get_ancestor_thread_num(1));
    }
  }
  printf("FINAL\n");
}

// BARRIER, CRITICAL, ATOMIC

void foo2() {
  int total = 0;
  #pragma omp parallel
  {
    printf("1 -> %d\n",omp_get_thread_num());
    #pragma omp barrier
    printf("2 -> %d\n",omp_get_thread_num());
    #pragma omp barrier
    printf("3 -> %d\n",omp_get_thread_num());
    #pragma omp critical
    ++total;
    printf("4 -> %d\n",omp_get_thread_num());
  }
  printf("total: %d\n",total);
}

// SINGLE, FOR 

void foo3() {
  int total = 0;
  #pragma omp parallel for schedule(dynamic) reduction(+:total)
  for (int i = 0; i < 1000000; i += 2) {
    if (i == 100) sleep(1);
    total += i;
  } 
  #pragma omp parallel
  {
    printf("# -> id: %d\n",omp_get_thread_num());
    #pragma omp single
    printf("SINGLE\n");
  }
  printf("1 -> total: %d\n",total);
  #pragma omp parallel
  {
    #pragma omp for schedule(dynamic) reduction(+:total)
    for (int i = 0; i < 100000; ++i) total += 3*i*i;
    printf("A -> id: %d\n",omp_get_thread_num());
    #pragma omp for schedule(dynamic) reduction(+:total)
    for (int i = 200; i > 100; i -= 2) total += 4*i*i;
    printf("B -> id: %d\n",omp_get_thread_num());
    #pragma omp for schedule(dynamic) reduction(+:total)
    for (int i = -1000; i < 1000; i += 3) total += 5*i*i;
    printf("C -> id: %d\n",omp_get_thread_num());
  }
  printf("2 -> total: %d\n",total);
}

// Rendimiento

void print(int *x, int n) {
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      printf("%d ",x[j+i*n]);
    }
    printf("\n");
  }
}

void multiply(int *a, int *b, int *c, int n) {
  #pragma omp parallel for
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      for (int k = 0; k < n; ++k) {
        c[j+n*i] += a[k+i*n]*b[j+k*n];
      }
    }
  }
}

void matrixmultiply(int n, int output) {
  int *a = (int *) malloc(sizeof(int)*n*n);
  int *b = (int *) malloc(sizeof(int)*n*n);
  int *c = (int *) calloc(n*n,sizeof(int));
  for (int i = 0; i < n*n; ++i) { a[i] = rand()%10; b[i] = rand()%10; }
  multiply(a,b,c,n);
  if (output) {
    printf("A:\n"); print(a,n);
    printf("B:\n"); print(b,n);
    printf("C:\n"); print(c,n);
  }
}


int main(int argc, char *argv[])
{
  //~ foo1();
  //~ foo2();
  //~ foo3();
  matrixmultiply(1200,0);
}
