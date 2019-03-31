all:
	mpicc main.c -lm 
	mpiexec -n 16 ./a.out
