#include<stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <mpi.h>

#define N	        16	

#define TEST_RANK	1

// some definition for print-outs used while developing.
//#define DEBUG	0
//#define DEBUG_CHOP_PHASE	0

//		Game of Life parrallel version.
/*
	Segments convention, for input and output(naming)
	f.e:
	A: BBBBB..B:C
	D:          E
	D	    E
	D	    E
	.	    .
	D	    E
	F: GGGG..GG:H
*/

int compute(int p[N+2][N+2], int i, int j)
{
//compute neighbours
	return p[i-1][j-1]+p[i][j-1]+p[i+1][j-1]+p[i-1][j]+p[i+1][j]+p[i-1][j+1]+p[i][j+1]+p[i+1][j+1];
}

void show(int p[N+2][N+2])
{
	//show output
	int i,j;
	printf("PRINT - OUT\n");
	for (i = 1; i < N+1; i++)
	{
		for (j = 1; j < N+1; j++) {
			printf("%4d\t", p[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

// my implementation for teh iteration process.
void my_iter( int *p[], int len, int rank)
{
	int temp[len+2][len+2];
	int i, j, neighbors;

//	printf("i timi einai %d\n", *( ((int *)p+(len+2)*1)+0) );
	
	for (i = 1; i < len+1; i++) {

		for (j = 1; j < len+1; j++) {
		//	neighbors = p[i-1][j-1]+p[i][j-1]+p[i+1][j-1]+p[i-1][j]+p[i+1][j]+p[i-1][j+1]+p[i][j+1]+p[i+1][j+1];
			//Syntax to comform with the style of array of pointers.
			neighbors = *(((int*)p+(len+2)*(i-1))+(j-1))	+
				    *(((int*)p+(len+2)*(i))+(j-1))	+
				    *(((int*)p+(len+2)*(i+1))+(j-1))	+
				    *(((int*)p+(len+2)*(i-1))+(j))	+
				    *(((int*)p+(len+2)*(i+1))+(j))	+
				    *(((int*)p+(len+2)*(i-1))+(j+1))	+
				    *(((int*)p+(len+2)*(i))+(j+1))	+
				    *(((int*)p+(len+2)*(i+1))+(j+1)) ;
			//printf("total for index: %d %d : %d neigb.\n", i, j, neighbors);

			temp[i][j] = *(((int*)p+(len+2)*i)+j) ;

			if( ( *(((int*)p+(len+2)*i)+j) == 1) && (neighbors < 2 || neighbors >3) ) {
				temp[i][j] = 0;
			} else if(  (*(((int*)p+(len+2)*i)+j) == 0) && (neighbors == 3) ) {
				temp[i][j] = 1;
			}
		}
	}

	for( i = 1; i < len+1; i++)
		for( j = 1; j < len+1; j++)
			*(((int*)p+(len+2)*i)+j) = temp[i][j];

	#ifdef DEBUG
	printf("After first generation is: (for rank %d)\n", rank);
	
	if(rank == TEST_RANK)	{
		for( i = 1; i < len+1; i++) {
			for( j = 1; j < len+1; j++)
			//	printf("%d ",temp[i][j]);
				printf("%d ",*(((int*)p+(len+2)*i)+j));
			printf("\n");
		}	
	}
	#endif
}

void iteration(int p[N+2][N+2])
{
	int temp[N+2][N+2]= {{0}};
	int i,j,neighbors;

	for (i = 1; i < N+1; i++)
	{
		for (j = 1; j < N+1; j++)
		{
			neighbors = compute(p,i, j);
			temp[i][j] = p[i][j];

			if((p[i][j] == 1)&&(neighbors < 2 || neighbors > 3)) {
				temp[i][j] = 0;
			} else if((p[i][j] == 0)&&(neighbors == 3)) {
				temp[i][j] = 1;
			}
		}
	}

	for (i = 1; i < N+1; i++)
		for (j = 1; j < N+1; j++)
			p[i][j] = temp[i][j];
}

double getRandom()
{
	return rand()*1.0/RAND_MAX;
}

void initialize(int p[N+2][N+2])
{
	int i, j;
	int test = 0;

	for (i = 0; i < N+2; i++)
		for (j = 0; j < N+2; j++) {
			p[i][j] = 0;
		//	p[i][j] = test;
			test +=1;
		}
///*	
	   for (i = 1; i < N+1; i++)
	       for (j = 1; j < N+1; j++)
	           if (getRandom()>0.9)
		 p[i][j] = 1;

//*/	
}
int main()
{
	/* 2D arraj declaration*/
	int p[N+2][N+2];
	int generations = 1;
	int i;
	int size, rank;
	int len;

	MPI_Init(NULL, NULL);
	MPI_Status status;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if(rank == 0) {
		printf("RUN for %d generations.\n", generations);
		srand(time(NULL));
		initialize(p);
		show(p);
		printf("\n");
	}

	len = N/sqrt(size);
//	if(rank == 0)
//		printf("Creating arrays of %d %d\n",len, len);

	int tmpmat[len][len];
	int j, k;
	int pers[len][len];

	// sending sub-arrays in the other processes.
	if(rank == 0) {
		
		//subarray for rank-0 process.		
		for( j = 0; j < len; j++) {
			for(k = 0; k < len; k++) {
				pers[j][k] = p[1+j][1+k];
			//	printf("%5d",pers[j][k]);
			}
			//printf("\n");
		}

//		printf("------------------r0\n");
		int  *tmp;
		int *find;	

		tmp = &p[1][1+2*len];
		find = &p[1][1];	
	
//		printf("o temp einai sto %d\n",*tmp);
//		printf("o find einai sto %d\n",*find);


		// subarrays for other than 0 ranks		
		for( i = 1; i < size; i++) {
			
//			printf(" for rank: %d\n", i);

			if( (i % (int)sqrt(size)) == 0) { // if we are in next row.
				// edw isos thema.
				tmp = find + (len-1) + 2*len +(len-1)*N + 1;
				find = tmp;
				//printf("o tmp einai : %d o find einai  %d\n", *tmp, *find);
				//printf("new row\n");			
				//tmp-=size;	
				//find = tmp;
			//	printf(" %d %d \n", *tmp, *find);	

				for( j = 0; j < len; j++, tmp+= N+2-len) {
				//for( j = 0; j < len; j++, tmp+= N-1) {

					for(k = 0; k < len; k++) {
						tmpmat[j][k] = *tmp;
				//		printf("%5d ", tmpmat[j][k]);	
						tmp++;
					}
				//	printf("\n");
				}	

			} else if( (i+1)% (int)sqrt(size) == 0) {   // if we are in last column.
			
			//	printf("Last column\n");			
				//tmp = find + (int)sqrt(size);	
				tmp = find + len;	
				find = tmp;	

				for( j = 0; j < len; j++, tmp += N+2-len) {

					for(k = 0; k < len; k++) {
						tmpmat[j][k] = *tmp;
						//printf("%5d", tmpmat[j][k]);	
						tmp++;
					}
//					printf("\n");
				}	

			} else { 
				// if we are not in new row or in last column.
//				printf("Normal\n");

				tmp = find +(int) sqrt(size);
				find = tmp;

//				printf("o temp einai ston %d\n",*tmp);

				for( j = 0; j < len; j++, tmp += N+2-len) {
				//for( j = 0; j < len; j++, tmp += N - 1) {

					for(k = 0; k < len; k++) {
					
						tmpmat[j][k] = *tmp;
//						printf("%5d ", tmpmat[j][k]);	
						tmp++;
					}
//					printf("\n");
				}	
			}			

			MPI_Send(tmpmat, len*len, MPI_INT, i, NULL, MPI_COMM_WORLD);
		}

		MPI_Barrier(MPI_COMM_WORLD);
//		printf("%d rank after barrier sended submatrices.\n", rank);
		
		// just bring it back to tmpmat;	
		for( j = 0; j < len; j++) {
			for(k = 0; k < len; k++) {
				tmpmat[j][k] = pers[j][k];
			}
		}
		
		int mat[len+2][len+2];
		
		//Inputs-outputs of rank-0.

		int ih, oh;
		int ig[len];
		int og[1][len];
		int ie[len];
		int oe[len][1];

		//zero-ing
		for( j = 0; j < len+2; j++)
			for( k = 0; k < len+2; k++)
				mat[j][k] = 0;

		//array cpy shifted padding	
		for( j = 0; j < len; j++)
			for( k = 0; k < len; k++)
				mat[1+j][1+k] = tmpmat[j][k];	
		
		#ifdef DEBUG	
		printf("Padded array of rank 0\n");

		for( j = 0; j < len+2; j++) {
			for( k = 0; k < len+2; k++) {
				printf("%3d ", mat[j][k]);
			}
			printf("\n");
		}		
		#endif	


		//// Looping phase.(playing the game)
		for(i=0; i<generations; i++)
		{
				
			oh = mat[len][len];
//			printf("OH: %d\n", oh);		
//			printf(" OG:\n");

			for(j = 0; j < len; j++) {
				og[0][j] = mat[len][j+1];
				//printf(" %3d ",og[0][j]);
				oe[j][0] = mat[j+1][len];

			}
//			printf("\n");
//			printf("OE: \n");

//			for(j = 0; j < len; j++)
//				printf(" %3d ",oe[j][0]);
//			printf("\n");
			
			//sending	
			MPI_Send(&oh, 1, MPI_INT, rank+(int)sqrt(size) + 1, NULL, MPI_COMM_WORLD);
			MPI_Send(&oe[0], len, MPI_INT, rank + 1, NULL, MPI_COMM_WORLD);
			MPI_Send(&og[0], len, MPI_INT, rank + (int)sqrt(size), NULL, MPI_COMM_WORLD);	

			//receiving	
				
			MPI_Recv(&ih, 1, MPI_INT, rank+(int)sqrt(size) + 1, NULL, MPI_COMM_WORLD, &status);
			
			mat[len+1][len+1] = ih;

			MPI_Recv(ig, len, MPI_INT, rank+(int)sqrt(size) , NULL, MPI_COMM_WORLD, &status);
			
			for( j = 0 ; j < len; j++) {
					mat[len+1][j+1] = ig[j];
				}

			MPI_Recv(ie, len, MPI_INT, rank+ 1, (int)NULL, MPI_COMM_WORLD, &status);
			
			for( j = 0 ; j < len; j++) {
				mat[j+1][len+1] = ie[j];
			}

			// code for iteration.
			


			my_iter((int **) mat, len, rank);


			if(rank == TEST_RANK) {
				printf("After my_iter: %d\n",rank);	
	
				int ll;
				for( ll = 1; ll < len+1; ll++) {
					for( j = 1; j < len+1; j++)
						printf("%d ",mat[ll][j]);
					printf("\n");
				}	
			}	
			//iteration(p);
			// barrier to have everything to show.
//			printf("PreBarier00\n");
			MPI_Barrier(MPI_COMM_WORLD);
//			printf("PostBarier00\n");
		//	show(p);
		}
		
		//	gather all sub-arrays from other processes.
		
		// Update on array p,the result from rank 0.
		for(j = 0; j < len; j++)
		       for(k = 0; k < len; k++)
				p[1+j][1+k] = mat[1+j][1+k];	       



		// Receiving sub-arrays from other processes(rank >0).
		int w;
		int trev[len*len];

		for(w = 1; w < size; w++) {

			MPI_Recv(trev, len*len, MPI_INT, w, (int)NULL, MPI_COMM_WORLD, &status);
			
			//printf("Received from rank-%d\n",w);	

			// placing the received results into
		        // the correct possitions at p-array,
			// for the final display.
			
			int r = w%(int)sqrt(size);
			int c = w/(int)sqrt(size);
			int m = 0;	

			for(j = 0; j < len; j++) {

				for(k = 0; k < len; k++) {

					p[1+j+c*len][1+k+r*len] = trev[m];

					#ifdef DEBUG
					if( w == TEST_RANK)	
						printf("%d %d = %d\n",1+j+c*len, 1+k+r*len, p[1+j+c*len][1+k+r*len]);
					#endif

					m++;
				}
			}

		}

		printf("Final state of the Game of Life's plane\n");
		show(p);

	} else { 

		// Other processes receiving their part of the matrice.
	
		MPI_Recv(tmpmat, len*len, MPI_INT, 0, (int)NULL, MPI_COMM_WORLD, &status); 
		MPI_Barrier(MPI_COMM_WORLD);

		//matrice to place their sub-array.
		int mat[len+2][len+2];
	
		//zero-ing
		for( j = 0; j < len+2; j++)
			for( k = 0; k < len+2; k++)
				mat[j][k] = 0;

		//array cpy shifted padding	
		for( j = 0; j < len; j++)
			for( k = 0; k < len; k++)
				mat[1+j][1+k] = tmpmat[j][k];	
		
	//	sleep(rank);
		
		#ifdef DEBUG	
		printf("Padded array of rank %d\n", rank);

		for( j = 0; j < len+2; j++) {
			for( k = 0; k < len+2; k++) {
				printf("%3d ", mat[j][k]);
			}
			printf("\n");
		}		
		#endif

		// in & out for playing the game.
		int ina, inc, inf, inh;
		int inb[len], ine[len], ind[len], ing[len];
		int oa, oc, of, oh;
		int ob[len], oe[len], od[len], og[len];


		for(i=0; i<generations; i++) {

			//preparing the in-outs	
			oh = mat[len][len];
			oc = mat[1][len];
			oa = mat[1][1];
			of = mat[len][1];	
			
			for( j = 0; j < len; j++) {
				ob[j] = mat[1][1+j];
				od[j] = mat[1+j][1];
				oe[j] = mat[1+j][len];
				og[j] = mat[len][1+j];
			}

			// a printout to ensure we cut the right slices.	

			#ifdef DEBUG_CHOP_PHASE	
				if(rank == 3) {
					printf(" a %d c %d f %d h %d\n", oa, oc, of, oh);	

					printf("ob:\n");
					for(j =  0; j < len;j++)
						printf(" %3d ",ob[j]);
					printf("\n");
				
					printf("oe:\n");
					for(j =  0; j < len;j++)
						printf(" %3d ",oe[j]);
					printf("\n");

					printf("og:\n");
					for(j =  0; j < len;j++)
						printf(" %3d ",og[j]);
					printf("\n");

					printf("od:\n");
					for(j =  0; j < len;j++)
						printf(" %3d ",od[j]);
					printf("\n");

				}
			#endif

			//	The logic of the game.
			//	Send and receive neighbour values for every other process than RANK <0>.
		//***********************************************************************************************************
				/*	Sending 	*/	

			// Sending the OA integer.works.
			if( ((rank > (int)sqrt(size)) && ((rank % (int)sqrt(size)) != 0)) ) {
			//	printf("Stelnw OA: %d ston %d\n", rank, rank - (int)sqrt(size) -1);	
				MPI_Send(&oa, 1, MPI_INT, rank - (int)sqrt(size) - 1, NULL, MPI_COMM_WORLD);
			}	
			//sending OC integer.works.
			if(  (rank >= (int)sqrt(size) ) && ( ( (rank+1) % (int)sqrt(size) ) != 0) ) {
			//	printf("Stelnw OC: %d ston %d\n", rank, rank - (int)sqrt(size)+1);	
				MPI_Send(&oc, 1, MPI_INT, rank - (int)sqrt(size) + 1, NULL, MPI_COMM_WORLD);
			}
			// sending OF integer.works.
			if( (rank % (int)sqrt(size) != 0) && (rank <= size - (int)sqrt(size)) ) {
			//	printf("Stelnw OF: %d ston %d\n", rank, rank + (int)sqrt(size) - 1);	
				MPI_Send(&of, 1, MPI_INT, rank + (int)sqrt(size) - 1, NULL, MPI_COMM_WORLD);
			}
			//sending OH integer.works.
			if( ( ( (rank+1)%(int)sqrt(size) ) != 0) && (rank < size - (int)sqrt(size)) ) {
				//printf("Stelnw OH: %d ston %d\n", rank, rank + (int)sqrt(size) + 1);	
				MPI_Send(&oh, 1, MPI_INT, rank + (int)sqrt(size) + 1, NULL, MPI_COMM_WORLD);
			}
			//sending OB segment.works.
			if( rank >= (int)sqrt(size) ) {
			//	printf("Stelnw OB: %d ston %d\n", rank, rank - (int)sqrt(size) );	
				MPI_Send(ob, len, MPI_INT, rank - (int)sqrt(size) , NULL, MPI_COMM_WORLD);
			}
			//sending OG segment.works.
			if( rank < size - (int)sqrt(size) ) {
				//printf("Stelnw OG: %d ston %d\n", rank, rank + (int)sqrt(size) );	
				MPI_Send(og, len, MPI_INT, rank + (int)sqrt(size) , NULL, MPI_COMM_WORLD);
			}
			//sending OD segment.works.
			if( rank % (int)sqrt(size) != 0 ) {
				//printf("Stelnw OD: %d ston %d\n", rank, rank -1);	
				MPI_Send(od, len, MPI_INT, rank - 1, NULL, MPI_COMM_WORLD);
			}
			//sending OE segment.works.
			if( (rank+1) % (int)sqrt(size) != 0 ) {
			//	printf("Stelnw OE: %d ston %d\n", rank, rank + 1);	
				MPI_Send(oe, len, MPI_INT, rank + 1, NULL, MPI_COMM_WORLD);
			}

				/*	Receiving	*/	
			//receiving INA.works.
			if( (rank > (int)sqrt(size)) && (rank % (int) sqrt(size) != 0) ) {
				MPI_Recv(&ina, 1, MPI_INT, rank-(int)sqrt(size)-1, (int)NULL, MPI_COMM_WORLD, &status);
				mat[0][0] = ina;
				//printf("Recv: %d from %d %d\n",rank, rank - (int)sqrt(size)-1, ina);
			}
			//receiving INC.works.
			if( (rank >= (int)sqrt(size)) && ( (rank+1) %(int)sqrt(size) != 0)) {
				MPI_Recv(&inc, 1, MPI_INT, rank-(int)sqrt(size)+1, (int)NULL, MPI_COMM_WORLD, &status);
				mat[0][len+1] = inc;
				//printf("Recv: rank: %d from %d value: %d\n",rank, rank - (int)sqrt(size)+1, inc);
			}			
			//receiving INF.works.
			if( (rank < size - (int)sqrt(size) ) && ( rank % (int)sqrt(size) != 0) ) {
				MPI_Recv(&inf, 1, MPI_INT, rank+(int)sqrt(size)-1, (int)NULL, MPI_COMM_WORLD, &status);
				mat[len+1][0] = inf;
				//printf("Recv: rank: %d from %d value: %d\n",rank, rank + (int)sqrt(size)-1, inf);
			}			
			//receiving INH.works.
			if( (rank < size - (int)sqrt(size) ) && ( (rank+1) % (int)sqrt(size) != 0) ) {
				MPI_Recv(&inh, 1, MPI_INT, rank+(int)sqrt(size)+1, (int)NULL, MPI_COMM_WORLD, &status);
				mat[len+1][len+1] = inh;
				//printf("Recv: Rank: %d from %d value: %d\n",rank, rank + (int)sqrt(size)+1, inh);
			}			
			//receiving INB.works.
			if( rank > (int)sqrt(size)) {
				MPI_Recv(&inb, len, MPI_INT, rank-(int)sqrt(size), (int)NULL, MPI_COMM_WORLD, &status);
					
				//printf("Recv: Rank: %d from %d :\n",rank, rank - (int)sqrt(size));
				for( j = 0 ; j < len; j++) {
					//printf(" %d \n", inb[j]);
					mat[0][j+1] = inb[j];
				}
			}	
			//receiving IND.works.
			if( (rank % (int)sqrt(size)) != 0) {

				MPI_Recv(ind, len, MPI_INT, rank-1, (int)NULL, MPI_COMM_WORLD, &status);
				//printf("Recv: Rank: %d from %d :\n",rank, rank-1);
				for( j = 0 ; j < len; j++) {
					//printf(" %d \n", ind[j]);
					mat[j+1][0] = ind[j];
				}
			}	
			//receiving INE.works.
			if( ( (rank+1) % (int)sqrt(size)) != 0) {

				MPI_Recv(ine, len, MPI_INT, rank+1, (int)NULL, MPI_COMM_WORLD, &status);
				//printf("Recv: Rank: %d from %d :\n",rank, rank+1);
				for( j = 0 ; j < len; j++) {
				//	printf(" %d \n", ine[j]);
					mat[j+1][len+1] = ine[j];
				}
			}	
			//receiving ING.works.
			if( rank < size - (int)sqrt(size) ) {

				MPI_Recv(ing, len, MPI_INT, rank+(int)sqrt(size), (int)NULL, MPI_COMM_WORLD, &status);
				//printf("Recv: Rank: %d from %d :\n",rank, rank+(int)sqrt(size));
				for( j = 0 ; j < len; j++) {
//					printf(" %d \n", ing[j]);
					mat[len+1][j+1] = ing[j];
				}
			}	

			/*
			// testing for validity of receptions.
			if(rank == 7) {

				for(int k = 0; k < len+2; k++) {
					for(int f = 0; f < len+2; f++)
						printf(" [%d][%d]:: %3d ", k, f, mat[k][f]);
					printf("\n");
				}
			}
			*/
		//***********************************************************************************************************
			/* test for validity in just one rank. 
			if(rank == 5) {
				printf("ADDR: %d\n",mat);	
				my_iter((int **) mat, len);

			}
			*/
			//Iteration
			//printf("Going for my_iter: rank: %d\n", rank);	
			
			my_iter((int **) mat, len, rank);
		
			#ifdef DEBUG	
			if(rank == TEST_RANK) {
				int ll;

				printf("After my_iter: %d\n",rank);	
				for( ll = 1; ll < len+1; ll++) {
					for( j = 1; j < len+1; j++)
						printf("%d ",mat[ll][j]);
					printf("\n");
				}	
			}	
			#endif

			//iteration(p);
			//Barrier might not be necessary since they are synchronizing with Recv()'s.
//			printf("Pre : %d\n",rank);
			MPI_Barrier(MPI_COMM_WORLD);
//			printf("Post : %d\n",rank);
		}
		


		// sending result of sub-array back to RANK 0 for display-ing.	
		int tosend[len*len];
		int w = 0;

		for( i = 0; i < len; i++) {
			for( j = 0; j < len; j++) {
				tosend[w] = mat[1+i][1+j];
				w++;	
				#ifdef DEBUG
				if(rank == TEST_RANK)
					printf("stelnw : %d\n",mat[1+i][1+j]);
				#endif
			}
		}
		//send to RANK-0.
		MPI_Send(tosend, len*len, MPI_INT, 0, NULL, MPI_COMM_WORLD);

	}

	/*sleep(rank);
	
	printf("RANK:%d\n",rank);

	for(int i = 0; i < len; i++) {
		for(int j = 0; j < len; j++)
			printf("%5d ",tmpmat[i][j]);
		printf("\n");	
	}

	*/
//	printf("calling finilize: rank: %d\n", rank);

	MPI_Finalize();

	return 0;
}
