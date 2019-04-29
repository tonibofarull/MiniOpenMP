.SUFFIXES: -omp -gomp -domp -dgomp -asm -dasm -seq
CC = gcc

OPENMP	 = -fopenmp
CFLAGS   = -Wall -O -std=c99 -march=native -I.
CLIBS    =  -L$(MINIOMP)/lib -lminiomp

.c-seq: 
	$(CC) $(CFLAGS) $< -o $@ 

.c-omp: 
	$(CC) $(CFLAGS) $(OPENMP) $< -o $@ $(CLIBS) 

.c-gomp: 
	$(CC) $(CFLAGS) $(OPENMP) $< -o $@ 

.c-asm: 
	$(CC) $(CFLAGS) $(OPENMP) -c -S $< -o $@ $(CLIBS) 

.c-domp: 
	$(CC) -D_DISPLAY_ $(CFLAGS) $(OPENMP) $< -o $@ $(CLIBS) -lX11 -L/usr/X11R6/lib

.c-dgomp: 
	$(CC) -D_DISPLAY_ $(CFLAGS) $(OPENMP) $< -o $@ -lX11 -L/usr/X11R6/lib

.c-dasm: 
	$(CC) -D_DISPLAY_ $(CFLAGS) $(OPENMP) -c -S $< -o $@ $(CLIBS) 


clean:
	rm -rf *-omp *-gomp *-asm *-domp *-dgomp *-seq *.txt *.e* *.o*

ultraclean:
	rm -rf *-omp *-gomp *-asm *-domp *-dgomp *-seq *.prv *.pcf *.row TRACE.mpits set-0 TRACE.sym *.o *.ppm .tareador_precomputed* *.prv *.pcf *.row dependency_graph* *.times.txt lab3-omp* *.ps *.txt *.log TRACE.* set-0 logs
