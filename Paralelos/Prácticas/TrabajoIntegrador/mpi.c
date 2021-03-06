#include<stdio.h>
#include<stdlib.h>
#include<mpi.h> 
#include<math.h>


#define MANAGER 0

/* Time in seconds from some point in the past */
double dwalltime();


int main(int argc,char*argv[]){
 double *A,*B,*C,*W,*Buffer,*results;
 int i,j,k,N;
 int check=1;
 double temp,total;
 double totaltick, communicationtick;

 //Controla los argumentos al programa
  if (argc < 2){
   printf("\n Faltan argumentos:: N dimension de la matriz \n");

   return 0;
  }
   N=atoi(argv[1]);





 int worker_id, n_proc, chunk, size_chunk;
 MPI_Init(&argc, &argv);
 MPI_Comm_size(MPI_COMM_WORLD, &n_proc);
 MPI_Comm_rank(MPI_COMM_WORLD, &worker_id);
 chunk=N/n_proc;
 size_chunk=chunk*N;

  B=(double*)malloc(sizeof(double)*N*N);
  Buffer=(double*)malloc(sizeof(double)*size_chunk);
  
  for(i=0;i<size_chunk;i++) Buffer[i]=0;
  
  results=(double*)malloc(sizeof(double)*3);

  if(worker_id == MANAGER){
 //Aloca memoria para las matrices
      A=(double*)malloc(sizeof(double)*N*N);
      C=(double*)malloc(sizeof(double)*N*N);
      W=(double*)malloc(sizeof(double)*N*N);
      //Inicializa la matriz 
      for(i=0;i<N;i++){
	for(j=0;j<N;j++){
		A[i*N+j]= 1.0;
		//Los valores para la matriz B deben ser cargados como una matriz transpuesta.
		B[i+N*j]= 1.0;
		C[i*N+j]= 1.0;
		W[i*N+j]= 1.0;
    }
  }   

  }
  else {
      A=(double*)malloc(sizeof(double)*size_chunk);
      W=(double*)malloc(sizeof(double)*size_chunk);
  }

 
 totaltick = dwalltime();

 MPI_Scatter (A, size_chunk, MPI_DOUBLE, A, size_chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
 MPI_Scatter (W, size_chunk, MPI_DOUBLE, W, size_chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

 results[0]=dwalltime() - totaltick;





 

 for(i=0;i<chunk;i++){
      for(j=0;j<N;j++){
	   temp += W[i*chunk+j];
	   total += A[i*chunk+j] * W[i*chunk+j];
	}
  }


communicationtick = dwalltime();

  MPI_Allreduce(&temp, &temp, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Allreduce(&total, &total, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  MPI_Scatter (A, size_chunk, MPI_DOUBLE, A, size_chunk, MPI_DOUBLE, MANAGER, MPI_COMM_WORLD);
  MPI_Bcast(B, N*N, MPI_DOUBLE, MANAGER, MPI_COMM_WORLD);

results[1]=dwalltime() - communicationtick;

total/=temp;
  //Realiza la ecuacion general




  for(i=0;i<chunk;i++){
      for(j=0;j<N;j++)
	    for(k=0;k<N;k++)
	    Buffer[i*chunk+j]+=sqrt((pow(A[i*chunk+k]-total,2))*(pow(B[j*N+k]-total,2)));
  }




communicationtick = dwalltime();

 MPI_Gather(Buffer, size_chunk, MPI_DOUBLE, C, size_chunk, MPI_DOUBLE, MANAGER, MPI_COMM_WORLD);

 results[2]=dwalltime() - communicationtick;

 totaltick = dwalltime() - totaltick;


 for(i=0;i<3;i++)
MPI_Allreduce(results+i, results+i, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);


  //Chequea los resultados
  if(worker_id == MANAGER){
      for(i=0;i<N*N;i++)
	      check= check&&(C[i]==0.0);
  if(check){
   printf("Tiempo total en segundos: %f \n", totaltick);
   for(i=0,communicationtick=0;i<3;i++) communicationtick+=results[i];
   printf("Tiempo de comunicacion en segundos: %f \n", communicationtick);
   printf("Resultado correcto\n");
  }else{
   printf("Resultado erroneo\n");
  }
 free(C);
 }
 free(A);
 free(B);
 free(Buffer);
 free(results);
 free(W);
 MPI_Finalize();
 return(0);
}



/*****************************************************************/

#include <sys/time.h>

double dwalltime()
{
	double sec;
	struct timeval tv;

	gettimeofday(&tv,NULL);
	sec = tv.tv_sec + tv.tv_usec/1000000.0;
	return sec;
}
