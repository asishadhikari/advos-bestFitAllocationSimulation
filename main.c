/* Best fit algorithm simulation
	Ashish Adhikari
	run as : ./sim 9   //for 9 processes
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
//mem size in bytes
#define MEMORY_SIZE (int)pow(2,22)
#define BLOCK_SIZE (int)pow(2,10)	//1 kb
//stop simulation these many allocations ??
#define REQUIRED_ALLOCATIONS (int)pow(2,5)

#define MAX_INDEX MEMORY_SIZE/BLOCK_SIZE

void generate_procs(int N);
void allocate_proc(int batch, int size);
//start index of suitable hole
int find_best_index(int size);
//deallocate N oldest processes
void free_memory(void);
int available_hole(void);

//memory bitmap
int *MEMORY;
//num of processes allocated memory
int num_allocated=0;
int batch = 0;  //process batch number
int min_batch = 0; //oldest process batch number 

int main(int argc, char *argv[]){
	if (argc==1 || argc > 2){
		printf("Invalid number of parameters!!\n");
		exit(-1);
	}
	int N = atoi(argv[1]);
	
	//initialise memory 
	MEMORY = (int*)malloc((sizeof(int)*MAX_INDEX));
	for (int i = 0; i < MAX_INDEX; i++)
		MEMORY[i]=0;
	//random number seed
	struct timeval seed;
	gettimeofday(&seed,NULL);
	srand(seed.tv_usec);
	//simulate until crtiteria met 
	while(num_allocated<REQUIRED_ALLOCATIONS){
		generate_procs(N);
	}
	return 0;
} 

void generate_procs(int N){
	int total_size = 0;
	int batch_num = batch;
	int process[N][1];
	for (int i = 0; i < N; i++){
		int size = rand()%91+10;
		total_size+=size;
		process[i][0] = size;
	}
	while (available_hole()<total_size)
		free_memory();
	
	for (int i = 0; i < N; i++)
		allocate_proc(batch_num,process[i][0]);

	batch++;

}

void allocate_proc(int batch_num, int size){
	int start = find_best_index(size);
	MEMORY[start] = batch_num;
	MEMORY[start+1]= size;
	MEMORY[start+size-1] = -1;
	for (int i = start+2; i <start+size-2; i++)
		MEMORY[i] = 1;

}
//free the oldest batch processes
void free_memory(){
	for (int i = 0; i <MAX_INDEX; i++){
		if(MEMORY[i]==min_batch && MEMORY[i+1]>9){
			for (int j = i; j < MEMORY[i+1]; j++)
				MEMORY[j] = 0;
		}
	}
	min_batch++;
}

//return start index of 
int find_best_index(int size){

}

int available_hole(){
	int hole = 0;
	for (int i = 0; i < MAX_INDEX; i++)
		if(MEMORY[i]==0 && MEMORY[i+1]==0)
			hole+=1;
	return hole;
}